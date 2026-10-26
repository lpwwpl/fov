#ifndef POLARIZATIONITEM_H
#define POLARIZATIONITEM_H

#include <qwt_plot_item.h>
#include <QVector>
#include <QPointF>
#include <complex>

struct PolarizationData {
	QPointF pos;               // 物理坐标 (x, y)
	std::complex<double> Ex;   // X方向偏振分量
	std::complex<double> Ey;   // Y方向偏振分量
	bool isLeftCircular;       // 是否左旋 (决定箭头方向)
};

class PolarizationItem : public QwtPlotItem
{
public:
	explicit PolarizationItem(const QVector<PolarizationData>& data);
	virtual ~PolarizationItem();

	virtual void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const override;

	void setRadius(double radius) { m_radius = radius; }

private:
	QVector<PolarizationData> m_data;
	double m_radius;

	void drawPolarization(QPainter *painter, const QPointF &pixelPos, bool isLeft) const;
};

#endif // POLARIZATIONITEM_H