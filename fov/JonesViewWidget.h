#ifndef JONESVIEWWIDGET_H
#define JONESVIEWWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPolygonF>
#include <qwt_plot.h>
#include <qwt_plot_item.h>
#include <qwt_scale_map.h>

#include "PlotSetting.h"
#include "QwtPropertyBrowser.h"
#include "PlotBase.h"

class QSplitter;

struct JonesVector {
	double x;
	double y;
	double azimuth;     // »¡¶È
	double ellipticity; // »¡¶È
};

class JonesPlotItem : public QwtPlotItem
{
public:
	JonesPlotItem(double x, double y, double azimuth, double ellipticity,
		const QColor& color, double width, double sizeData,
		double ellMin, double ellMax);
	virtual ~JonesPlotItem() {}

	virtual void draw(QPainter* painter,
		const QwtScaleMap& xMap, const QwtScaleMap& yMap,
		const QRectF& canvasRect) const override;

	void setShowArrow(bool show) { m_showArrow = show; }
	void setEllipseSize(double sizeData) { m_ellipseSize = sizeData; }
	void setEllRange(double minV, double maxV) { m_ellMin = minV; m_ellMax = maxV; }

public:
	double m_x, m_y;
	double m_azimuth;
	double m_ellipticity;
	QColor m_color;
	double m_width;
	double m_ellipseSize;
	double m_ellMin;
	double m_ellMax;

private:
	bool   m_showArrow = true;
};

class JonesViewWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JonesViewWidget(QWidget* parent = nullptr);
	~JonesViewWidget();

	bool loadDataFromFile(const QString& filename);
	void clearData();
	void resetView();
	void autoScaleAxes();
	void saveInitialView();

public Q_SLOTS:
	void slotUpdateItemStyle(int index);
	void zoomIn();
	void zoomOut();
	void fitView();

	void updateXScaleAxes();
	void updateYScaleAxes();
	void updateAxesSettings();
	void applyManualAxisRanges();
	void applyTickFonts();
	void showLegend(bool show);
	void setLegendPosition(bool leftSide);
	void slotDisplayProperties(int);

	void setAllEllipseSize(double sizeData);
	void setAllEllipseShowArrow(bool show);

private:
	void setupFieldViewPlot();
	void setupInteractions();
	void updatePlot(const QVector<JonesVector>& data);
	void clearAllJonesItems();
	void enforceAspectRatio(double aspect = 1.0);

	QwtPlot* m_plot;
	MyPlotPanner* m_panner;
	MyPlotMagnifier* m_magnifier;

	QList<JonesPlotItem*> m_jonesItems;
	QVector<JonesVector> m_jonesData;

	double m_defaultRange = 1.5;
	double m_initialXMin, m_initialXMax;
	double m_initialYMin, m_initialYMax;
	double m_initialXMin_orig, m_initialXMax_orig;
	double m_initialYMin_orig, m_initialYMax_orig;
	double m_current_factor = 1;

	double m_ellipseSize = 0.025;
	bool   m_keepAspect = true;

	PlotSettings* m_settings;
	PlotBase* m_toolBar_plot;
	QwtPlotGrid* m_grid;
	QwtPropertyBrowser* m_simple_browser;
	QSplitter* m_splitter;
};

#endif // JONESVIEWWIDGET_H