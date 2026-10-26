#pragma once

#include <QWidget>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_picker.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_picker_machine.h>
#include <qwt_text.h>
#include <qwt_symbol.h>

#include "PlotSetting.h"
class QToolBar;

class PlotBase : public QWidget
{
    Q_OBJECT
public:
    explicit PlotBase(QWidget* parent = nullptr);
    virtual ~PlotBase();

public Q_SLOTS:
    // 保存SVG
    void saveAsSVG(const QString& fileName);
    void zoomIn();
    void zoomOut();
    void fitView();
    void on_display_chk_StateChanged(int);
Q_SIGNALS:
    void signalZoomIn();
    void signalZoomOut();
    void signalFitView();
    void signalDisplayProperties(int);
public:
    QwtPlot* m_plot;

    QwtPlotGrid* m_grid;
    QwtLegend* m_legend;
    QToolBar* m_toolBar;

    PlotSettings* m_settings;

    // 默认范围
    double m_defaultXMin = 0.0, m_defaultXMax = 200.0;
    double m_defaultYMin = 0.0, m_defaultYMax = 1.1;
};