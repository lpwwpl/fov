#ifndef FIELDGLYPHWIDGET_H
#define FIELDGLYPHWIDGET_H
#include <QWidget>
#include <QVector>
#include <QColor>
#include <qwt_plot_item.h>
#include "PlotBase.h"
#include "QwtPropertyBrowser.h"

// Reusable QwtPlotItem; each property-browser entry points to one item.
class FieldGlyphItem : public QwtPlotItem {
public:
    enum Shape { Circle, Line, Cone };
    FieldGlyphItem(Shape shape, double x, double y, double angleDeg,
                   double size, QColor color, double width);
    void draw(QPainter*, const QwtScaleMap&, const QwtScaleMap&, const QRectF&) const override;
    Shape shape;
    double x, y, angle, length, width;
    QColor color;
};

class QWidget;
class FieldGlyphWidget : public QWidget {
    Q_OBJECT
public:
    explicit FieldGlyphWidget(FieldGlyphItem::Shape shape, QWidget* parent = nullptr);
    ~FieldGlyphWidget() override;
    bool loadFieldDataFile(const QString& path);
    void clearData();
    QwtPropertyBrowser* propertyBrowser() const { return m_browser; }
public slots:
    void slotUpdateItemStyle(int index);
    void slotDisplayProperties(int state);
    void zoomIn();
    void zoomOut();
    void fitView();
    void updateXScaleAxes();
    void updateYScaleAxes();
    void updateScaleDiv();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
private:
    void applyAxes();
    void syncAxisProperties(bool xAxis, bool yAxis);
    void autoAxis(bool xAxis);
    FieldGlyphItem::Shape m_shape;
    PlotBase* m_base;
    QwtPropertyBrowser* m_browser;

    QList<FieldGlyphItem*> m_items;
    double m_xMin=-16, m_xMax=16, m_yMin=-16, m_yMax=16;
    bool m_updatingAxis=false;
};
#endif
