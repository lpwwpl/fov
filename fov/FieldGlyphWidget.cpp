#include "FieldGlyphWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QSplitter>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QMessageBox>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <qwt_scale_map.h>
#include <qwt_scale_div.h>
#include <QScopedValueRollback>
#include <limits>
#include <QSignalBlocker>
#include <qwt_scale_engine.h>
#include <QEvent>
#include <QWheelEvent>
#include <QTimer>
#include <qwt_scale_widget.h>

FieldGlyphItem::FieldGlyphItem(Shape s, double px, double py, double deg,
                               double size, QColor c, double w)
    : shape(s), x(px), y(py), angle(deg), length(size), width(w), color(c) { setZ(100); }

void FieldGlyphItem::draw(QPainter* p, const QwtScaleMap& xm,
                          const QwtScaleMap& ym, const QRectF&) const {
    const QPointF center(xm.transform(x), ym.transform(y));
    const double rx=std::abs(xm.transform(x+length)-xm.transform(x));
    const double ry=std::abs(ym.transform(y+length)-ym.transform(y));
    const double a=qDegreesToRadians(angle);
    p->save(); p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(QPen(color, width)); p->setBrush(Qt::NoBrush);
    if(shape==Circle || shape==Cone) p->drawEllipse(center,rx,ry);
    if(shape==Line) {
        // Input Angle is the line direction in degrees: 0 horizontal, 90 vertical.
        // Magnitude is the FULL line length (not the half-length).
        const double t=a;
        QPointF d(0.5*rx*std::cos(t),-0.5*ry*std::sin(t));
        p->drawLine(center-d,center+d);
    } else if(shape==Cone) {
        // Two tangent-like edges on the outer side of the circle.
        const double opening=0.55;
        QPointF tip(center.x()+1.75*rx*std::cos(a),center.y()-1.75*ry*std::sin(a));
        QPointF side1(center.x()+rx*std::cos(a+opening),center.y()-ry*std::sin(a+opening));
        QPointF side2(center.x()+rx*std::cos(a-opening),center.y()-ry*std::sin(a-opening));
        QPainterPath wedge; wedge.moveTo(side1); wedge.lineTo(tip); wedge.lineTo(side2);
        p->drawPath(wedge);
    }
    p->restore();
}

FieldGlyphWidget::FieldGlyphWidget(FieldGlyphItem::Shape shape, QWidget* parent)
  : QWidget(parent), m_shape(shape), m_base(new PlotBase(this)), m_browser(nullptr) {
    m_browser=new QwtPropertyBrowser(m_base->m_settings,m_base->m_plot,
                     m_base->m_grid,m_base->m_legend,this);
    auto* layout=new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    auto* splitter = new QSplitter(this);
    splitter->addWidget(m_base);
    splitter->addWidget(m_browser);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setCollapsible(1, false);
    splitter->setSizes({900, 300});
    m_browser->setMinimumWidth(300);
    m_browser->setMaximumWidth(400);
    layout->addWidget(splitter);
    auto* plot=m_base->m_plot;
    plot->setTitle("FOV"); m_base->m_settings->title=QString::fromUtf8("FOV");
    m_browser->applyTitleSettings();
    plot->setAxisTitle(QwtPlot::xBottom,"X");plot->setAxisTitle(QwtPlot::yLeft,"Y");
    // PlotBase enables the grid by default; keep it visible on initial display.
    m_base->m_settings->gridVisible = true;
    m_base->m_grid->enableX(true);
    m_base->m_grid->enableY(true);
    m_base->m_grid->attach(plot);
    // Wheel zoom is handled by this widget for Circle/Line/Cone only.
    plot->canvas()->installEventFilter(this);
    // Initialize the property model from the actual displayed axes.
    m_base->m_settings->xAxis.autoRange = true;
    m_base->m_settings->yAxis.autoRange = true;
    applyAxes();
    // Preserve the exact user-entered limits instead of Qwt rounding them.
    plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);
    { const bool oldBuilding = m_browser->m_bBuilding;
      m_browser->m_bBuilding = true;
      m_browser->applyXAxisSettings();
      m_browser->applyYAxisSettings();
      m_browser->m_bBuilding = oldBuilding; }
    connect(m_base,SIGNAL(signalZoomIn()),this,SLOT(zoomIn()));
    connect(m_base,SIGNAL(signalZoomOut()),this,SLOT(zoomOut()));
    connect(m_base,SIGNAL(signalFitView()),this,SLOT(fitView()));
    connect(m_base,SIGNAL(signalDisplayProperties(int)),this,SLOT(slotDisplayProperties(int)));
    connect(m_browser,SIGNAL(signalUpdateItemStyle(int)),this,SLOT(slotUpdateItemStyle(int)));
    connect(m_browser,SIGNAL(signalXScaleAxes()),this,SLOT(updateXScaleAxes()));
    connect(m_browser,SIGNAL(signalYScaleAxes()),this,SLOT(updateYScaleAxes()));
    connect(m_browser,SIGNAL(signalUpdateScaleDiv()),this,SLOT(updateScaleDiv()));
    // Also react after the property-browser finishes updating its settings.
    connect(m_browser, &QwtPropertyBrowser::propertyChanged, this, [this]() {
        if (m_updatingAxis) return;
        updateScaleDiv();
    });
    updateXScaleAxes();
    updateYScaleAxes();
    QTimer::singleShot(0, this, [this]() {
        if (!m_base || !m_browser) return;
        QScopedValueRollback<bool> guard(m_updatingAxis, true);
        m_base->m_plot->replot();
        syncAxisProperties(true, true);
    });
    // Qwt tick labels belong to QwtScaleWidget, not to the axis title.
    // Reapply fonts after the property manager finishes processing a font edit.
    // This also handles edits to sub-properties (family / point size / bold).
    auto applyTickFonts = [this]() {
        QwtPlot* plot = m_base->m_plot;
        const PlotSettings* settings = m_base->m_settings;
        const auto applyFont = [plot](int axis, const QFont& font) {
            plot->setAxisFont(axis, font);
            if (QwtScaleWidget* widget = plot->axisWidget(axis)) {
                widget->setFont(font);
                widget->updateGeometry();
                widget->update();
            }
        };
        applyFont(QwtPlot::xBottom, settings->xAxis.tickFont);
        applyFont(QwtPlot::yLeft, settings->yAxis.tickFont);
        plot->replot();
    };
    applyTickFonts();
    connect(m_browser->m_manager, &QtVariantPropertyManager::valueChanged,
            this, [this, applyTickFonts](QtProperty* property, const QVariant&) {
        const auto& xProps = m_browser->m_axisProps[QwtPlot::xBottom];
        const auto& yProps = m_browser->m_axisProps[QwtPlot::yLeft];
        if (property != xProps.tickFontProperty && property != yProps.tickFontProperty)
            return;
        QTimer::singleShot(0, this, applyTickFonts);
    });
    setMinimumSize(640,480);
}
FieldGlyphWidget::~FieldGlyphWidget(){clearData();}
void FieldGlyphWidget::applyAxes(){
    m_base->m_plot->setAxisScale(QwtPlot::xBottom,m_xMin,m_xMax);
    m_base->m_plot->setAxisScale(QwtPlot::yLeft,m_yMin,m_yMax);
    m_base->m_plot->replot();
}
void FieldGlyphWidget::clearData(){
    m_base->m_settings->m_items.clear();
    for(auto* item:m_items){item->detach();delete item;}
    m_items.clear();m_browser->updateItemCombo();m_base->m_plot->replot();
}
bool FieldGlyphWidget::loadFieldDataFile(const QString& path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,QString::fromUtf8("读取失败"),path);return false;
    }
    clearData();
    QTextStream in(&file);
    while(!in.atEnd()){
        QString line=in.readLine().trimmed();
        QStringList parts=line.split(QRegularExpression(QString::fromUtf8("\\s+")),QString::SkipEmptyParts);
        if(parts.size()<4)continue;
        bool ok1=false,ok2=false,ok3=false,ok4=false;
        int idx=parts[0].toInt(&ok1);
        double x=parts[1].toDouble(&ok2), y=parts[2].toDouble(&ok3),mag=parts[3].toDouble(&ok4);
        if(!(ok1&&ok2&&ok3&&ok4))continue;
        double angle=parts.size()>=5 ? parts[4].toDouble() : qRadiansToDegrees(std::atan2(y,x));
        // Line: magnitude is total length; other glyph types keep their existing sizing.
        double size = (m_shape == FieldGlyphItem::Line) ? mag : 0.12+1.25*mag;
        auto* glyph=new FieldGlyphItem(m_shape,x,y,angle,size,Qt::black,0.8);
        glyph->attach(m_base->m_plot);m_items.append(glyph);
        MTFPlotItem prop;
        prop.index=m_items.size()-1; prop.m_x=x;prop.m_y=y;
        prop.m_angle=angle;prop.m_length=size;prop.m_color=Qt::black;
        prop.m_width=0.8;prop.m_group=0;prop.m_item=glyph;
        m_base->m_settings->m_items[prop.index]=prop;
        Q_UNUSED(idx);
    }
    m_browser->updateItemCombo();
    // File loading changes the automatic field limits.
    if (m_base->m_settings->xAxis.autoRange) updateXScaleAxes();
    if (m_base->m_settings->yAxis.autoRange) updateYScaleAxes();
    m_base->m_plot->replot();
    return !m_items.isEmpty();
}
void FieldGlyphWidget::slotUpdateItemStyle(int index){
    if(!m_base->m_settings->m_items.contains(index))return;
    const MTFPlotItem& prop=m_base->m_settings->m_items[index];
    auto* item=dynamic_cast<FieldGlyphItem*>(prop.m_item);
    if(!item)return;
    item->x=prop.m_x;item->y=prop.m_y;item->angle=prop.m_angle;
    item->length=std::max(0.0,prop.m_length);item->width=prop.m_width;
    item->color=prop.m_color;item->setVisible(prop.visible);
    m_base->m_plot->replot();
}
void FieldGlyphWidget::slotDisplayProperties(int state){}
// Update axis limits and tick interval from the property model.  This is
// called for min/max/step changes (signalUpdateScaleDiv).
void FieldGlyphWidget::updateScaleDiv(){
    if (m_updatingAxis) return;
    auto* plot = m_base->m_plot;
    const auto* settings = m_base->m_settings;
    const auto& x = settings->xAxis;
    const auto& y = settings->yAxis;
    // Never overwrite manual values with the displayed/rounded scale division.
    if (!x.autoRange && x.max > x.min) {
        m_xMin=x.min; m_xMax=x.max;
        plot->setAxisScale(QwtPlot::xBottom, x.min, x.max,
                           x.step > 0.0 ? x.step : 0.0);
    }
    if (!y.autoRange && y.max > y.min) {
        m_yMin=y.min; m_yMax=y.max;
        plot->setAxisScale(QwtPlot::yLeft, y.min, y.max,
                           y.step > 0.0 ? y.step : 0.0);
    }
    plot->replot();
}

void FieldGlyphWidget::syncAxisProperties(bool xAxis, bool yAxis){
    auto* settings=m_base->m_settings;
    auto* plot=m_base->m_plot;
    // Do not block the property manager: QtTreePropertyBrowser needs its
    // valueChanged/propertyChanged signals to repaint editor values.  Use the
    // browser's existing re-entry guard so onValueChanged() does not write back.
    const bool oldBuilding = m_browser->m_bBuilding;
    m_browser->m_bBuilding = true;
    if(xAxis){
        const QwtScaleDiv div=plot->axisScaleDiv(QwtPlot::xBottom);
        settings->xAxis.min=div.lowerBound();
        settings->xAxis.max=div.upperBound();
        const QList<double> ticks=div.ticks(QwtScaleDiv::MajorTick);
        if(ticks.size()>1) settings->xAxis.step=ticks[1]-ticks[0];
        m_xMin=div.lowerBound(); m_xMax=div.upperBound();
        m_browser->applyXAxisSettings();
    }
    if(yAxis){
        const QwtScaleDiv div=plot->axisScaleDiv(QwtPlot::yLeft);
        settings->yAxis.min=div.lowerBound();
        settings->yAxis.max=div.upperBound();
        const QList<double> ticks=div.ticks(QwtScaleDiv::MajorTick);
        if(ticks.size()>1) settings->yAxis.step=ticks[1]-ticks[0];
        m_yMin=div.lowerBound(); m_yMax=div.upperBound();
        m_browser->applyYAxisSettings();
    }
    m_browser->m_bBuilding = oldBuilding;
}

void FieldGlyphWidget::autoAxis(bool xAxis){
    if(m_updatingAxis) return;
    QScopedValueRollback<bool> guard(m_updatingAxis,true);
    double lo=std::numeric_limits<double>::max();
    double hi=std::numeric_limits<double>::lowest();
    for (const FieldGlyphItem* item : m_items){
        if (!item->isVisible()) continue;
        const double center = xAxis ? item->x : item->y;
        // Account for the rendered glyph, including the cone tip.
        const double reach=std::abs(item->length)*(item->shape==FieldGlyphItem::Cone?1.75:1.0);
        lo=std::min(lo,center-reach);
        hi=std::max(hi,center+reach);
    }
    if(lo>hi){lo=-16;hi=16;}
    double margin=(hi-lo)*0.05;
    if(margin<0.1)margin=0.1;
    lo-=margin;hi+=margin;
    auto* plot=m_base->m_plot;
    const int axis=xAxis ? QwtPlot::xBottom : QwtPlot::yLeft;
    // 0 asks Qwt to calculate a suitable major-tick interval.
    plot->setAxisScale(axis,lo,hi,0.0);
    plot->replot();
    syncAxisProperties(xAxis,!xAxis);
}
void FieldGlyphWidget::updateXScaleAxes(){
    if(m_updatingAxis)return;
    if(m_base->m_settings->xAxis.autoRange)autoAxis(true);
    else updateScaleDiv();
}
void FieldGlyphWidget::updateYScaleAxes(){
    if(m_updatingAxis)return;
    if(m_base->m_settings->yAxis.autoRange)autoAxis(false);
    else updateScaleDiv();
}
void FieldGlyphWidget::fitView(){
    // Fit always returns to the full glyph extent, regardless of manual mode.
    autoAxis(true);
    autoAxis(false);
    // Always refresh min/max/step from the final Qwt scale division.
    QScopedValueRollback<bool> guard(m_updatingAxis, true);
    syncAxisProperties(true, true);
}
void FieldGlyphWidget::zoomIn(){
    auto* plot=m_base->m_plot;
    const auto x=plot->axisScaleDiv(QwtPlot::xBottom);
    const auto y=plot->axisScaleDiv(QwtPlot::yLeft);
    const double xc=(x.lowerBound()+x.upperBound())/2.0;
    const double yc=(y.lowerBound()+y.upperBound())/2.0;
    const double xh=(x.upperBound()-x.lowerBound())*0.4;
    const double yh=(y.upperBound()-y.lowerBound())*0.4;
    plot->setAxisScale(QwtPlot::xBottom,xc-xh,xc+xh);
    plot->setAxisScale(QwtPlot::yLeft,yc-yh,yc+yh);
    plot->replot();
    QScopedValueRollback<bool> guard(m_updatingAxis, true);
    m_base->m_settings->xAxis.autoRange = false;
    m_base->m_settings->yAxis.autoRange = false;
    syncAxisProperties(true, true);
}
void FieldGlyphWidget::zoomOut(){
    auto* plot=m_base->m_plot;
    const auto x=plot->axisScaleDiv(QwtPlot::xBottom);
    const auto y=plot->axisScaleDiv(QwtPlot::yLeft);
    const double xc=(x.lowerBound()+x.upperBound())/2.0;
    const double yc=(y.lowerBound()+y.upperBound())/2.0;
    const double xh=(x.upperBound()-x.lowerBound())*0.625;
    const double yh=(y.upperBound()-y.lowerBound())*0.625;
    plot->setAxisScale(QwtPlot::xBottom,xc-xh,xc+xh);
    plot->setAxisScale(QwtPlot::yLeft,yc-yh,yc+yh);
    plot->replot();
    QScopedValueRollback<bool> guard(m_updatingAxis, true);
    m_base->m_settings->xAxis.autoRange = false;
    m_base->m_settings->yAxis.autoRange = false;
    syncAxisProperties(true, true);
}

// Mouse wheel over the Qwt canvas zooms the currently displayed field.
// Reuse toolbar slots so axis/property synchronization stays consistent.
bool FieldGlyphWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (m_base && watched == m_base->m_plot->canvas() &&
        event->type() == QEvent::Wheel) {
        auto* wheel = static_cast<QWheelEvent*>(event);
        const int delta = wheel->angleDelta().y();
        if (delta > 0) zoomIn();
        else if (delta < 0) zoomOut();
        if (delta != 0) {
            wheel->accept();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
