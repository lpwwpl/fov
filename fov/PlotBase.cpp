#include "PlotBase.h"

#include <QToolBar>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMenu>
#include <QCheckBox>
#include <qwt_text.h>
#include <qwt_symbol.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
PlotBase::PlotBase(QWidget* parent) : QWidget(parent)
, m_plot(nullptr)
, m_grid(nullptr)
, m_legend(nullptr)
, m_toolBar(nullptr)
{
    //QWidget* plotWidget = new QWidget();
    QVBoxLayout* plotLayout = new QVBoxLayout(this);
    plotLayout->setContentsMargins(0, 0, 0, 0);

    // 创建绘图区域
    m_plot = new QwtPlot(this);
    m_plot->setCanvasBackground(Qt::white);
    m_plot->setMinimumSize(400, 300);
    //m_plot->setAxisTitle(QwtPlot::xBottom, "空间频率 (cycles/mm)");
    //m_plot->setAxisTitle(QwtPlot::yLeft, "MTF值");
    m_plot->setAxisScale(QwtPlot::xBottom, m_defaultXMin, m_defaultXMax);
    m_plot->setAxisScale(QwtPlot::yLeft, m_defaultYMin, m_defaultYMax);

    // 网格
    m_grid = new QwtPlotGrid();
    m_grid->enableX(true);
    m_grid->enableY(true);
    m_grid->setMajorPen(QPen(Qt::lightGray, 0.5, Qt::SolidLine));
    m_grid->setMinorPen(QPen(Qt::lightGray, 0.5, Qt::DotLine));
    m_grid->attach(m_plot);

    // 图例
    m_legend = new QwtLegend();
    m_legend->setDefaultItemMode(QwtLegendData::Checkable);
    m_plot->insertLegend(m_legend, QwtPlot::RightLegend);
    m_legend->setFont(QFont("Helvetica", 9));
    // 工具栏
    m_toolBar = new QToolBar(this);
    m_toolBar->addAction(tr("自适应"), this, &PlotBase::fitView);
    m_toolBar->addAction(tr("放大"), this, &PlotBase::zoomIn);
    m_toolBar->addAction(tr("缩小"), this, &PlotBase::zoomOut);



    m_toolBar->addSeparator();
    QAction* saveSVGAction = m_toolBar->addAction(tr("保存SVG"), [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("保存为SVG"), QString(), tr("SVG文件 (*.svg)"));
        if (!fileName.isEmpty()) saveAsSVG(fileName);
        });
    saveSVGAction->setToolTip(tr("将当前图像保存为SVG文件"));

    QCheckBox* display_chk = new QCheckBox("显示属性", this);
    display_chk->setChecked(true);
    m_toolBar->addWidget(display_chk);

    connect(display_chk, &QCheckBox::stateChanged, this, &PlotBase::on_display_chk_StateChanged);

    plotLayout->addWidget(m_toolBar);
    plotLayout->addWidget(m_plot);

    m_settings = new PlotSettings();
}

PlotBase::~PlotBase()
{

}
void PlotBase::on_display_chk_StateChanged(int value)
{
    signalDisplayProperties(value);
}
void PlotBase::saveAsSVG(const QString& fileName)
{
    QwtPlotRenderer renderer;
    renderer.renderDocument(m_plot, fileName, QSizeF(300, 200), 85);
}

void PlotBase::zoomIn()
{
    signalZoomIn();
}
void PlotBase::zoomOut()
{
    signalZoomOut();
}
void PlotBase::fitView()
{
    signalFitView();
}