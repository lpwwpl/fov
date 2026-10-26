#include "JonesViewWidget.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>
#include <QPainter>
#include <QPolygonF>
#include <QSplitter>
#include <QSignalBlocker>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_div.h>
#include <cmath>

#include <QTimer>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =========================================================================
// JonesPlotItem 实现
// =========================================================================
JonesPlotItem::JonesPlotItem(double x, double y, double azimuth, double ellipticity,
	const QColor& color, double width, double sizeData, double ellMin, double ellMax)
	: m_x(x), m_y(y), m_azimuth(azimuth), m_ellipticity(ellipticity),
	m_color(color), m_width(width), m_ellipseSize(sizeData),
	m_ellMin(ellMin), m_ellMax(ellMax)
{
	setZ(100);
}

void JonesPlotItem::draw(QPainter* painter,
	const QwtScaleMap& xMap, const QwtScaleMap& yMap,
	const QRectF& canvasRect) const
{
	// -------------------------------------------------------------
	// 1. 线性映射：将 ellipticity 拉伸对比度
	// 数据范围约 [0.79, 2.35] → 映射到视觉轴比 [0.20, 0.80]
	// -------------------------------------------------------------
	double chiVal = std::abs(m_ellipticity);
	double range = m_ellMax - m_ellMin;

	double norm = 0.5; // 防止除以 0 时的默认值
	if (range > 1e-6) {
		norm = (chiVal - m_ellMin) / range;
	}

	// 线性映射到 0.20 ~ 0.80 之间，这样能明显看出胖瘦区别
	double ratio = 0.20 + norm * 0.60;
	ratio = qBound(0.20, ratio, 0.80);

	// 2. 椭圆半轴
	const double a = m_ellipseSize;
	const double b = m_ellipseSize * ratio;

	// -------------------------------------------------------------
	// 3. 方位角：弧度，直接使用
	// -------------------------------------------------------------
	const double phi = m_azimuth;
	const double cosA = std::cos(phi);
	const double sinA = std::sin(phi);

	// 4. 生成椭圆点
	const int N = 36;
	const int i0 = int(0.72 * N);
	const int i1 = qMin(i0 + 4, N);

	QPolygonF poly;
	poly.reserve(N + 1);
	QPointF pStart, pEnd;

	for (int i = 0; i <= N; ++i) {
		const double t = 2.0 * M_PI * i / N;
		const double lx = a * std::cos(t);
		const double ly = b * std::sin(t);
		const double rx = lx * cosA - ly * sinA;
		const double ry = lx * sinA + ly * cosA;

		QPointF pt(xMap.transform(m_x + rx), yMap.transform(m_y + ry));
		poly << pt;

		if (i == i0) pStart = pt;
		if (i == i1) pEnd = pt;
	}

	// 5. 视口裁剪
	if (!canvasRect.intersects(poly.boundingRect().adjusted(-15, -15, 15, 15))) return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// 6. 绘制椭圆外框
	QPen pen(m_color, m_width);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setCapStyle(Qt::RoundCap);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPolyline(poly);

	// 7. 绘制逆时针箭头（V字形空心线条，大幅加长）
	if (m_showArrow) {
		double dx = pEnd.x() - pStart.x();
		double dy = pEnd.y() - pStart.y();
		double len = std::hypot(dx, dy);
		if (len > 1e-6) {
			dx /= len;
			dy /= len;

			// 箭头长度设置（可以在此处微调，12.0 已经比较明显）
			const double headLen = 6;
			// 箭头张角（30度）
			const double arrowAngle = M_PI / 6.0;

			const double tipX = pEnd.x();
			const double tipY = pEnd.y();
			const double angle = std::atan2(dy, dx);

			// 利用三角函数算出 V 字形左右两个尾翼的端点
			QPointF p1 = QPointF(tipX, tipY) - QPointF(headLen * std::cos(angle - arrowAngle), headLen * std::sin(angle - arrowAngle));
			QPointF p2 = QPointF(tipX, tipY) - QPointF(headLen * std::cos(angle + arrowAngle), headLen * std::sin(angle + arrowAngle));

			// 使用与椭圆相同的线宽，不填充（空心）
			painter->setPen(QPen(m_color, m_width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
			painter->setBrush(Qt::NoBrush);
			painter->drawLine(QPointF(tipX, tipY), p1);
			painter->drawLine(QPointF(tipX, tipY), p2);
		}
	}

	painter->restore();
}

// =========================================================================
// JonesViewWidget 实现
// =========================================================================
JonesViewWidget::JonesViewWidget(QWidget* parent)
	: QWidget(parent), m_plot(nullptr), m_panner(nullptr), m_magnifier(nullptr)
{
	m_toolBar_plot = new PlotBase(this);
	m_plot = m_toolBar_plot->m_plot;
	m_grid = m_toolBar_plot->m_grid;
	m_settings = m_toolBar_plot->m_settings;

	connect(m_toolBar_plot, SIGNAL(signalFitView()), this, SLOT(fitView()));
	connect(m_toolBar_plot, SIGNAL(signalZoomIn()), this, SLOT(zoomIn()));
	connect(m_toolBar_plot, SIGNAL(signalZoomOut()), this, SLOT(zoomOut()));
	connect(m_toolBar_plot, SIGNAL(signalDisplayProperties(int)), this, SLOT(slotDisplayProperties(int)));

	m_simple_browser = new QwtPropertyBrowser(m_settings, m_plot, m_grid, m_toolBar_plot->m_legend, this);
	connect(m_simple_browser, SIGNAL(signalUpdateItemStyle(int)), this, SLOT(slotUpdateItemStyle(int)));
	connect(m_simple_browser, SIGNAL(signalXScaleAxes()), this, SLOT(updateXScaleAxes()));
	connect(m_simple_browser, SIGNAL(signalYScaleAxes()), this, SLOT(updateYScaleAxes()));
	connect(m_simple_browser, SIGNAL(signalUpdateScaleDiv()), this, SLOT(applyManualAxisRanges()));
	connect(m_simple_browser, &QwtPropertyBrowser::propertyChanged, this, &JonesViewWidget::applyTickFonts);

	m_splitter = new QSplitter(this);
	m_splitter->addWidget(m_toolBar_plot);
	m_splitter->addWidget(m_simple_browser);
	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 0);
	m_simple_browser->setMinimumWidth(300);
	m_simple_browser->setMaximumWidth(400);
	m_splitter->setCollapsible(1, false);
	m_splitter->setSizes({ 900, 300 });

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_splitter);

	setupFieldViewPlot();
	setupInteractions();
	// Qwt finalizes scale divisions after the widget enters the event loop.
	QTimer::singleShot(0, this, [this]() {
		if (!m_plot || !m_simple_browser) return;
		m_plot->replot();
		updateAxesSettings();
	});
	setMinimumSize(640, 480);
}

JonesViewWidget::~JonesViewWidget()
{
	if (m_settings) m_settings->m_items.clear();
	clearAllJonesItems();
}

void JonesViewWidget::setupFieldViewPlot()
{
	m_plot->setCanvasBackground(Qt::white);

	QwtText title("Polarization Ellipses on Surface");
	title.setFont(QFont("sans", 12, QFont::Bold));
	m_plot->setTitle(title);
	m_settings->title = title.text();
	m_simple_browser->applyTitleSettings();

	m_plot->setAxisTitle(QwtPlot::xBottom, "X Position");
	m_plot->setAxisTitle(QwtPlot::yLeft, "Y Position");

	m_grid->enableX(true);
	m_grid->enableY(true);
	m_grid->enableXMin(true);
	m_grid->enableYMin(true);
	m_grid->setMajorPen(QPen(QColor(220, 220, 220), 1, Qt::SolidLine));
	m_grid->setMinorPen(QPen(QColor(240, 240, 240), 1, Qt::DotLine));
	m_grid->attach(m_plot);

	m_settings->xAxis.title = "X Position";
	m_settings->yAxis.title = "Y Position";
	m_simple_browser->applyXAxisSettings();
	m_simple_browser->applyYAxisSettings();

	m_plot->canvas()->setContextMenuPolicy(Qt::CustomContextMenu);
}

void JonesViewWidget::setupInteractions()
{
	m_panner = new MyPlotPanner(m_plot->canvas());
	m_panner->setMouseButton(Qt::LeftButton);

	m_magnifier = new MyPlotMagnifier(m_plot->canvas());
	m_magnifier->setMouseButton(Qt::RightButton, Qt::ControlModifier);
	m_magnifier->setWheelFactor(1.15);

	connect(m_panner, &MyPlotPanner::panFinished, this, &JonesViewWidget::updateAxesSettings);
	connect(m_magnifier, &MyPlotMagnifier::zoomed, this, &JonesViewWidget::updateAxesSettings);
}

void JonesViewWidget::clearAllJonesItems()
{
	if (m_settings) m_settings->m_items.clear();
	for (auto item : m_jonesItems) {
		item->detach();
		delete item;
	}
	m_jonesItems.clear();
}

void JonesViewWidget::clearData()
{
	m_jonesData.clear();
	clearAllJonesItems();
	if (m_simple_browser) m_simple_browser->updateItemCombo();
	m_plot->replot();
}

bool JonesViewWidget::loadDataFromFile(const QString& filename)
{
	clearData();
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "错误", "无法打开文件: " + filename);
		return false;
	}

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty() || line.startsWith("#") || line.startsWith("azimuth")) continue;

		QStringList parts = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
		if (parts.size() >= 5) {
			JonesVector vec;
			vec.azimuth = parts[1].toDouble();
			vec.ellipticity = parts[2].toDouble();
			vec.x = parts[3].toDouble();
			vec.y = parts[4].toDouble();
			m_jonesData.append(vec);
		}
	}
	file.close();

	if (m_jonesData.isEmpty()) {
		QMessageBox::warning(this, "错误", "文件中没有找到有效的偏振数据");
		return false;
	}

	updatePlot(m_jonesData);
	saveInitialView();
	fitView();
	return true;
}

// 绘制全部 758 个椭圆
void JonesViewWidget::updatePlot(const QVector<JonesVector>& data)
{
	clearAllJonesItems();
	if (data.isEmpty()) return;

	// 1. 自动计算 ellipticity 的极值，用于后续的线性映射
	double ellMin = 1e9, ellMax = -1e9;
	for (const JonesVector& vec : data) {
		double v = std::abs(vec.ellipticity);
		ellMin = qMin(ellMin, v);
		ellMax = qMax(ellMax, v);
	}
	if (ellMax - ellMin < 1e-6) ellMax = ellMin + 1e-6;

	QColor baseColor(30, 30, 160); // 深蓝色
	const double baseWidth = 0.6;  // 细线

	int index = 0;
	for (const JonesVector& vec : data) {
		JonesPlotItem* item = new JonesPlotItem(
			vec.x, vec.y, vec.azimuth, vec.ellipticity,
			baseColor, baseWidth, m_ellipseSize, ellMin, ellMax);
		item->attach(m_plot);
		m_jonesItems.append(item);

		MTFPlotItem settingItem;
		settingItem.index = index;
		settingItem.m_x = vec.x;
		settingItem.m_y = vec.y;
		settingItem.m_angle = vec.azimuth;
		settingItem.m_group = (vec.ellipticity > 0);
		settingItem.m_color = baseColor;
		settingItem.m_length = m_ellipseSize;
		settingItem.m_width = baseWidth;
		settingItem.m_item = item;
		m_settings->m_items[index] = settingItem;

		index++;
	}

	m_simple_browser->updateItemCombo();
	m_plot->replot();
}

void JonesViewWidget::slotUpdateItemStyle(int index)
{
	if (index < 0 || index >= m_settings->m_items.size()) return;
	MTFPlotItem& setting = m_settings->m_items[index];
	if (!setting.m_item) return;

	JonesPlotItem* jonesItem = dynamic_cast<JonesPlotItem*>(setting.m_item);
	if (!jonesItem) return;

	jonesItem->setVisible(setting.visible);
	jonesItem->m_x = setting.m_x;
	jonesItem->m_y = setting.m_y;
	jonesItem->m_azimuth = setting.m_angle;
	jonesItem->m_color = setting.m_color;
	jonesItem->m_ellipseSize = setting.m_length;
	jonesItem->m_width = setting.m_width;

	m_plot->replot();
}

void JonesViewWidget::fitView()
{
	autoScaleAxes();
	if (m_keepAspect) enforceAspectRatio(1.0);
	m_plot->replot();
	updateAxesSettings();
}

void JonesViewWidget::resetView()
{
	m_plot->setAxisScale(QwtPlot::xBottom, -m_defaultRange, m_defaultRange);
	m_plot->setAxisScale(QwtPlot::yLeft, -m_defaultRange, m_defaultRange);
	m_plot->replot();
}

void JonesViewWidget::autoScaleAxes()
{
	if (!m_jonesItems.isEmpty()) {
		double xMin = m_jonesItems.first()->m_x, xMax = xMin;
		double yMin = m_jonesItems.first()->m_y, yMax = yMin;
		for (auto item : m_jonesItems) {
			xMin = qMin(xMin, item->m_x); xMax = qMax(xMax, item->m_x);
			yMin = qMin(yMin, item->m_y); yMax = qMax(yMax, item->m_y);
		}
		double xPad = qMax((xMax - xMin) * 0.05, m_ellipseSize * 2.0);
		double yPad = qMax((yMax - yMin) * 0.05, m_ellipseSize * 2.0);

		m_initialXMin_orig = xMin - xPad;
		m_initialXMax_orig = xMax + xPad;
		m_initialYMin_orig = yMin - yPad;
		m_initialYMax_orig = yMax + yPad;
	}

	m_initialXMin = m_initialXMin_orig;
	m_initialXMax = m_initialXMax_orig;
	m_initialYMin = m_initialYMin_orig;
	m_initialYMax = m_initialYMax_orig;

	m_plot->setAxisScale(QwtPlot::xBottom, m_initialXMin, m_initialXMax);
	m_plot->setAxisScale(QwtPlot::yLeft, m_initialYMin, m_initialYMax);
	m_current_factor = 1;
	updateAxesSettings();
}

void JonesViewWidget::enforceAspectRatio(double aspect)
{
	if (!m_plot) return;
	QWidget* canvas = m_plot->canvas();
	const int w = canvas->width();
	const int h = canvas->height();
	if (w <= 0 || h <= 0) return;

	const QwtScaleDiv xDiv = m_plot->axisScaleDiv(QwtPlot::xBottom);
	const QwtScaleDiv yDiv = m_plot->axisScaleDiv(QwtPlot::yLeft);

	double xMin = xDiv.lowerBound(), xMax = xDiv.upperBound();
	double yMin = yDiv.lowerBound(), yMax = yDiv.upperBound();
	const double xRange = xMax - xMin;
	const double yRange = yMax - yMin;
	if (xRange <= 0 || yRange <= 0) return;

	const double pxPerDataX = w / xRange;
	const double pxPerDataY = h / yRange;
	const double current = pxPerDataX / pxPerDataY;

	if (current > aspect * 1.001) {
		const double newXRange = xRange * current / aspect;
		const double cx = (xMin + xMax) * 0.5;
		xMin = cx - newXRange * 0.5;
		xMax = cx + newXRange * 0.5;
	}
	else if (current < aspect / 1.001) {
		const double newYRange = yRange * aspect / current;
		const double cy = (yMin + yMax) * 0.5;
		yMin = cy - newYRange * 0.5;
		yMax = cy + newYRange * 0.5;
	}
	else {
		return;
	}

	m_plot->setAxisScale(QwtPlot::xBottom, xMin, xMax);
	m_plot->setAxisScale(QwtPlot::yLeft, yMin, yMax);

	m_initialXMin = m_initialXMin_orig = xMin;
	m_initialXMax = m_initialXMax_orig = xMax;
	m_initialYMin = m_initialYMin_orig = yMin;
	m_initialYMax = m_initialYMax_orig = yMax;

	updateAxesSettings();
}

void JonesViewWidget::zoomIn()
{
	m_initialXMin *= 0.9; m_initialXMax *= 0.9;
	m_initialYMin *= 0.9; m_initialYMax *= 0.9;
	m_plot->setAxisScale(QwtPlot::xBottom, m_initialXMin, m_initialXMax);
	m_plot->setAxisScale(QwtPlot::yLeft, m_initialYMin, m_initialYMax);
	m_current_factor *= 0.9;
	m_plot->replot();
	updateAxesSettings();
}

void JonesViewWidget::zoomOut()
{
	m_initialXMin *= 1.1; m_initialXMax *= 1.1;
	m_initialYMin *= 1.1; m_initialYMax *= 1.1;
	m_plot->setAxisScale(QwtPlot::xBottom, m_initialXMin, m_initialXMax);
	m_plot->setAxisScale(QwtPlot::yLeft, m_initialYMin, m_initialYMax);
	m_current_factor *= 1.1;
	m_plot->replot();
	updateAxesSettings();
}

void JonesViewWidget::updateAxesSettings()
{
	if (!m_plot || !m_settings || !m_simple_browser) return;
	const auto sync = [this](int axis, PlotSettings::AxisSettings& cfg) {
		const QwtScaleDiv div = m_plot->axisScaleDiv(axis);
		cfg.min = div.lowerBound();
		cfg.max = div.upperBound();
		const QList<double> ticks = div.ticks(QwtScaleDiv::MajorTick);
		if (ticks.size() >= 2) cfg.step = ticks[1] - ticks[0];
	};
	sync(QwtPlot::xBottom, m_settings->xAxis);
	sync(QwtPlot::yLeft, m_settings->yAxis);
	const bool oldBuilding = m_simple_browser->m_bBuilding;
	m_simple_browser->m_bBuilding = true;
	m_simple_browser->applyXAxisSettings();
	m_simple_browser->applyYAxisSettings();
	m_simple_browser->m_bBuilding = oldBuilding;
	applyTickFonts();
}

void JonesViewWidget::applyTickFonts()
{
	if (!m_plot || !m_settings) return;
	const auto apply = [this](int axis, const QFont& font) {
		m_plot->setAxisFont(axis, font);
		if (QwtScaleWidget* scale = m_plot->axisWidget(axis)) {
			scale->setFont(font);
			scale->updateGeometry();
			scale->update();
		}
	};
	apply(QwtPlot::xBottom, m_settings->xAxis.tickFont);
	apply(QwtPlot::yLeft, m_settings->yAxis.tickFont);
	m_plot->replot();
}

void JonesViewWidget::applyManualAxisRanges()
{
	if (!m_plot || !m_settings) return;
	const auto apply = [this](int axis, const PlotSettings::AxisSettings& settings) {
		if (settings.autoRange || !(settings.max > settings.min)) return;
		m_plot->axisScaleEngine(axis)->setAttribute(QwtScaleEngine::Floating, true);
		m_plot->setAxisScale(axis, settings.min, settings.max,
			settings.step > 0.0 ? settings.step : 0.0);
	};
	apply(QwtPlot::xBottom, m_settings->xAxis);
	apply(QwtPlot::yLeft, m_settings->yAxis);
	applyTickFonts();
}

void JonesViewWidget::updateXScaleAxes()
{
	if (!m_settings->xAxis.autoRange) { applyManualAxisRanges(); return; }
	double low = m_initialXMin_orig, high = m_initialXMax_orig;
	if (!m_jonesItems.isEmpty()) {
		low = high = m_jonesItems.first()->m_x;
		for (auto item : m_jonesItems) {
			low = qMin(low, item->m_x);
			high = qMax(high, item->m_x);
		}
		const double pad = qMax((high - low) * 0.1, m_ellipseSize * 3.0);
		low -= pad; high += pad;
	}
	m_plot->setAxisScale(QwtPlot::xBottom, low, high);
	m_plot->replot();
	updateAxesSettings();
}

void JonesViewWidget::updateYScaleAxes()
{
	if (!m_settings->yAxis.autoRange) { applyManualAxisRanges(); return; }
	double low = m_initialYMin_orig, high = m_initialYMax_orig;
	if (!m_jonesItems.isEmpty()) {
		low = high = m_jonesItems.first()->m_y;
		for (auto item : m_jonesItems) {
			low = qMin(low, item->m_y);
			high = qMax(high, item->m_y);
		}
		const double pad = qMax((high - low) * 0.1, m_ellipseSize * 3.0);
		low -= pad; high += pad;
	}
	m_plot->setAxisScale(QwtPlot::yLeft, low, high);
	m_plot->replot();
	updateAxesSettings();
}

void JonesViewWidget::saveInitialView()
{
	const QwtScaleDiv xDiv = m_plot->axisScaleDiv(QwtPlot::xBottom);
	const QwtScaleDiv yDiv = m_plot->axisScaleDiv(QwtPlot::yLeft);
	m_initialXMin = m_initialXMin_orig = xDiv.lowerBound();
	m_initialXMax = m_initialXMax_orig = xDiv.upperBound();
	m_initialYMin = m_initialYMin_orig = yDiv.lowerBound();
	m_initialYMax = m_initialYMax_orig = yDiv.upperBound();
	m_current_factor = 1;
}

void JonesViewWidget::setLegendPosition(bool leftSide)
{
	m_settings->legend.position = leftSide ? QwtPlot::LeftLegend : QwtPlot::RightLegend;
	m_simple_browser->applyLegendSettings();
}

void JonesViewWidget::showLegend(bool show)
{
	m_settings->legend.visible = show;
	m_simple_browser->applyLegendSettings();
}

void JonesViewWidget::slotDisplayProperties(int value)
{
	m_simple_browser->setVisible(value);
}

void JonesViewWidget::setAllEllipseSize(double sizeData)
{
	m_ellipseSize = sizeData;
	for (auto item : m_jonesItems) item->setEllipseSize(sizeData);
	m_plot->replot();
}

void JonesViewWidget::setAllEllipseShowArrow(bool show)
{
	for (auto item : m_jonesItems) item->setShowArrow(show);
	m_plot->replot();
}
