#include <QTimer>
//#include "fieldviewwidget.h"
//#include <QVBoxLayout>
//#include <QFile>
//#include <QTextStream>
//#include <QDebug>
//#include <QMessageBox>
//#include <cmath>
//#include <QMenu>
//#include <QAction>
//
//#ifndef M_PI
//#define M_PI 3.14159265358979323846
//#endif
//
//FieldViewWidget::FieldViewWidget(QWidget* parent)
//    : QWidget(parent), maxMagnitude(0.0), minMagnitude(0.0)
//{
//    // 创建布局
//    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->setContentsMargins(0, 0, 0, 0);
//
//    // 创建QCustomPlot
//    customPlot = new QCustomPlot(this);
//    layout->addWidget(customPlot);
//
//    // 设置图表
//    setupFieldViewPlot();
//    resize(QSize(640, 480));
//}
//
//FieldViewWidget::~FieldViewWidget()
//{
//}
//
//void FieldViewWidget::setupFieldViewPlot()
//{
//    // 设置交互功能
//    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
//
//    // 设置坐标轴
//    customPlot->xAxis->setLabel("X");
//    customPlot->yAxis->setLabel("Y");
//
//    // 设置坐标轴范围（对称显示）
//    customPlot->xAxis->setRange(-m_defaultRange, m_defaultRange);
//    customPlot->yAxis->setRange(-m_defaultRange, m_defaultRange);
//
//    // 设置网格
//    customPlot->xAxis->grid()->setVisible(true);
//    customPlot->yAxis->grid()->setVisible(true);
//    customPlot->xAxis->grid()->setSubGridVisible(true);
//    customPlot->yAxis->grid()->setSubGridVisible(true);
//
//    // 设置标题
//    customPlot->plotLayout()->insertRow(0);
//    QCPTextElement* title = new QCPTextElement(customPlot, "FOV", QFont("sans", 12, QFont::Bold));
//    customPlot->plotLayout()->addElement(0, 0, title);
//
//    // 启用上下文菜单
//    customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
//
//    // 连接自定义上下文菜单
//    connect(customPlot, &QCustomPlot::customContextMenuRequested, this, [this](const QPoint& pos) {
//        QMenu menu(this);
//        QAction* resetAction = menu.addAction("重置视图");
//        QAction* autoRangeAction = menu.addAction("自动调整范围");
//
//        QAction* selectedAction = menu.exec(customPlot->mapToGlobal(pos));
//
//        if (selectedAction == resetAction) {
//            resetView();
//        }
//        else if (selectedAction == autoRangeAction) {
//            updatePlot();
//        }
//        });
//
//    // 连接双击重置视图
//    connect(customPlot, &QCustomPlot::mouseDoubleClick, this, [this](QMouseEvent* event) {
//        Q_UNUSED(event)
//            resetView();
//        });
//}
//
//bool FieldViewWidget::loadFieldDataFile(const QString& filename)
//{
//    // 清空现有数据
//    clearData();
//
//    QFile file(filename);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        QMessageBox::warning(this, "错误", "无法打开文件: " + filename);
//        return false;
//    }
//
//    QTextStream in(&file);
//    int lineNumber = 0;
//    bool hasValidData = false;
//
//    // 初始化最大值最小值
//    maxMagnitude = -1e9;
//    minMagnitude = 1e9;
//
//    while (!in.atEnd()) {
//        QString line = in.readLine().trimmed();
//        lineNumber++;
//
//        // 跳过空行和标题行
//        if (line.isEmpty() || line.startsWith("index") || line.startsWith("#")) {
//            continue;
//        }
//
//        // 解析数据行
//        QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
//        if (parts.size() >= 5) {
//            bool ok;
//            FieldData data;
//
//            data.index = parts[0].toInt(&ok);
//            if (!ok) continue;
//
//            data.xField = parts[1].toDouble(&ok);
//            if (!ok) continue;
//
//            data.yField = parts[2].toDouble(&ok);
//            if (!ok) continue;
//
//            data.magnitude = parts[3].toDouble(&ok);
//            if (!ok) continue;
//
//            data.angle = parts[4].toDouble(&ok);
//            if (!ok) continue;
//
//            fieldData.append(data);
//            hasValidData = true;
//
//            // 更新最大值最小值
//            if (data.magnitude > maxMagnitude) maxMagnitude = data.magnitude;
//            if (data.magnitude < minMagnitude) minMagnitude = data.magnitude;
//        }
//    }
//
//    file.close();
//
//    if (hasValidData) {
//        updatePlot();
//        return true;
//    }
//    else {
//        QMessageBox::warning(this, "错误", "文件中没有找到有效的视场数据");
//        return false;
//    }
//}
//
//void FieldViewWidget::clearData()
//{
//    fieldData.clear();
//    maxMagnitude = 0.0;
//    minMagnitude = 0.0;
//    customPlot->clearItems();
//    customPlot->replot();
//}
//
//void FieldViewWidget::resetView()
//{
//    // 重置到默认范围
//    customPlot->xAxis->setRange(-m_defaultRange, m_defaultRange);
//    customPlot->yAxis->setRange(-m_defaultRange, m_defaultRange);
//    customPlot->replot();
//}
//
//void FieldViewWidget::updatePlot()
//{
//    // 清除之前的箭头
//    customPlot->clearItems();
//
//    // 自动调整坐标轴范围
//    if (!fieldData.isEmpty()) {
//        double xMin = 1e9, xMax = -1e9, yMin = 1e9, yMax = -1e9;
//        for (const FieldData& data : fieldData) {
//            if (data.xField < xMin) xMin = data.xField;
//            if (data.xField > xMax) xMax = data.xField;
//            if (data.yField < yMin) yMin = data.yField;
//            if (data.yField > yMax) yMax = data.yField;
//        }
//
//        // 添加边距
//        double xRange = xMax - xMin;
//        double yRange = yMax - yMin;
//        double marginX = xRange * 0.1;
//        double marginY = yRange * 0.1;
//
//        // 如果数据范围很小，使用默认范围
//        if (xRange < 1e-6 || yRange < 1e-6) {
//            customPlot->xAxis->setRange(-m_defaultRange, m_defaultRange);
//            customPlot->yAxis->setRange(-m_defaultRange, m_defaultRange);
//        }
//        else {
//            customPlot->xAxis->setRange(xMin - marginX, xMax + marginX);
//            customPlot->yAxis->setRange(yMin - marginY, yMax + marginY);
//        }
//    }
//    else {
//        // 没有数据时使用默认范围
//        resetView();
//        return;
//    }
//
//    // 绘制箭头
//    drawFieldArrows();
//
//    customPlot->replot();
//}
//
//void FieldViewWidget::drawFieldArrows()
//{
//    // 获取当前坐标轴范围来计算基准箭头长度
//    double xRange = customPlot->xAxis->range().size();
//    double yRange = customPlot->yAxis->range().size();
//    double baseArrowLength = 4; // 基准长度为视图范围的8%
//
//    for (const FieldData& data : fieldData) {
//        // 创建箭头
//        QCPItemLine* arrow = new QCPItemLine(customPlot);
//
//        // 计算箭头方向（角度转换为弧度）
//        double angleRad = data.angle * M_PI / 180.0;
//
//        // 箭头起点（数据点位置）
//        arrow->start->setCoords(data.xField, data.yField);
//
//        // 计算箭头长度（与magnitude成正比）
//        double arrowLength = getArrowLength(data.magnitude) * baseArrowLength;
//
//        // 箭头终点（基于角度和长度计算）
//        double endX = data.xField + arrowLength * std::cos(angleRad);
//        double endY = data.yField + arrowLength * std::sin(angleRad);
//        arrow->end->setCoords(endX, endY);
//
//        // 设置箭头样式
//        QPen arrowPen = getArrowPen(data.magnitude);
//        arrow->setPen(arrowPen);
//        arrow->setHead(QCPLineEnding::esSpikeArrow);
//
//        // 设置箭头头部大小与线条粗细成比例
//        double headSize = arrowPen.widthF() * 2.0;
//        arrow->setHead(QCPLineEnding(QCPLineEnding::esSpikeArrow, headSize, headSize * 0.8));
//    }
//}
//
//double FieldViewWidget::getArrowLength(double magnitude)
//{
//    // 根据幅值计算箭头长度比例 (0.5 - 2.0倍)
//    if (maxMagnitude <= minMagnitude) {
//        return 0.5; // 默认长度
//    }
//
//    // 归一化幅值
//    double normalized = (magnitude - minMagnitude) / (maxMagnitude - minMagnitude);
//
//    // 长度在0.1到1.0之间线性变化
//    return 0.1 + normalized * 1;
//}
//
//QPen FieldViewWidget::getArrowPen(double magnitude)
//{
//    // 根据幅值设置箭头颜色和粗细
//    // 使用蓝色系，幅值越大颜色越深/线越粗
//
//    // 归一化幅值
//    double normalized = 0.5;
//    if (maxMagnitude > minMagnitude) {
//        normalized = (magnitude - minMagnitude) / (maxMagnitude - minMagnitude);
//    }
//
//    // 计算颜色（从浅蓝到深蓝）
//    int blueValue = 100 + static_cast<int>(155 * normalized);
//    QColor arrowColor(0, 0, blueValue);
//
//    // 计算线宽（1 - 3.0）
//    double lineWidth = 1 + normalized * 2.5;
//
//    return QPen(arrowColor, lineWidth);
//}
#include "FieldViewWidget.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QPainterPath>
#include <cmath>
#include <limits>
#include <algorithm>
#include <qwt_text.h>
#include <qwt_scale_div.h>
#include <QFileDialog>
#include <QSplitter>
#include <QSignalBlocker>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ArrowItem实现
void ArrowItem::draw(QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect) const
{
    Q_UNUSED(canvasRect);

    painter->save();

    // 将数据坐标转换为屏幕坐标
    double x = xMap.transform(m_x);
    double y = yMap.transform(m_y);

    // 计算箭头方向
    double angleRad = m_angle * M_PI / 180.0;
    double dx = m_length * cos(angleRad);
    double dy = m_length * sin(angleRad);

    // 箭头头部参数
    double arrowHeadLength = m_width * 3.0;
    double arrowHeadWidth = m_width * 2.0;

    // 创建箭头路径
    QPainterPath path;

    // 箭头起点
    path.moveTo(x, y);

    // 箭头终点
    double endX = xMap.transform(m_x + dx);
    double endY = yMap.transform(m_y + dy);
    path.lineTo(endX, endY);

    // 绘制箭头主体
    QPen pen(m_color, m_width);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    // 绘制箭头头部
    double angle = atan2(endY - y, endX - x);

    // 计算箭头头部的两个侧点
    double x1 = endX - arrowHeadLength * cos(angle) + arrowHeadWidth * sin(angle);
    double y1 = endY - arrowHeadLength * sin(angle) - arrowHeadWidth * cos(angle);

    double x2 = endX - arrowHeadLength * cos(angle) - arrowHeadWidth * sin(angle);
    double y2 = endY - arrowHeadLength * sin(angle) + arrowHeadWidth * cos(angle);

    QPainterPath arrowHead;
    arrowHead.moveTo(endX, endY);
    arrowHead.lineTo(x1, y1);
    arrowHead.lineTo(x2, y2);
    arrowHead.lineTo(endX, endY);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(m_color));
    painter->drawPath(arrowHead);

    painter->restore();
}

// FieldViewWidget实现
FieldViewWidget::FieldViewWidget(QWidget* parent)
    : QWidget(parent)
    , m_plot(nullptr)
    , m_panner(nullptr)
    , m_magnifier(nullptr)
    , maxMagnitude(0.0)
    , minMagnitude(0.0)
{
    //// 创建布局
    //QVBoxLayout* layout = new QVBoxLayout(this);
    //layout->setContentsMargins(0, 0, 0, 0);

    //// 创建QwtPlot
    //m_plot = new QwtPlot(this);
    //layout->addWidget(m_plot);


    m_toolBar_plot = new PlotBase(this);
    m_plot = m_toolBar_plot->m_plot;
    m_grid = m_toolBar_plot->m_grid;
    //m_legend = m_toolBar_plot->m_legend;
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
    // 刻度字体属性改变后，刷新真正承载刻度数字的 QwtScaleWidget。
    connect(m_simple_browser, &QwtPropertyBrowser::propertyChanged,
            this, &FieldViewWidget::applyTickFonts);
    //m_simple_browser = new SimplePropertyBrowser(m_settings,m_plot,this);
    // 分割器
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


    // 设置图表
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

FieldViewWidget::~FieldViewWidget()
{
    // PlotSettings stores raw QwtPlotItem pointers. Remove those references
    // before destroying QwtPlotItems; the property editor may still exist.
    if (m_settings) m_settings->m_items.clear();
    clearAllArrows();
}

void FieldViewWidget::setupFieldViewPlot()
{
    // 设置画布背景
    m_plot->setCanvasBackground(Qt::white);
    //m_plot->canvas()->setPaintAttribute(QwtPlotCanvas::BackingStore, true);

    // 设置标题
    QwtText title("FOV");
    title.setFont(QFont("sans", 12, QFont::Bold));
    m_plot->setTitle(title);
    m_settings->title = title.text();
    m_simple_browser->applyTitleSettings();

    // 设置坐标轴标签
    m_plot->setAxisTitle(QwtPlot::xBottom, "X");
    m_plot->setAxisTitle(QwtPlot::yLeft, "Y");

    // 设置坐标轴范围（对称显示）
    m_plot->setAxisScale(QwtPlot::xBottom, -m_defaultRange, m_defaultRange);
    m_plot->setAxisScale(QwtPlot::yLeft, -m_defaultRange, m_defaultRange);

    m_grid->enableX(true);
    m_grid->enableY(true);
    m_grid->enableXMin(true);
    m_grid->enableYMin(true);

    // 设置主网格线样式
    QPen majorGridPen(QColor(200, 200, 200), 1, Qt::SolidLine);
    m_grid->setMajorPen(majorGridPen);

    // 设置次网格线样式
    QPen minorGridPen(QColor(220, 220, 220), 1, Qt::DotLine);
    m_grid->setMinorPen(minorGridPen);

    m_grid->attach(m_plot);

    // 设置坐标轴样式
    QPen axisPen(Qt::black, 2);
    //m_plot->setAxisPen(QwtPlot::xBottom, axisPen);
    //m_plot->setAxisPen(QwtPlot::yLeft, axisPen);

    // 启用抗锯齿
    //m_plot->canvas()->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);

    // 设置右键菜单
    m_plot->canvas()->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接自定义上下文菜单
    connect(m_plot->canvas(), &QWidget::customContextMenuRequested,
        this, [this](const QPoint& pos) {
            QMenu menu(this);
            QAction* resetAction = menu.addAction("重置视图");
            QAction* autoRangeAction = menu.addAction("自动调整范围");

            QAction* selectedAction = menu.exec(m_plot->canvas()->mapToGlobal(pos));

            if (selectedAction == resetAction) {
                resetView();
            }
            else if (selectedAction == autoRangeAction) {
                updatePlot();
            }
        });

    // 连接双击重置视图
    //connect(m_plot->canvas(), &QWidget::mouseDoubleClick,
    //    this, [this](QMouseEvent* event) {
    //        Q_UNUSED(event);
    //        resetView();
    //    });
}

void FieldViewWidget::slotUpdateItemStyle(int index)
{
    if (index < 0 || index >= m_settings->m_items.size()) return;
    MTFPlotItem& item = m_settings->m_items[index];
    if (!item.m_item) return;    

    ArrowItem* arrow_item = dynamic_cast<ArrowItem*>(item.m_item);
    if (!arrow_item) return;
    arrow_item->setVisible(item.visible);
    arrow_item->m_width = (item.m_width);
    arrow_item->m_color = (item.m_color);
    arrow_item->m_x = (item.m_x);
    arrow_item->m_y = (item.m_y);
    arrow_item->m_angle = (item.m_angle);
    arrow_item->m_length = (item.m_length);
    m_plot->replot();
}

void FieldViewWidget::setupInteractions()
{
    // 创建拖拽功能 (对应 QCP::iRangeDrag)
    m_panner = new MyPlotPanner(m_plot->canvas());
    m_panner->setMouseButton(Qt::LeftButton);

    // 创建缩放功能 (对应 QCP::iRangeZoom)
    m_magnifier = new MyPlotMagnifier(m_plot->canvas());
    m_magnifier->setMouseButton(Qt::RightButton, Qt::ControlModifier);
    m_magnifier->setWheelFactor(1.1);

    connect(m_panner, &MyPlotPanner::panFinished, this, &FieldViewWidget::updateAxesSettings);
    connect(m_magnifier, &MyPlotMagnifier::zoomed, this, &FieldViewWidget::updateAxesSettings);
}

void FieldViewWidget::clearAllArrows()
{
    // Avoid leaving stale item pointers in the browser's backing model.
    if (m_settings) m_settings->m_items.clear();
    for (auto arrow : m_arrowItems) {
        arrow->detach();
        delete arrow;
    }
    m_arrowItems.clear();
}

bool FieldViewWidget::loadFieldDataFile(const QString& filename)
{
    // 清空现有数据
    clearData();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + filename);
        return false;
    }

    QTextStream in(&file);
    int lineNumber = 0;
    bool hasValidData = false;

    // 初始化最大值最小值
    maxMagnitude = -1e9;
    minMagnitude = 1e9;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        // 跳过空行和标题行
        if (line.isEmpty() || line.startsWith("index") || line.startsWith("#")) {
            continue;
        }

        // 解析数据行
        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (parts.size() >= 5) {
            bool ok;
            FieldData data;

            data.index = parts[0].toInt(&ok);
            if (!ok) continue;

            data.xField = parts[1].toDouble(&ok);
            if (!ok) continue;

            data.yField = parts[2].toDouble(&ok);
            if (!ok) continue;

            data.magnitude = parts[3].toDouble(&ok);
            if (!ok) continue;

            data.angle = parts[4].toDouble(&ok);
            if (!ok) continue;

            fieldData.append(data);
            hasValidData = true;

            // 更新最大值最小值
            if (data.magnitude > maxMagnitude) maxMagnitude = data.magnitude;
            if (data.magnitude < minMagnitude) minMagnitude = data.magnitude;
        }
    }

    file.close();


    if (hasValidData) {
        updatePlot();
        saveInitialView();
        fitView();
        return true;
    }
    else {
        QMessageBox::warning(this, "错误", "文件中没有找到有效的视场数据");
        return false;
    }
}

void FieldViewWidget::clearData()
{
    fieldData.clear();
    maxMagnitude = 0.0;
    minMagnitude = 0.0;
    clearAllArrows();
    if (m_simple_browser) m_simple_browser->updateItemCombo();
    m_plot->replot();
}

void FieldViewWidget::resetView()
{
    // 重置到默认范围
    m_plot->setAxisScale(QwtPlot::xBottom, -m_defaultRange, m_defaultRange);
    m_plot->setAxisScale(QwtPlot::yLeft, -m_defaultRange, m_defaultRange);
    m_plot->replot();
}

void FieldViewWidget::updatePlot()
{
    // 清除之前的箭头
    clearAllArrows();

    // 自动调整坐标轴范围
    if (!fieldData.isEmpty()) {
        double xMin = 1e9, xMax = -1e9, yMin = 1e9, yMax = -1e9;
        for (const FieldData& data : fieldData) {
            if (data.xField < xMin) xMin = data.xField;
            if (data.xField > xMax) xMax = data.xField;
            if (data.yField < yMin) yMin = data.yField;
            if (data.yField > yMax) yMax = data.yField;
        }

        // 添加边距
        double xRange = xMax - xMin;
        double yRange = yMax - yMin;
        double marginX = xRange * 0.1;
        double marginY = yRange * 0.1;

        // 如果数据范围很小，使用默认范围
        if (xRange < 1e-6 || yRange < 1e-6) {
            m_plot->setAxisScale(QwtPlot::xBottom, -m_defaultRange, m_defaultRange);
            m_plot->setAxisScale(QwtPlot::yLeft, -m_defaultRange, m_defaultRange);
        }
        else {
            m_plot->setAxisScale(QwtPlot::xBottom, xMin - marginX, xMax + marginX);
            m_plot->setAxisScale(QwtPlot::yLeft, yMin - marginY, yMax + marginY);
        }
    }
    else {
        // 没有数据时使用默认范围
        resetView();
        return;
    }

    // 绘制箭头
    drawFieldArrows();

    m_plot->replot();
}

void FieldViewWidget::drawFieldArrows()
{
    // 获取当前坐标轴范围来计算基准箭头长度
    double xRange = m_plot->axisScaleDiv(QwtPlot::xBottom).range();
    double yRange = m_plot->axisScaleDiv(QwtPlot::yLeft).range();
    double baseArrowLength = qMin(xRange, yRange) * 0.08; // 基准长度为视图范围的8%

    m_settings->m_items.clear();
    int index = 0;
    for (const FieldData& data : fieldData) {
        // 计算箭头长度（与magnitude成正比）
        double arrowLength = getArrowLength(data.magnitude) * baseArrowLength;

        // 获取箭头颜色
        QColor arrowColor = getArrowColor(data.magnitude);

        // 获取箭头线宽
        QPen arrowPen = getArrowPen(data.magnitude);

        // 创建自定义箭头项
        ArrowItem* arrow = new ArrowItem(
            data.xField, data.yField, data.angle,
            arrowLength, arrowColor, arrowPen.widthF()
        );

        arrow->attach(m_plot);
        m_arrowItems.append(arrow);

        MTFPlotItem item;
        item.index = index;
        item.m_x = data.xField;
        item.m_y = data.yField;
        item.m_angle = data.angle;
        item.m_color = arrowColor;
        item.m_length = arrowLength;
        item.m_width = arrowPen.widthF();
        item.m_group = 0;
        item.m_item = arrow;
        m_settings->m_items[index] = item;

        index++;
    }
    m_simple_browser->updateItemCombo();
}

double FieldViewWidget::getArrowLength(double magnitude)
{
    // 根据幅值计算箭头长度比例 (0.5 - 2.0倍)
    if (maxMagnitude <= minMagnitude) {
        return 0.5; // 默认长度
    }

    // 归一化幅值
    double normalized = (magnitude - minMagnitude) / (maxMagnitude - minMagnitude);

    // 长度在0.5到2.0之间线性变化
    return 0.0 + normalized * 1.2;
}

QPen FieldViewWidget::getArrowPen(double magnitude)
{
    // 根据幅值设置箭头线宽（1 - 3.0）
    double lineWidth = 1.0;
    if (maxMagnitude > minMagnitude) {
        double normalized = (magnitude - minMagnitude) / (maxMagnitude - minMagnitude);
        lineWidth = 1.0 + normalized * 2.0;
    }

    QColor arrowColor = getArrowColor(magnitude);
    return QPen(arrowColor, lineWidth);
}

QColor FieldViewWidget::getArrowColor(double magnitude)
{
    // 根据幅值设置箭头颜色
    // 使用蓝色系，幅值越大颜色越深

    // 归一化幅值
    double normalized = 0.5;
    if (maxMagnitude > minMagnitude) {
        normalized = (magnitude - minMagnitude) / (maxMagnitude - minMagnitude);
    }

    // 计算颜色（从浅蓝到深蓝）
    int blueValue = 100 + static_cast<int>(155 * normalized);
    int greenValue = static_cast<int>(100 * (1.0 - normalized));
    return QColor(0, greenValue, blueValue);
}

void FieldViewWidget::zoomOut()
{
    //m_plot->setAxisScale(QwtPlot::xBottom,
    //    m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound() * 1.1,
    //    m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound() * 1.1);
    //m_plot->setAxisScale(QwtPlot::yLeft,
    //    m_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound() * 1.1,
    //    m_plot->axisScaleDiv(QwtPlot::yLeft).upperBound() * 1.1);
    //m_plot->replot();

    //m_settings->xAxis.min = m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    //m_settings->xAxis.max = m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound();

    //m_settings->yAxis.min = m_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    //m_settings->yAxis.max = m_plot->axisScaleDiv(QwtPlot::yLeft).upperBound();

    //m_simple_browser->applyXAxisSettings();
    //m_simple_browser->applyYAxisSettings();
    m_initialXMin = m_initialXMin * 1.1;
    m_initialXMax = m_initialXMax * 1.1;
    m_initialYMax = m_initialYMax * 1.1;
    m_initialYMin = m_initialYMin * 1.1;
    // 恢复初始视图范围
    m_plot->setAxisScale(QwtPlot::xBottom, m_initialXMin, m_initialXMax);
    m_plot->setAxisScale(QwtPlot::yLeft, m_initialYMin, m_initialYMax);
    m_plot->setAxisScale(QwtPlot::yRight, m_initialYMin, m_initialYMax);

    m_current_factor = m_current_factor * 1.1;
    //m_simple_browser->applyXAxisSettings();
    //m_simple_browser->applyYAxisSettings();
    // 重新绘制
    m_plot->replot();

    updateAxesSettings();
}

void FieldViewWidget::fitView()
{
    autoScaleAxes();
    m_plot->replot();
}
void FieldViewWidget::updateAxesSettings()
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
    // blockSignals(browser) 无法拦截 manager 的 valueChanged，必须阻断 manager。
    const bool oldBuilding = m_simple_browser->m_bBuilding;
    m_simple_browser->m_bBuilding = true;
    m_simple_browser->applyXAxisSettings();
    m_simple_browser->applyYAxisSettings();
    m_simple_browser->m_bBuilding = oldBuilding;
    applyTickFonts();
}
void FieldViewWidget::autoScaleAxes()
{
    m_initialXMin = m_initialXMin_orig;
    m_initialXMax = m_initialXMax_orig;
    m_initialYMin = m_initialYMin_orig;
    m_initialYMax = m_initialYMax_orig;

    // 恢复初始视图范围
    m_plot->setAxisScale(QwtPlot::xBottom, m_initialXMin, m_initialXMax);
    m_plot->setAxisScale(QwtPlot::yLeft, m_initialYMin, m_initialYMax);
    m_plot->setAxisScale(QwtPlot::yRight, m_initialYMin, m_initialYMax);

    m_current_factor = 1;

    // 重新绘制
    m_plot->replot();

    //m_simple_browser->applyXAxisSettings();
    //m_simple_browser->applyYAxisSettings();
    updateAxesSettings();
    //if (!fieldData.isEmpty()) {
    //    double xMin = 1e9, xMax = -1e9, yMin = 1e9, yMax = -1e9;
    //    for (const FieldData& data : fieldData) {
    //        if (data.xField < xMin) xMin = data.xField;
    //        if (data.xField > xMax) xMax = data.xField;
    //        if (data.yField < yMin) yMin = data.yField;
    //        if (data.yField > yMax) yMax = data.yField;
    //    }

    //    // 添加边距
    //    double xRange = xMax - xMin;
    //    double yRange = yMax - yMin;
    //    double marginX = xRange * 0.1;
    //    double marginY = yRange * 0.1;

    //    // 如果数据范围很小，使用默认范围
    //    if (xRange < 1e-6 || yRange < 1e-6) {
    //        m_plot->setAxisScale(QwtPlot::xBottom, -m_defaultRange, m_defaultRange);
    //        m_plot->setAxisScale(QwtPlot::yLeft, -m_defaultRange, m_defaultRange);


    //        m_settings->yAxis.min = -m_defaultRange/2;
    //        m_settings->yAxis.max = m_defaultRange / 2;
    //        m_settings->yAxis.min = -m_defaultRange / 2;
    //        m_settings->yAxis.max = m_defaultRange / 2;
    //    }
    //    else {
    //        m_plot->setAxisScale(QwtPlot::xBottom, xMin - marginX, xMax + marginX);
    //        m_plot->setAxisScale(QwtPlot::yLeft, yMin - marginY, yMax + marginY);

    //        m_settings->yAxis.min = xMin - marginX;
    //        m_settings->yAxis.max = xMax + marginX;
    //        m_settings->yAxis.min = yMin - marginY;
    //        m_settings->yAxis.max = yMax + marginY;
    //    }
    //}
    //else {
    //    // 没有数据时使用默认范围
    //    resetView();
    //    return;
    //}

    //m_simple_browser->applyXAxisSettings();
    //m_simple_browser->applyYAxisSettings();
}

void FieldViewWidget::zoomIn()
{
    //m_plot->setAxisScale(QwtPlot::xBottom,
    //    m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound() * 0.9,
    //    m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound() * 0.9);
    //m_plot->setAxisScale(QwtPlot::yLeft,
    //    m_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound() * 0.9,
    //    m_plot->axisScaleDiv(QwtPlot::yLeft).upperBound() * 0.9);
    //m_plot->replot();

    //m_settings->xAxis.min = m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    //m_settings->xAxis.max = m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound();

    //m_settings->yAxis.min = m_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    //m_settings->yAxis.max = m_plot->axisScaleDiv(QwtPlot::yLeft).upperBound();

    //m_simple_browser->applyXAxisSettings();
    //m_simple_browser->applyYAxisSettings();

    m_initialXMin = m_initialXMin * 0.9;
    m_initialXMax = m_initialXMax * 0.9;
    m_initialYMax = m_initialYMax * 0.9;
    m_initialYMin = m_initialYMin * 0.9;
    // 恢复初始视图范围
    m_plot->setAxisScale(QwtPlot::xBottom, m_initialXMin, m_initialXMax);
    m_plot->setAxisScale(QwtPlot::yLeft, m_initialYMin, m_initialYMax);
    m_plot->setAxisScale(QwtPlot::yRight, m_initialYMin, m_initialYMax);
    // 更新点列位置
    m_current_factor = m_current_factor * 0.9;
    //m_simple_browser->applyXAxisSettings();
    //m_simple_browser->applyYAxisSettings();
    // 重新绘制
    m_plot->replot();

    updateAxesSettings();
}


void FieldViewWidget::applyTickFonts()
{
	return;
	//////////////////////////////////
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

void FieldViewWidget::applyManualAxisRanges()
{
    if (!m_plot || !m_settings) return;
    const auto apply = [this](int axis, const PlotSettings::AxisSettings& settings) {
        if (settings.autoRange || !(settings.max > settings.min)) return;
        // Floating 防止 Qwt 对手动设置的边界再次向外取整。
        m_plot->axisScaleEngine(axis)->setAttribute(QwtScaleEngine::Floating, true);
        m_plot->setAxisScale(axis, settings.min, settings.max,
                             settings.step > 0.0 ? settings.step : 0.0);
    };
    apply(QwtPlot::xBottom, m_settings->xAxis);
    apply(QwtPlot::yLeft, m_settings->yAxis);
    applyTickFonts();
}

void FieldViewWidget::updateXScaleAxes()
{
    if (!m_settings->xAxis.autoRange) { applyManualAxisRanges(); return; }
    double low = m_initialXMin_orig;
    double high = m_initialXMax_orig;
    if (!fieldData.isEmpty()) {
        low = high = fieldData.first().xField;
        for (const FieldData& d : fieldData) {
            low = qMin(low, d.xField);
            high = qMax(high, d.xField);
        }
        const double pad = qMax((high - low) * 0.1, 1.0);
        low -= pad; high += pad;
    }
    m_plot->setAxisScale(QwtPlot::xBottom, low, high);
    m_plot->replot();
    updateAxesSettings();
}

void FieldViewWidget::updateYScaleAxes()
{
    if (!m_settings->yAxis.autoRange) { applyManualAxisRanges(); return; }
    double low = m_initialYMin_orig;
    double high = m_initialYMax_orig;
    if (!fieldData.isEmpty()) {
        low = high = fieldData.first().yField;
        for (const FieldData& d : fieldData) {
            low = qMin(low, d.yField);
            high = qMax(high, d.yField);
        }
        const double pad = qMax((high - low) * 0.1, 1.0);
        low -= pad; high += pad;
    }
    m_plot->setAxisScale(QwtPlot::yLeft, low, high);
    m_plot->setAxisScale(QwtPlot::yRight, low, high);
    m_plot->replot();
    updateAxesSettings();
}

void FieldViewWidget::saveInitialView()
{
    // 保存初始视图范围
    QwtScaleDiv xScaleDiv = m_plot->axisScaleDiv(QwtPlot::xBottom);
    QwtScaleDiv yScaleDiv = m_plot->axisScaleDiv(QwtPlot::yLeft);

    m_initialXMin = xScaleDiv.lowerBound();
    m_initialXMax = xScaleDiv.upperBound();
    m_initialYMin = yScaleDiv.lowerBound();
    m_initialYMax = yScaleDiv.upperBound();

    m_initialXMin_orig = xScaleDiv.lowerBound();
    m_initialXMax_orig = xScaleDiv.upperBound();
    m_initialYMin_orig = yScaleDiv.lowerBound();
    m_initialYMax_orig = yScaleDiv.upperBound();

    m_current_factor = 1;
}
void FieldViewWidget::setLegendPosition(bool leftSide)
{
    m_settings->legend.position = leftSide ? QwtPlot::LeftLegend : QwtPlot::RightLegend;
    m_simple_browser->applyLegendSettings();
}
void FieldViewWidget::showLegend(bool show)
{
    m_settings->legend.visible = show;
    m_simple_browser->applyLegendSettings();
}

void FieldViewWidget::slotDisplayProperties(int value)
{
    m_simple_browser->setVisible(value);
}
