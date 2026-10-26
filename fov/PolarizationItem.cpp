#include "PolarizationItem.h"
#include <QPainter>
#include <qwt_scale_map.h>
#include <cmath>

#include "qwt_text.h"

#define M_PI 3.1415926

PolarizationItem::PolarizationItem(const QVector<PolarizationData>& data)
	: QwtPlotItem(QwtText("Jones Vectors")), m_data(data), m_radius(7.0)
{
	setItemInterest(QwtPlotItem::LegendInterest, true);
}

PolarizationItem::~PolarizationItem() {}

void PolarizationItem::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
	if (m_data.isEmpty()) return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// 使用偏紫色的细线，模拟原图效果
	QColor lineColor(100, 120, 220, 200);
	QPen pen(lineColor, 1.0);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);

	for (const auto &pt : m_data) {
		double px = xMap.transform(pt.pos.x());
		double py = yMap.transform(pt.pos.y());

		// 视口裁剪，提高渲染性能
		if (!canvasRect.contains(px, py)) continue;

		drawPolarization(painter, QPointF(px, py), pt.isLeftCircular);
	}

	painter->restore();
}

void PolarizationItem::drawPolarization(QPainter *painter, const QPointF &pixelPos, bool isLeft) const
{
	// 1. 绘制空心椭圆/圆
	painter->drawEllipse(pixelPos, m_radius, m_radius);

	// 2. 绘制旋转箭头
	double arrowAngle = M_PI / 4.0;
	if (isLeft) arrowAngle = -M_PI / 4.0;

	QPointF arrowPos = pixelPos + QPointF(m_radius * std::cos(arrowAngle), -m_radius * std::sin(arrowAngle));
	double tangentAngle = arrowAngle + (isLeft ? M_PI / 2.0 : -M_PI / 2.0);

	double arrowLen = 3.5;
	double headAngle = M_PI / 5.0;

	QPointF p1 = arrowPos - QPointF(arrowLen * std::cos(tangentAngle - headAngle), -arrowLen * std::sin(tangentAngle - headAngle));
	QPointF p2 = arrowPos - QPointF(arrowLen * std::cos(tangentAngle + headAngle), -arrowLen * std::sin(tangentAngle + headAngle));

	painter->drawLine(arrowPos, p1);
	painter->drawLine(arrowPos, p2);
}