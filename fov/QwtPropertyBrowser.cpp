#include "QwtPropertyBrowser.h"
#include <qwt_legend.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <QFont>
#include <QVBoxLayout>

#include <qwt_text.h>
#include <qwt_symbol.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_legend_label.h>
#include <qwt_plot_grid.h>
#include <qwt_plot.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include "StringEditorFactory.h"

QwtPropertyBrowser::QwtPropertyBrowser(PlotSettings* settings, QwtPlot* plot, QwtPlotGrid* grid, QwtLegend* legend, QWidget* parent)
    : QWidget(parent)
    , m_plot(plot),m_grid(grid),m_legend(legend), m_settings(settings)
   /* , m_currentCurve(nullptr)*/
{
    m_manager = new QtVariantPropertyManager(this);
    connect(m_manager, SIGNAL(valueChanged(QtProperty* ,const QVariant&)),
        this, SLOT(onValueChanged(QtProperty* ,const QVariant&)));


    m_browser = new QtTreePropertyBrowser(this);
    m_browser->setPropertiesWithoutValueMarked(false);


    QtVariantEditorFactory* variantFactory = new QtVariantEditorFactory(this);
    m_browser->setFactoryForManager(m_manager, variantFactory);

    m_stringManager = new QtStringPropertyManager(this);
    connect(m_stringManager, SIGNAL(valueChanged(QtProperty*, const QString&)),
        this, SLOT(onValueChanged(QtProperty*, const QString&)));

    m_stringEditorFactory = new StringEditorFactory(this);
    m_browser->setFactoryForManager(m_stringManager, m_stringEditorFactory);

    m_bBuilding = true;
    createGlobalProperties();
    createAxisProperties();    
    createCurveProperties();
    createPlotItemProperties();
    m_bBuilding = false;

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_browser);

    InitSetupUI();
}

QwtPropertyBrowser::~QwtPropertyBrowser()
{
}

void QwtPropertyBrowser::InitSetupUI()
{
    // 全局属性
    m_gridVisibleProperty->setValue(m_settings->gridVisible);
    m_originProperty->setValue(m_settings->origin);
    //m_titleProperty->setValue(m_settings->title);
    m_stringManager->setValue(m_titleProperty, m_settings->title);
    m_titleFontProperty->setValue(m_settings->titleFont);
    m_titleColorProperty->setValue(m_settings->titleColor);
    m_backgroundColorProperty->setValue(m_settings->backgroundColor);
    //QtVariantProperty* m_canvasBackgroundProperty;
    m_legendVisibleProperty->setValue(m_settings->legend.visible);
    m_legendPositionProperty->setValue(m_settings->legend.position);
    m_legendFontProperty->setValue(m_settings->legend.font);
    m_legendColorProperty->setValue(m_settings->legend.color);

    m_axisProps[QwtPlot::xBottom].visibleProperty->setValue(m_settings->xAxis.visible);
    m_axisProps[QwtPlot::xBottom].titleProperty->setValue(m_settings->xAxis.title);
    m_axisProps[QwtPlot::xBottom].titleFontProperty->setValue(m_settings->xAxis.titleFont);
    m_axisProps[QwtPlot::xBottom].titleColorProperty->setValue(m_settings->xAxis.tickColor);
    m_axisProps[QwtPlot::xBottom].autoRangeProperty->setValue(m_settings->xAxis.autoRange);
    m_axisProps[QwtPlot::xBottom].minProperty->setValue(m_settings->xAxis.min);
    m_axisProps[QwtPlot::xBottom].maxProperty->setValue(m_settings->xAxis.max);
    m_axisProps[QwtPlot::xBottom].stepProperty->setValue(m_settings->xAxis.step);
    m_axisProps[QwtPlot::xBottom].tickFontProperty->setValue(m_settings->xAxis.tickFont);
    m_axisProps[QwtPlot::xBottom].tickColorProperty->setValue(m_settings->xAxis.tickColor);

    m_axisProps[QwtPlot::yLeft].visibleProperty->setValue(m_settings->yAxis.visible);
    m_axisProps[QwtPlot::yLeft].titleProperty->setValue(m_settings->yAxis.title);
    m_axisProps[QwtPlot::yLeft].titleFontProperty->setValue(m_settings->yAxis.titleFont);
    m_axisProps[QwtPlot::yLeft].titleColorProperty->setValue(m_settings->yAxis.tickColor);
    m_axisProps[QwtPlot::yLeft].autoRangeProperty->setValue(m_settings->yAxis.autoRange);
    m_axisProps[QwtPlot::yLeft].minProperty->setValue(m_settings->yAxis.min);
    m_axisProps[QwtPlot::yLeft].maxProperty->setValue(m_settings->yAxis.max);
    m_axisProps[QwtPlot::yLeft].stepProperty->setValue(m_settings->yAxis.step);
    m_axisProps[QwtPlot::yLeft].tickFontProperty->setValue(m_settings->yAxis.tickFont);
    m_axisProps[QwtPlot::yLeft].tickColorProperty->setValue(m_settings->yAxis.tickColor);
}
void QwtPropertyBrowser::onValueChanged(QtProperty* property, const QString& value)
{
    if (m_bBuilding)return;
    Q_UNUSED(property)
        Q_UNUSED(value)

        if (!m_plot)
            return;
        if (property == m_titleProperty)
        {
            m_settings->title = value;
            applyTitleSettings_plot();
        }
}
void QwtPropertyBrowser::onValueChanged(QtProperty* property, const QVariant& value)
{
    if (m_bBuilding)return;
    Q_UNUSED(property)
        Q_UNUSED(value)

        if (!m_plot)
            return;


    // 只处理我们关心的 "标题" 属性
    if (property == m_curveGroupProperty) {
        int selectedIndex = value.toInt();  // 枚举值索引
        loadGroupSettings(selectedIndex);
    }
    else  if (property == m_curveTitleProperty) 
    {      
        int selectedIndex = value.toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex) 
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引
            m_curveGroupEditableProperty->setValue(false);
            loadLineSettings(it.key());
            
        }
    }
    else if (property == m_backgroundColorProperty)
    {
        m_settings->backgroundColor = value.value<QColor>();
        applyBackgroundColor_plot();
    }
    else if (property == m_gridVisibleProperty)
    {
        m_settings->gridVisible = value.toBool();
        applyGridSettings_plot();
    }
    else if (property == m_originProperty)
    {
        m_settings->origin = static_cast<PlotSettings::Origin>(value.toInt());
        applyOrigin_plot();
    }
    else if (property == m_titleFontProperty)
    {
        m_settings->titleFont = value.value<QFont>();
        applyTitleSettings_plot();
    }
    else if (property == m_titleColorProperty)
    {
        m_settings->titleColor = value.value<QColor>();
        applyTitleSettings_plot();
    }
    else if (property == m_legendVisibleProperty)
    {
        m_settings->legend.visible = value.toBool();
        applyLegendSettings_plot();
    }
    else if (property == m_legendPositionProperty)
    {
        m_settings->legend.position = static_cast<QwtPlot::LegendPosition>(value.toInt());
        applyLegendSettings_plot();
    }

    else if (property == m_legendFontProperty)
    {
        m_settings->legend.font = value.value<QFont>();
        applyLegendSettings_plot();
    }
    else if (property == m_legendColorProperty)
    {
        m_settings->legend.color = value.value<QColor>();
        applyLegendSettings_plot();
    }
    else if (property == m_curveVisibleProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();

            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.visible = value.toBool();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.visible = value.toBool();
                updateCurveStyle(index);
            }
        }
        //updateCurveStyle(index);
    }
    else if (property == m_curveLineColorProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.lineColor = value.value<QColor>();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.lineColor = value.value<QColor>();
                updateCurveStyle(index);
            }
            setCurrentCurve(index);
        }
        //updateCurveStyle(index);
    }
    else if (property == m_curveLineCurveProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.useCurve = value.toBool();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.useCurve = value.toBool();
                updateCurveStyle(index);
            }
            setCurrentCurve(index);
        }
        //setCurrentCurve(index);
    }
    else if (property == m_curveLineWidthProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.lineWidth = value.toDouble();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.lineWidth = value.toDouble();
                updateCurveStyle(index);
            }
            setCurrentCurve(index);
        }
        //setCurrentCurve(index);
    }
    else if (property == m_curveLineStyleProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            Qt::PenStyle penStyle = (Qt::PenStyle)value.toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.lineStyle = penStyle;
                        updateCurveStyle(line.index);
                    }
                }
                if (penStyle == Qt::CustomDashLine)
                {
                    m_curveCustomPatternProperty->setEnabled(true);
                }
            }
            else
            {
                m_curveCustomPatternProperty->setEnabled(false);
                m_settings->m_lines[index].m_style.lineStyle = penStyle;
                updateCurveStyle(index);

                if (penStyle == Qt::CustomDashLine)
                {
                    m_curveCustomPatternProperty->setEnabled(true);
                }
            }
            setCurrentCurve(index);
        }
        //setCurrentCurve(index);
    }
    else if (property == m_curveCustomPatternProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            QVector<qreal> pattern = parseDashPattern(value.toString());
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.customDashPattern = pattern;
                        line.m_style.lineStyle = Qt::CustomDashLine;
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.customDashPattern = pattern;
                m_settings->m_lines[index].m_style.lineStyle = Qt::CustomDashLine;

                updateCurveStyle(index);
            }
            setCurrentCurve(index);
        }
        //setCurrentCurve(index);
        //int index = m_curveTitleProperty->value().toInt();
        //QVector<qreal> pattern = parseDashPattern(value.toString());
        //m_settings->m_lines[index].m_style.customDashPattern = pattern;
        //m_settings->m_lines[index].m_style.lineStyle = Qt::CustomDashLine;
        //updateCurveStyle(index);
    }
    else if (property == m_curveSymbolStyleProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.fillPointStyle = (FillPointType)value.toInt();
                        line.setPointFillValue();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                int index = m_curveTitleProperty->value().toInt();
                m_settings->m_lines[index].m_style.fillPointStyle = (FillPointType)value.toInt();
                m_settings->m_lines[index].setPointFillValue();                
                updateCurveStyle(index);
            }
            setCurrentCurve(index); 
        }
    }
    else if (property == m_curveSymbolSizeProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.pointSize = value.toDouble();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.pointSize = value.toDouble();
                updateCurveStyle(index);
            }
            setCurrentCurve(index);
        }
    }
    else if (property == m_curveSymbolColorProperty)
    {
        int selectedIndex = m_curveTitleProperty->value().toInt();  // 枚举值索引

        if (m_CurveTitles.size() > 0 && m_CurveTitles.size() > selectedIndex)
        {
            auto it = std::next(m_CurveTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_curveGroupProperty->value().toInt();
            if (m_settings->m_bCurveGroupEditable)
            {
                for (int i = 0; i < m_settings->m_lines.size(); i++)
                {
                    MTFLine& line = m_settings->m_lines[i];
                    if (line.m_group == groupId)
                    {
                        line.m_style.pointColor = value.value<QColor>();
                        updateCurveStyle(line.index);
                    }
                }
            }
            else
            {
                m_settings->m_lines[index].m_style.pointColor = value.value<QColor>();
                updateCurveStyle(index);
            }
            setCurrentCurve(index);
        }
        //setCurrentCurve(index);
    }
    else if (property == m_curveGroupEditableProperty)
    {
        m_settings->m_bCurveGroupEditable = value.toBool();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].titleProperty)
    {
        m_settings->xAxis.title = value.toString();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].titleColorProperty)
    {
        m_settings->xAxis.titleColor = value.value<QColor>();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].titleFontProperty)
    {
        m_settings->xAxis.titleFont = value.value<QFont>();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].autoRangeProperty)
    {
        m_settings->xAxis.autoRange = value.toBool();
        m_axisProps[QwtPlot::xBottom].minProperty->setEnabled(!value.toBool());
        m_axisProps[QwtPlot::xBottom].maxProperty->setEnabled(!value.toBool());
        m_axisProps[QwtPlot::xBottom].stepProperty->setEnabled(!value.toBool());
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].minProperty)
    {
        m_settings->xAxis.min = value.toDouble();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].maxProperty)
    {
        m_settings->xAxis.max = value.toDouble();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].stepProperty)
    {
        m_settings->xAxis.step = value.toDouble();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].tickColorProperty)
    {
        m_settings->xAxis.tickColor = value.value<QColor>();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].tickFontProperty)
    {
        m_settings->xAxis.tickFont = value.value<QFont>();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::xBottom].visibleProperty)
    {
        m_settings->xAxis.visible = value.toBool();
        applyXAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].titleProperty)
    {
        m_settings->yAxis.title = value.toString();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].titleColorProperty)
    {
        m_settings->yAxis.titleColor = value.value<QColor>();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].titleFontProperty)
    {
        m_settings->yAxis.titleFont = value.value<QFont>();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].autoRangeProperty)
    {
        m_settings->yAxis.autoRange = value.toBool();
        m_axisProps[QwtPlot::yLeft].minProperty->setEnabled(!value.toBool());
        m_axisProps[QwtPlot::yLeft].maxProperty->setEnabled(!value.toBool());
        m_axisProps[QwtPlot::yLeft].stepProperty->setEnabled(!value.toBool());
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].minProperty)
    {
        m_settings->yAxis.min = value.toDouble();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].maxProperty)
    {
        m_settings->yAxis.max = value.toDouble();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].stepProperty)
    {
        m_settings->yAxis.step = value.toDouble();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].tickColorProperty)
    {
        m_settings->yAxis.tickColor = value.value<QColor>();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].tickFontProperty)
    {
        m_settings->yAxis.tickFont = value.value<QFont>();
        applyYAxisSettings_plot();
    }
    else if (property == m_axisProps[QwtPlot::yLeft].visibleProperty)
    {
        m_settings->yAxis.visible = value.toBool();
        applyYAxisSettings_plot();
    }
    else if (property == m_itemGroupEditableProperty)
    {
        m_settings->m_bItemGroupEditable = value.toBool();
    }
    else if (property == m_itemTitleProperty)
    {
        int selectedIndex = value.toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引
            m_itemGroupEditableProperty->setValue(false);
            loadItemSettings(it.key());
        }
    }
    else if (property == m_itemGroupProperty)
    {
        int selectedIndex = value.toInt();  // 枚举值索引

        loadItemGroupSettings(selectedIndex);
    }
    else if (property == m_itemXProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.m_x = value.toDouble();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].m_x = value.toDouble();
                updateItemStyle(index);
            }

            setCurrentItem(index);
        }
    }
    else if (property == m_itemYProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.m_y = value.toDouble();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].m_y = value.toDouble();
                updateItemStyle(index);
            }
            setCurrentItem(index);
        }
    }
    else if (property == m_itemAngleProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.m_angle = value.toDouble();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].m_angle = value.toDouble();
                updateItemStyle(index);
            }
            setCurrentItem(index);
        }
    }
    else if (property == m_itemLengthProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.m_length = value.toDouble();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].m_length = value.toDouble();
                updateItemStyle(index);
            }
            setCurrentItem(index);
        }
    }
    else if (property == m_itemColorProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.m_color = value.value<QColor>();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].m_color = value.value<QColor>();
                updateItemStyle(index);
            }
            setCurrentItem(index);
        }
    }
    else if (property == m_itemWidthProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.m_width = value.toDouble();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].m_width = value.toDouble();
                updateItemStyle(index);
            }
            setCurrentItem(index);
        }
    }
    else if (property == m_itemVisiableProperty)
    {
        int selectedIndex = m_itemTitleProperty->value().toInt();  // 枚举值索引

        if (m_itemTitles.size() > 0 && m_itemTitles.size() > selectedIndex)
        {
            auto it = std::next(m_itemTitles.begin(), selectedIndex); // N-1 转换为 0-based 索引

            int index = it.key();
            int groupId = m_itemGroupProperty->value().toInt();
            if (m_settings->m_bItemGroupEditable)
            {
                for (int i = 0; i < m_settings->m_items.size(); i++)
                {
                    MTFPlotItem& item = m_settings->m_items[i];
                    if (item.m_group == groupId)
                    {
                        item.visible = value.toBool();
                        updateItemStyle(item.index);
                    }
                }
            }
            else
            {
                m_settings->m_items[index].visible = value.toBool();
                updateItemStyle(index);
            }
            setCurrentItem(index);
        }
    }
    m_plot->replot();
    emit propertyChanged();
}

// -------------------- 全局属性 --------------------
void QwtPropertyBrowser::createGlobalProperties()
{
    group_global = m_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "全局设置");

    m_gridVisibleProperty = m_manager->addProperty(QVariant::Bool, "显示网格");
    group_global->addSubProperty(m_gridVisibleProperty);


    QStringList origins;
    origins << "左下角" << "右下角" << "中心";
    m_originProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(),  "原点位置");
    m_originProperty->setAttribute("enumNames", origins);
    group_global->addSubProperty(m_originProperty);

    m_titleProperty = m_stringManager->addProperty("标题");
    group_global->addSubProperty(m_titleProperty);

    m_titleFontProperty = m_manager->addProperty(QVariant::Font, "标题字体");
    group_global->addSubProperty(m_titleFontProperty);

    m_titleColorProperty = m_manager->addProperty(QVariant::Color, "标题颜色");
    group_global->addSubProperty(m_titleColorProperty);

    m_backgroundColorProperty = m_manager->addProperty(QVariant::Color, "背景色");
    group_global->addSubProperty(m_backgroundColorProperty);

    //m_canvasBackgroundProperty = m_manager->addProperty(QVariant::Color, "画布背景色");
    //group_global->addSubProperty(m_canvasBackgroundProperty);

    m_legendVisibleProperty = m_manager->addProperty(QVariant::Bool, "显示图例");
    group_global->addSubProperty(m_legendVisibleProperty);

    QStringList positions;
    positions << "左" << "右" << "上" << "下";
    m_legendPositionProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "图例位置");
    m_legendPositionProperty->setAttribute("enumNames", positions);
    group_global->addSubProperty(m_legendPositionProperty);

    m_legendFontProperty = m_manager->addProperty(QVariant::Font, "图例字体");
    group_global->addSubProperty(m_legendFontProperty);

    m_legendColorProperty = m_manager->addProperty(QVariant::Color, "图例文字颜色");
    group_global->addSubProperty(m_legendColorProperty);

    m_browser->addProperty(group_global);
}

// -------------------- 坐标轴属性 --------------------
void QwtPropertyBrowser::createAxisProperties()
{
    QList<int> axes;
    axes << QwtPlot::xBottom << QwtPlot::xTop << QwtPlot::yLeft << QwtPlot::yRight;
    QStringList axisNames;
    axisNames << "X 底部" << "X 顶部" << "Y 左侧" << "Y 右侧";

    for (int i = 0; i < axes.size(); i=i+2) {
        int axisId = axes[i];
        QString name = axisNames[i];

        QtVariantProperty* group = m_manager->addProperty(QtVariantPropertyManager::groupTypeId(), name);
        m_browser->addProperty(group);

        AxisProperties props;

        props.visibleProperty = m_manager->addProperty(QVariant::Bool, "显示");
        group->addSubProperty(props.visibleProperty);

        props.titleProperty = m_manager->addProperty(QVariant::String, "标题");
        group->addSubProperty(props.titleProperty);

        props.titleFontProperty = m_manager->addProperty(QVariant::Font, "标题字体");
        group->addSubProperty(props.titleFontProperty);

        props.titleColorProperty = m_manager->addProperty(QVariant::Color, "标题颜色");
        group->addSubProperty(props.titleColorProperty);

        props.autoRangeProperty = m_manager->addProperty(QVariant::Bool, "自动范围");
        group->addSubProperty(props.autoRangeProperty);

        props.minProperty = m_manager->addProperty(QVariant::Double, "最小值");
        props.minProperty->setAttribute("minimum", -1e6);
        props.minProperty->setAttribute("maximum", 1e6);
        group->addSubProperty(props.minProperty);

        props.maxProperty = m_manager->addProperty(QVariant::Double, "最大值");
        props.maxProperty->setAttribute("minimum", -1e6);
        props.maxProperty->setAttribute("maximum", 1e6);
        group->addSubProperty(props.maxProperty);

        props.stepProperty = m_manager->addProperty(QVariant::Double, "步长");
        props.stepProperty->setAttribute("minimum", 0);
        props.stepProperty->setAttribute("maximum", 1e6);
        group->addSubProperty(props.stepProperty);

        props.tickFontProperty = m_manager->addProperty(QVariant::Font, "刻度字体");
        group->addSubProperty(props.tickFontProperty);

        props.tickColorProperty = m_manager->addProperty(QVariant::Color, "刻度颜色");
        group->addSubProperty(props.tickColorProperty);


        m_axisProps[axisId] = props;
    }
}


void QwtPropertyBrowser::createPlotItemProperties()
{
    group_item = m_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "当前图形");

    m_itemTitleProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "标题");
    group_item->addSubProperty(m_itemTitleProperty);

    m_itemGroupProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "分组");
    group_item->addSubProperty(m_itemGroupProperty);

    m_itemGroupEditableProperty = m_manager->addProperty(QVariant::Bool, "是否对全组生效");
    group_item->addSubProperty(m_itemGroupEditableProperty);

    m_itemVisiableProperty = m_manager->addProperty(QVariant::Bool, "可见");
    group_item->addSubProperty(m_itemVisiableProperty);

    m_itemXProperty = m_manager->addProperty(QVariant::Double, "X坐标");
    group_item->addSubProperty(m_itemXProperty);

    m_itemYProperty = m_manager->addProperty(QVariant::Double, "Y坐标");
    group_item->addSubProperty(m_itemYProperty);

    m_itemAngleProperty = m_manager->addProperty(QVariant::Double, "角度");;
    group_item->addSubProperty(m_itemAngleProperty);

    m_itemLengthProperty = m_manager->addProperty(QVariant::Double, "长度");
    group_item->addSubProperty(m_itemLengthProperty);

    m_itemColorProperty = m_manager->addProperty(QVariant::Color, "颜色");
    group_item->addSubProperty(m_itemColorProperty);

    m_itemWidthProperty = m_manager->addProperty(QVariant::Double, "宽度");
    group_item->addSubProperty(m_itemWidthProperty);

    m_browser->addProperty(group_item);
}
// -------------------- 曲线属性 --------------------
void QwtPropertyBrowser::createCurveProperties()
{
    group_curve = m_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "当前曲线");

    m_curveGroupProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "分组");
    group_curve->addSubProperty(m_curveGroupProperty);

    m_curveGroupEditableProperty = m_manager->addProperty(QVariant::Bool, "是否对全组生效");
    group_curve->addSubProperty(m_curveGroupEditableProperty);

    m_curveTitleProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "标题");
    group_curve->addSubProperty(m_curveTitleProperty);

    m_curveVisibleProperty = m_manager->addProperty(QVariant::Bool, "可见");
    group_curve->addSubProperty(m_curveVisibleProperty);

    m_curveLineColorProperty = m_manager->addProperty(QVariant::Color, "线条颜色");
    group_curve->addSubProperty(m_curveLineColorProperty);

    m_curveLineWidthProperty = m_manager->addProperty(QVariant::Int, "线条宽度");
    m_curveLineWidthProperty->setAttribute("minimum", 1);
    m_curveLineWidthProperty->setAttribute("maximum", 10);
    group_curve->addSubProperty(m_curveLineWidthProperty);

    m_curveCustomPatternProperty = m_manager->addProperty(QVariant::String, "自定义样式");
    m_curveCustomPatternProperty->setAttribute("placeholderText", QVariant("例如: 5 2 1 2"));
    group_curve->addSubProperty(m_curveCustomPatternProperty);

    QStringList styles;
    styles << "无" << "实线" << "虚线" << "点线" << "点划线" << "点点虚线" << "自定义" << "点";
    m_curveLineStyleProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "线条样式");
    m_curveLineStyleProperty->setAttribute("enumNames", styles);
    group_curve->addSubProperty(m_curveLineStyleProperty);

    QStringList curve_styles;
    curve_styles << "折线" << "曲线";
    m_curveLineCurveProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "绘制样式");
    m_curveLineCurveProperty->setAttribute("enumNames", curve_styles);
    group_curve->addSubProperty(m_curveLineCurveProperty);



    //{"小点", QwtSymbol::Ellipse, true},
    //{ "X形交叉", QwtSymbol::XCross, false },
    //{ "加号十字形符号", QwtSymbol::Cross, false },
    //{ "未填充的圆圈", QwtSymbol::Ellipse, false },
    //{ "填充的圆圈", QwtSymbol::Ellipse, true },
    //{ "未填充的矩形", QwtSymbol::Rect, false },
    //{ "填充的矩形", QwtSymbol::Rect, true },
    //{ "未填充的三角形（尖端在底部）", QwtSymbol::DTriangle, false },
    //{ "填充的三角形", QwtSymbol::DTriangle, true },
    //{ "未填充的菱形", QwtSymbol::Diamond, false },
    //{ "填充的菱形", QwtSymbol::Diamond, true },
    //{ "未填充的星形", QwtSymbol::Star1, false },
    //{ "填充的星形", QwtSymbol::Star1, true }
    QStringList symbolStyles;
    symbolStyles << "小点" << "X形交叉" << "加号十字形符号" << "未填充的圆圈" << "填充的圆圈" << "未填充的矩形" 
        << "X填充的矩形" << "未填充的三角形（尖端在底部）" << "填充的三角形" << "未填充的菱形" 
        << "填充的菱形" << "未填充的星形" << "填充的星形";
    m_curveSymbolStyleProperty = m_manager->addProperty(QtVariantPropertyManager::enumTypeId(), "点样式");
    m_curveSymbolStyleProperty->setAttribute("enumNames", symbolStyles);
    group_curve->addSubProperty(m_curveSymbolStyleProperty);

    m_curveSymbolSizeProperty = m_manager->addProperty(QVariant::Int, "点大小");
    m_curveSymbolSizeProperty->setAttribute("minimum", 0);
    m_curveSymbolSizeProperty->setAttribute("maximum", 20);
    group_curve->addSubProperty(m_curveSymbolSizeProperty);

    m_curveSymbolColorProperty = m_manager->addProperty(QVariant::Color, "点颜色");
    group_curve->addSubProperty(m_curveSymbolColorProperty);

    //m_curveSymbolFilledProperty = m_manager->addProperty(QVariant::Bool, "填充点");
    //group_curve->addSubProperty(m_curveSymbolFilledProperty);

    m_browser->addProperty(group_curve);
}

void QwtPropertyBrowser::updateItemCombo()
{
    m_itemTitles.clear();
    m_itemGroups.clear();
    for (int i = 0; i < m_settings->m_items.size(); ++i) {
        const MTFPlotItem& item = m_settings->m_items[i];
        QString displayName = QString("Item %1").arg(item.index);
        if (!item.label.isEmpty()) displayName += ": " + item.label;
        m_itemTitles[item.index] = displayName;

        if (!m_itemGroups.contains(QString::number(item.m_group)))
        {
            m_itemGroups << QString::number(item.m_group);
        }

    }
    m_itemTitleProperty->setAttribute("enumNames", QStringList(m_itemTitles.values()));
    m_itemGroups.sort();

    //m_settings->m_bCurveGroupEditable
    if (m_itemGroups.size() > 0)
    {
        m_itemGroupProperty->setEnabled(true);
        m_itemGroupProperty->setAttribute("enumNames", m_itemGroups);
    }
    else
    {
        m_itemGroupProperty->setEnabled(false);
    }
}
void QwtPropertyBrowser::updateLineCombo()
{
    m_CurveTitles.clear();
    m_CurveGroups.clear();
    for (int i = 0; i < m_settings->m_lines.size(); ++i) {
        const MTFLine& line = m_settings->m_lines[i];
        QString displayName = QString("Line %1").arg(line.index);
        if (!line.label.isEmpty()) displayName += ": " + line.label;
        m_CurveTitles[line.index] = displayName;

        if (!m_CurveGroups.contains(QString::number(line.m_group)))
        {
            m_CurveGroups << QString::number(line.m_group);
        }

    }
    
    m_curveTitleProperty->setAttribute("enumNames", QStringList(m_CurveTitles.values()));
    m_CurveGroups.sort();

    //m_settings->m_bCurveGroupEditable
    if (m_CurveGroups.size() > 0)
    { 
        m_curveGroupProperty->setEnabled(true); 
        m_curveGroupProperty->setAttribute("enumNames", m_CurveGroups);
    }
    else
    {
        m_curveGroupProperty->setEnabled(false);
    }    
}


void QwtPropertyBrowser::updateCurveStyle(int index)
{
    if (index < 0 || index >= m_settings->m_lines.size()) return;
    MTFLine& line = m_settings->m_lines[index];
    if (!line.curve) return;

    //loadLineSettings(index);

    //DotLine,       DashDotLine,        DashDotDotLine,
    Qt::PenStyle lineStyle = line.m_style.lineStyle;
    if (lineStyle == (Qt::CustomDashLine + 1))
    {
        line.curve->setStyle(QwtPlotCurve::Dots);
    }
    else
    {
        // 线条
        QPen pen = line.curve->pen();
        pen.setWidth(line.m_style.lineWidth);
        pen.setColor(line.m_style.lineColor);
        pen.setStyle(line.m_style.lineStyle);
        if (line.m_style.lineStyle == Qt::CustomDashLine && !line.m_style.customDashPattern.isEmpty()) {
            pen.setDashPattern(line.m_style.customDashPattern);
        }
        line.curve->setPen(pen);
        line.curve->setStyle(QwtPlotCurve::Lines);
    }
    // 曲线/折线
    line.curve->setCurveAttribute(QwtPlotCurve::Fitted, line.m_style.useCurve);

    // 点符号
    if (line.curve->symbol())
    {
        line.curve->setSymbol(nullptr);
    }

    if ((line.m_style.pointStyle != QwtSymbol::NoSymbol) /*&& (lineStyle != (Qt::CustomDashLine + 1))*/) {
        QwtSymbol* sym = new QwtSymbol(line.m_style.pointStyle);
        sym->setSize(line.m_style.pointSize);
        sym->setColor(line.m_style.pointColor);
        QPen symPen = sym->pen();
        symPen.setWidth(line.m_style.pointWidth);
        symPen.setColor(line.m_style.pointColor);
        sym->setPen(symPen);
        QBrush brush;
        line.setPointFillValue();
        brush.setStyle(line.m_style.pointFilled ? Qt::SolidPattern : Qt::NoBrush);
        brush.setColor(line.m_style.pointColor);
        sym->setBrush(brush);
        line.curve->setSymbol(sym);
    }
    else {
        line.curve->setSymbol(nullptr);
    }

    if (line.m_style.visible)
    {
        line.curve->setVisible(true);
    }
    else
    {
        line.curve->setVisible(false);
    }
    m_plot->replot();
}
void QwtPropertyBrowser::loadGroupSettings(int index)
{
    if (index < 0 || index >= m_settings->m_lines.size()) return;

    m_CurveTitles.clear();
  
    for (int i = 0; i < m_settings->m_lines.size(); ++i) {
        const MTFLine& line = m_settings->m_lines[i];
        if (line.m_group == index)
        {
            QString displayName = QString("Line %1").arg(line.index);
            if (!line.label.isEmpty()) displayName += ": " + line.label;
            m_CurveTitles[line.index] = displayName;
        }
    }
    
    m_curveTitleProperty->setAttribute("enumNames", QStringList(m_CurveTitles.values()));

    if (m_CurveTitles.size() > 0)
        setCurrentCurve(m_CurveTitles.keys()[0]);

}
void QwtPropertyBrowser::loadLineSettings(int index) 
{
    if (index < 0 || index >= m_settings->m_lines.size()) return;
    MTFLine& line = m_settings->m_lines[index];

    m_curveVisibleProperty->setValue(line.m_style.visible);
    m_curveLineWidthProperty->setValue(line.m_style.lineWidth);
    // 设置线颜色
    m_curveLineColorProperty->setValue(line.m_style.lineColor);
    m_curveLineStyleProperty->setValue((int)line.m_style.lineStyle);
    m_curveLineCurveProperty->setValue(line.m_style.useCurve);
    // 点样式
    m_curveSymbolStyleProperty->setValue(line.m_style.fillPointStyle);
    // 点大小
    m_curveSymbolSizeProperty->setValue(line.m_style.pointSize);
    // 点颜色
    m_curveSymbolColorProperty->setValue(line.m_style.pointColor);

    m_plot->replot();
};

void QwtPropertyBrowser::updateLegendItemsStyle()
{
    if (!m_legend) return;
    QList<QwtLegendLabel*> labels = m_legend->findChildren<QwtLegendLabel*>();
    for (QwtLegendLabel* label : labels) {
        label->setFont(m_settings->legend.font);
        QPalette pal = label->palette();
        pal.setColor(QPalette::Text, m_settings->legend.color);
        label->setPalette(pal);
    }
    m_plot->replot();
}
void QwtPropertyBrowser::setCurrentCurve(int value)
{
    //lineCombo->setCurrentIndex(index);
    m_curveTitleProperty->setValue(value);
    //onValueChanged(m_curveTitleProperty, value);
}


void QwtPropertyBrowser::loadItemSettings(int index)
{
    if (index < 0 || index >= m_settings->m_items.size()) return;
    const MTFPlotItem& item = m_settings->m_items[index];

    m_itemVisiableProperty->setValue(item.visible);
    m_itemWidthProperty->setValue(item.m_width);
    m_itemXProperty -> setValue(item.m_x);
    m_itemYProperty->setValue(item.m_y);
    m_itemAngleProperty->setValue(item.m_angle);
    m_itemLengthProperty->setValue(item.m_length);
    m_itemColorProperty->setValue(item.m_color);

    m_plot->replot();
}
void QwtPropertyBrowser::loadItemGroupSettings(int index)
{
    if (index < 0 || index >= m_settings->m_items.size()) return;

    m_itemTitles.clear();
    for (int i = 0; i < m_settings->m_items.size(); ++i) {
        const MTFPlotItem& item = m_settings->m_items[i];
        if (item.m_group == index)
        {
            QString displayName = QString("Item %1").arg(item.index);
            if (!item.label.isEmpty()) displayName += ": " + item.label;
            m_itemTitles[item.index] = displayName;
        }
    }
    m_itemTitleProperty->setAttribute("enumNames", QStringList(m_itemTitles.values()));


    if (m_itemTitles.size() > 0)
        setCurrentItem(0);
}
void QwtPropertyBrowser::setCurrentItem(int value)
{
    m_itemTitleProperty->setValue(value);
}
void QwtPropertyBrowser::updateItemStyle(int index)
{
    if (index < 0 || index >= m_settings->m_items.size()) return;
    MTFPlotItem& item = m_settings->m_items[index];
    if (!item.m_item) return;


    //loadItemSettings(m_itemTitleProperty->value().toInt());


    emit signalUpdateItemStyle(index);
}


void QwtPropertyBrowser::applyXAxisSettings()
{
    m_axisProps[QwtPlot::xBottom].visibleProperty->setValue(m_settings->xAxis.visible);
    m_axisProps[QwtPlot::xBottom].titleProperty->setValue(m_settings->xAxis.title);
    QFont font = m_settings->xAxis.titleFont;
    m_axisProps[QwtPlot::xBottom].titleFontProperty->setValue(font);
    QColor color = m_settings->xAxis.titleColor;
    m_axisProps[QwtPlot::xBottom].titleColorProperty->setValue(color);
    m_axisProps[QwtPlot::xBottom].autoRangeProperty->setValue(m_settings->xAxis.autoRange);
    m_axisProps[QwtPlot::xBottom].minProperty->setValue(m_settings->xAxis.min);
    m_axisProps[QwtPlot::xBottom].maxProperty->setValue(m_settings->xAxis.max);
    m_axisProps[QwtPlot::xBottom].stepProperty->setValue(m_settings->xAxis.step);
    font = m_settings->xAxis.tickFont;
    m_axisProps[QwtPlot::xBottom].tickFontProperty->setValue(font);
    color = m_settings->xAxis.tickColor;
    m_axisProps[QwtPlot::xBottom].tickColorProperty->setValue(color);
}
void QwtPropertyBrowser::applyYAxisSettings()
{
    m_axisProps[QwtPlot::yLeft].visibleProperty->setValue(m_settings->yAxis.visible);
    m_axisProps[QwtPlot::yLeft].titleProperty->setValue(m_settings->yAxis.title);
    QFont font = m_settings->yAxis.titleFont;
    m_axisProps[QwtPlot::yLeft].titleFontProperty->setValue(font);
    QColor color = m_settings->yAxis.titleColor;
    m_axisProps[QwtPlot::yLeft].titleColorProperty->setValue(color);
    m_axisProps[QwtPlot::yLeft].autoRangeProperty->setValue(m_settings->yAxis.autoRange);
    m_axisProps[QwtPlot::yLeft].minProperty->setValue(m_settings->yAxis.min);
    m_axisProps[QwtPlot::yLeft].maxProperty->setValue(m_settings->yAxis.max);
    m_axisProps[QwtPlot::yLeft].stepProperty->setValue(m_settings->yAxis.step);
    font = m_settings->yAxis.tickFont;
    m_axisProps[QwtPlot::yLeft].tickFontProperty->setValue(font);
    color = m_settings->yAxis.tickColor;
    m_axisProps[QwtPlot::yLeft].tickColorProperty->setValue(color);
}
//只更新plot
void QwtPropertyBrowser::applyBackgroundColor()
{
    m_backgroundColorProperty->setValue(m_settings->backgroundColor);
}

void QwtPropertyBrowser::applyTitleSettings()
{
    //m_titleProperty->setValue(m_settings->title);
    m_stringManager->setValue(m_titleProperty, m_settings->title);
    m_titleFontProperty->setValue(m_settings->titleFont);
    QColor color = m_settings->titleColor;
    m_titleColorProperty->setValue(color);
}
void QwtPropertyBrowser::applyLegendSettings()
{
    QFont font = m_settings->titleFont;
    m_legendFontProperty->setValue(font);
    QColor color = m_settings->titleColor;
    m_legendColorProperty->setValue(color);
    m_legendVisibleProperty->setValue(m_settings->legend.visible);
    m_legendPositionProperty->setValue(m_settings->legend.position);
}
void QwtPropertyBrowser::applyOrigin()
{
    m_originProperty->setValue(m_settings->origin);
}
void QwtPropertyBrowser::autoXScaleAxes()
{
    signalXScaleAxes();
    //double minX = std::numeric_limits<double>::max();
    //double maxX = std::numeric_limits<double>::lowest();
    //bool hasData = false;
    //for (const MTFLine& line : m_settings->m_lines) {
    //    if (!line.m_style.visible || line.data.isEmpty()) continue;
    //    hasData = true;
    //    for (const QPointF& p : line.data) {
    //        if (p.x() < minX) minX = p.x();
    //        if (p.x() > maxX) maxX = p.x();
    //    }
    //}
    //if (hasData) {
    //    double xMargin = (maxX - minX) * 0.05;
    //    if (xMargin <= 0) xMargin = 1.0;

    //    m_plot->setAxisScale(QwtPlot::xBottom, minX - xMargin, maxX + xMargin);
    //    QwtScaleDiv temp = m_plot->axisScaleDiv(QwtPlot::xBottom);
    //    QList<double> ticks = temp.ticks(QwtScaleDiv::MajorTick);
    //    m_settings->xAxis.min = minX - xMargin;
    //    m_settings->xAxis.max = maxX + xMargin;

    //    m_settings->xAxis.step = ticks[1] - ticks[0];
    //}
    //else {
    //    m_settings->xAxis.min = m_defaultXMin;
    //    m_settings->xAxis.max = m_defaultXMax;
    //}
}
void QwtPropertyBrowser::autoYScaleAxes()
{
    signalYScaleAxes();
    //double minY = std::numeric_limits<double>::max();
    //double maxY = std::numeric_limits<double>::lowest();
    //bool hasData = false;
    //for (const MTFLine& line : m_settings->m_lines) {
    //    if (!line.m_style.visible || line.data.isEmpty()) continue;
    //    hasData = true;
    //    for (const QPointF& p : line.data) {
    //        if (p.y() < minY) minY = p.y();
    //        if (p.y() > maxY) maxY = p.y();
    //    }
    //}
    //if (hasData) {
    //    double yMargin = (maxY - minY) * 0.05;
    //    if (yMargin <= 0) yMargin = 0.1;
    //    m_plot->setAxisScale(QwtPlot::yLeft, std::max(0.0, minY - yMargin), maxY + yMargin);
    //    QwtScaleDiv temp = m_plot->axisScaleDiv(QwtPlot::xBottom);
    //    QList<double> ticks = temp.ticks(QwtScaleDiv::MajorTick);

    //    m_settings->yAxis.min = minY - yMargin;
    //    m_settings->yAxis.max = maxY + yMargin;
    //    temp = m_plot->axisScaleDiv(QwtPlot::yLeft);
    //    ticks = temp.ticks(QwtScaleDiv::MajorTick);
    //    m_settings->yAxis.step = ticks[1] - ticks[0];

    //}
    //else {
    //    m_settings->yAxis.min = m_defaultYMin;
    //    m_settings->yAxis.max = m_defaultYMax;
    //}
}

void QwtPropertyBrowser::applyGridSettings()
{
    m_gridVisibleProperty->setValue(m_settings->gridVisible);
}

void QwtPropertyBrowser::applyXAxisSettings_plot()
{    // X轴
    if (m_settings->xAxis.visible)
    {
        m_plot->enableAxis(QwtPlot::xBottom);
        QwtText xTitle(m_settings->xAxis.title);
        xTitle.setFont(m_settings->xAxis.titleFont);
        xTitle.setColor(m_settings->xAxis.titleColor);
        m_plot->setAxisTitle(QwtPlot::xBottom, xTitle);
        //m_plot->setAxisFont(QwtPlot::xBottom, m_settings->xAxis.titleFont);
        QwtScaleWidget* xScale = m_plot->axisWidget(QwtPlot::xBottom);
        if (xScale) {
			xScale->setFont(m_settings->xAxis.tickFont);
            QPalette pal = xScale->palette();
            pal.setColor(QPalette::Text, m_settings->xAxis.tickColor);
            xScale->setPalette(pal);
        }
        if (!m_settings->xAxis.autoRange)
        {
            //m_plot->setAxisScale(QwtPlot::xBottom, m_settings->xAxis.min, m_settings->xAxis.max, m_settings->xAxis.step);
            m_plot->replot();
            signalUpdateScaleDiv();
        }
        else
        {
            autoXScaleAxes();
            //m_plot->setAxisScale(QwtPlot::xBottom, m_settings->xAxis.min, m_settings->xAxis.max);
        }
    }
    else 
    {
        m_plot->enableAxis(QwtPlot::xBottom, false);
    }


    m_plot->replot();
}
void QwtPropertyBrowser::applyYAxisSettings_plot()
{
    // Y轴
    if (m_settings->yAxis.visible)
    {
        m_plot->enableAxis(QwtPlot::yLeft);
        QwtText yTitle(m_settings->yAxis.title);
        yTitle.setFont(m_settings->yAxis.titleFont);
        yTitle.setColor(m_settings->yAxis.titleColor);
        m_plot->setAxisTitle(QwtPlot::yLeft, yTitle);
        //m_plot->setAxisFont(QwtPlot::yLeft, m_settings->yAxis.titleFont);
        QwtScaleWidget* yScale = m_plot->axisWidget(QwtPlot::yLeft);
        if (yScale) {
			yScale->setFont(m_settings->yAxis.tickFont);
            QPalette pal = yScale->palette();
            pal.setColor(QPalette::Text, m_settings->yAxis.tickColor);
            yScale->setPalette(pal);
        }
        if (!m_settings->yAxis.autoRange)
        {
            //m_plot->setAxisScale(QwtPlot::yLeft, m_settings->yAxis.min, m_settings->yAxis.max, m_settings->yAxis.step);
            m_plot->replot();
            signalUpdateScaleDiv();
        }
        else
        {
            autoYScaleAxes();
        }
    }
    else {
        m_plot->enableAxis(QwtPlot::yLeft, false);
    }
    m_plot->replot();
}
void QwtPropertyBrowser::applyBackgroundColor_plot()
{
    m_plot->setCanvasBackground(m_settings->backgroundColor);
}
void QwtPropertyBrowser::applyGridSettings_plot()
{
    if (m_settings->gridVisible)
    {
        m_grid->enableX(true);
        m_grid->enableY(true);
        m_grid->setMajorPen(QPen(m_settings->gridMajorColor, 0, m_settings->gridMajorStyle));
        m_grid->setMinorPen(QPen(m_settings->gridMinorColor, 0, m_settings->gridMinorStyle));
        m_grid->attach(m_plot);
    }
    else {
        m_grid->detach();
    }
    m_plot->replot();
}
void QwtPropertyBrowser::applyTitleSettings_plot()
{
    QwtText title(m_settings->title);
    title.setFont(m_settings->titleFont);
    title.setColor(m_settings->titleColor);
    m_plot->setTitle(title);
    m_plot->replot();
}
void QwtPropertyBrowser::applyLegendSettings_plot()
{
    if (m_settings->legend.visible)
    {
        // 显示图例：如果之前移除了，需要重新插入；如果已存在，直接显示并更新样式
        if (!m_legend) {
            m_legend = new QwtLegend();
            // 可能需要对图例进行一些初始化设置
        }
        m_plot->insertLegend(m_legend, m_settings->legend.position);
        
        m_legend->setVisible(true);
        updateLegendItemsStyle();
    }
    else
    {
        // 隐藏图例：从 Plot 中移除，完全消失
        delete m_legend;
        m_plot->insertLegend(nullptr);
        m_legend = NULL;
        // 如果 m_legend 是你管理的成员，可以保留对象以便下次快速显示，
        // 或者直接删除并置空（取决于你的内存管理策略）
        // 这里假设保留 m_legend 对象，仅从 plot 中移除
    }
    emit signalUpdateLegend();
    //if (m_settings->legend.visible)
    //{
    //    m_legend->setVisible(true);
    //    m_plot->insertLegend(m_legend, m_settings->legend.position);
    //    updateLegendItemsStyle();
    //}
    //else
    //{
    //    m_legend->setVisible(false);
    //    
    //}
    m_plot->replot();
}
void QwtPropertyBrowser::applyOrigin_plot()
{
    QwtScaleDiv xDiv = m_plot->axisScaleDiv(QwtPlot::xBottom);
    QwtScaleDiv yDiv = m_plot->axisScaleDiv(QwtPlot::yLeft);
    switch (m_settings->origin) {
    case PlotSettings::BottomLeft:
    {
        // 取消反转
        m_plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Inverted, false);
        m_plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, false);
        // 确保范围是正序
        if (xDiv.lowerBound() > xDiv.upperBound()) {
            m_plot->setAxisScale(QwtPlot::xBottom, xDiv.upperBound(), xDiv.lowerBound());
        }
        else
        {
            m_plot->setAxisScale(QwtPlot::xBottom, xDiv.lowerBound(), xDiv.upperBound());
        }
        if (yDiv.lowerBound() > yDiv.upperBound())
        {
            m_plot->setAxisScale(QwtPlot::yLeft, yDiv.upperBound(), yDiv.lowerBound());
        }
        else
        {
            m_plot->setAxisScale(QwtPlot::yLeft, yDiv.lowerBound(), yDiv.upperBound());
        }
    }
    break;
    case PlotSettings::BottomRight:
    {
        // 反转 X 轴，Y 轴不反转
        m_plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Inverted, true);
        m_plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, false);
        // 设置范围为从小到大的值，但引擎会负责反转
        if (xDiv.lowerBound() > xDiv.upperBound()) {
            m_plot->setAxisScale(QwtPlot::xBottom, xDiv.lowerBound(), xDiv.upperBound());
        }
        else
        {
            m_plot->setAxisScale(QwtPlot::xBottom, xDiv.upperBound(), xDiv.lowerBound());
        }
        if (yDiv.lowerBound() > yDiv.upperBound())
        {
            m_plot->setAxisScale(QwtPlot::yLeft, yDiv.upperBound(), yDiv.lowerBound());
        }
        else
        {
            m_plot->setAxisScale(QwtPlot::yLeft, yDiv.lowerBound(), yDiv.upperBound());
        }
    }
    break;
    case PlotSettings::Center:
    {
        // 取消反转，设置对称范围
        m_plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Inverted, false);
        m_plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, false);

        // 计算数据范围（与之前相同）
        //double xMin, xMax, yMin, yMax;
        //if (m_settings->m_lines.isEmpty()) {
        //    xMin = m_defaultXMin;
        //    xMax = m_defaultXMax;
        //    yMin = m_defaultYMin;
        //    yMax = m_defaultYMax;
        //}
        //else {
        //    xMin = std::numeric_limits<double>::max();
        //    xMax = std::numeric_limits<double>::lowest();
        //    yMin = std::numeric_limits<double>::max();
        //    yMax = std::numeric_limits<double>::lowest();
        //    bool hasData = false;
        //    for (const MTFLine& line : m_settings->m_lines) {
        //        if (!line.m_style.visible || line.data.isEmpty()) continue;
        //        hasData = true;
        //        for (const QPointF& p : line.data) {
        //            if (p.x() < xMin) xMin = p.x();
        //            if (p.x() > xMax) xMax = p.x();
        //            if (p.y() < yMin) yMin = p.y();
        //            if (p.y() > yMax) yMax = p.y();
        //        }
        //    }
        //    if (!hasData) {
        //        xMin = m_defaultXMin;
        //        xMax = m_defaultXMax;
        //        yMin = m_defaultYMin;
        //        yMax = m_defaultYMax;
        //    }
        //}
        //double xRange = qMax(qAbs(xMin), qAbs(xMax)) * 1.05;
        //double yRange = qMax(qAbs(yMin), qAbs(yMax)) * 1.05;
        //m_plot->setAxisScale(QwtPlot::xBottom, -xRange, xRange);
        //m_plot->setAxisScale(QwtPlot::yLeft, -yRange, yRange);
        double xRange = xDiv.upperBound() - xDiv.lowerBound();
        double yRange = yDiv.upperBound() - yDiv.lowerBound();
        m_plot->setAxisScale(QwtPlot::xBottom, -xRange/2, xRange/2);
        m_plot->setAxisScale(QwtPlot::yLeft, -yRange/2, yRange/2);
    }
    break;
    }
    m_plot->replot();    
    signalUpdateScaleDiv();
}

QVector<qreal> QwtPropertyBrowser::parseDashPattern(const QString& text)
{
    QVector<qreal> pattern;
    QStringList parts = text.split(' ', QString::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok;
        qreal v = part.toDouble(&ok);
        if (ok && v > 0) pattern.append(v);
    }
    if (pattern.size() % 2 != 0) pattern.append(pattern.last()); // 简单补全
    return pattern;
}