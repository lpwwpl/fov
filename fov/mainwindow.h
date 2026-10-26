#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
class QStackedWidget;
class QComboBox;
class QSplitter;
class QWidget;
class CircleViewWidget;
class LineViewWidget;
class ConeViewWidget;
class FieldViewWidget;
class JonesViewWidget;
class QwtPropertyBrowser;
class PlotBase;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
private slots:
    void openFieldFile();
    void importJones();
    void setFieldType(int index);
    void setPropertiesVisible(int state);
private:
    QString prepareArrowFile(const QString& source);
    void addView(QWidget* view);
    QSplitter* m_splitter = nullptr;
    QStackedWidget* m_plotStack = nullptr;
    QStackedWidget* m_propertyStack = nullptr;
    QComboBox* m_fieldTypeCombo = nullptr;
    QWidget* m_controlWidget = nullptr;
    CircleViewWidget* m_circle = nullptr;
    LineViewWidget* m_line = nullptr;
    FieldViewWidget* m_arrow = nullptr;
    JonesViewWidget* m_jones = nullptr;
    ConeViewWidget* m_cone = nullptr;
    QString m_arrowTemp;
};
#endif
