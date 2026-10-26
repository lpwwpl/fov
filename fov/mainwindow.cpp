#include "mainwindow.h"
#include "CircleViewWidget.h"
#include "LineViewWidget.h"
#include "ConeViewWidget.h"
#include "FieldViewWidget.h"
#include "JonesViewWidget.h"
#include "PlotBase.h"
#include "QwtPropertyBrowser.h"
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QDir>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QKeySequence>
#include <QToolBar>
#include <QCheckBox>
#include <QLayout>
#include <QSignalBlocker>
#include <QtMath>
#include <cmath>

// In keeping with DeformationWidget::setupUI(), the MainWindow owns
// one splitter and one right-side control layout. The existing view
// implementations (including FieldViewWidget) are not modified.
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QWidget* central = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Horizontal, central);
    QWidget* leftPanel = new QWidget(m_splitter);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    // Keep each entire view (and its PlotBase toolbar) in the left stack.
    m_plotStack = new QStackedWidget(leftPanel);
    leftLayout->addWidget(m_plotStack, 1);
    m_controlWidget = new QWidget(m_splitter);
    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlWidget);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    controlLayout->setSpacing(5);

    QLabel* typeLabel = new QLabel(QString::fromUtf8("视场图类型"), m_controlWidget);
    m_fieldTypeCombo = new QComboBox(m_controlWidget);
	m_fieldTypeCombo->addItems(QStringList()
		<< QString::fromUtf8("空心圆") << QString::fromUtf8("直线")
		<< QString::fromUtf8("箭头") << QString::fromUtf8("锥角"));
    controlLayout->addWidget(typeLabel);
    controlLayout->addWidget(m_fieldTypeCombo);
    m_propertyStack = new QStackedWidget(m_controlWidget);
    controlLayout->addWidget(m_propertyStack, 1);

    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(m_controlWidget);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setCollapsible(1, false);
    m_controlWidget->setMinimumWidth(300);
    m_controlWidget->setMaximumWidth(400);
    m_splitter->setSizes({900, 300});
    mainLayout->addWidget(m_splitter);
    setCentralWidget(central);

    // Instantiate original independent view controllers. Only their existing
    // PlotBase and QwtPropertyBrowser widgets are moved to the shared layout.
    m_circle = new CircleViewWidget(this);
    m_line = new LineViewWidget(this);
    m_arrow = new FieldViewWidget(this);
    m_cone = new ConeViewWidget(this);

    addView(m_circle);
    addView(m_line);
    addView(m_arrow);
    addView(m_cone);
    //addView(m_jones);

    connect(m_fieldTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::setFieldType);
    setFieldType(0);

    QMenu* fileMenu = menuBar()->addMenu(QString::fromUtf8("文件(&F)"));
    QAction* openAction = fileMenu->addAction(QString::fromUtf8("打开文件(&O)..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFieldFile);
    QAction* jonesAction = fileMenu->addAction(QString::fromUtf8("导入Jones..."));
    connect(jonesAction, &QAction::triggered, this, &MainWindow::importJones);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(QString::fromUtf8("退出(&X)"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    setWindowTitle(QString::fromUtf8("FOV 视场图"));
    resize(1200, 820);
}

void MainWindow::addView(QWidget* view)
{
    // Preserve the entire original view hierarchy, including PlotBase,
    // its toolbar, its layout and the existing signal/slot connections.
    // Reparenting PlotBase on its own can interfere with its old splitter.
    QwtPropertyBrowser* browser = view->findChild<QwtPropertyBrowser*>();
    Q_ASSERT(browser);
    if (!browser) return;

    // Only relocate the property browser to the right side.
    // Qt automatically removes it from its former layout on reparenting.
    m_propertyStack->addWidget(browser);

    // The old FieldGlyphWidget right-side panel is now empty; hide it,
    // so it cannot intercept mouse events or consume plot space.
    // FieldViewWidget has the browser directly in its splitter, which
    // automatically drops that pane when the browser is reparented.
    for (QSplitter* splitter : view->findChildren<QSplitter*>()) {
        for (int i = 0; i < splitter->count(); ++i) {
            QWidget* pane = splitter->widget(i);
            if (pane && pane != view->findChild<PlotBase*>() &&
                !pane->findChild<PlotBase*>()) {
                pane->hide();
            }
        }
        if (splitter->count() > 1)
            splitter->setSizes({1000, 0});
    }

    m_plotStack->addWidget(view);
    connect(view->findChild<PlotBase*>(), SIGNAL(signalDisplayProperties(int)),
            this, SLOT(setPropertiesVisible(int)));
}

void MainWindow::setPropertiesVisible(int state)
{
    m_controlWidget->setVisible(state != 0);
}

void MainWindow::openFieldFile(){
    const QString path = QFileDialog::getOpenFileName(
        this, QString::fromUtf8("打开视场数据文件"), QString(),
        QString::fromUtf8("视场数据 (*.txt);;所有文件 (*)"));
    if(path.isEmpty()) return; // 取消时保持当前视图不变。

    // 先确认文件中至少包含一条合法数据，以免误选文件清空现有图像。
    QFile input(path);
    if(!input.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,QString::fromUtf8("打开失败"),
                             QString::fromUtf8("无法读取文件：\n%1").arg(path));
        return;
    }
    QTextStream stream(&input);
    bool valid = false;
    while(!stream.atEnd()){
        const QStringList parts=stream.readLine().trimmed().split(
            QRegularExpression(QString::fromUtf8("\\s+")), QString::SkipEmptyParts);
        if(parts.size()<4) continue;
        bool ok0=false,ok1=false,ok2=false,ok3=false;
        parts[0].toInt(&ok0); parts[1].toDouble(&ok1);
        parts[2].toDouble(&ok2); parts[3].toDouble(&ok3);
        if(ok0&&ok1&&ok2&&ok3){valid=true;break;}
    }
    input.close();
    if(!valid){
        QMessageBox::warning(this,QString::fromUtf8("格式错误"),
                             QString::fromUtf8("文件未包含有效数据（index X-Field Y-Field Magnitude）。"));
        return;
    }

    const QString newArrowTemp=prepareArrowFile(path);
    if(newArrowTemp.isEmpty()){
        QMessageBox::warning(this,QString::fromUtf8("加载失败"),
                             QString::fromUtf8("无法为箭头视图准备数据文件。"));
        return;
    }
    m_circle->loadFieldDataFile(path);
    m_line->loadFieldDataFile(path);
    m_cone->loadFieldDataFile(path);
    m_arrow->loadFieldDataFile(newArrowTemp);
    const QString oldTemp=m_arrowTemp;
    m_arrowTemp=newArrowTemp;
    if(!oldTemp.isEmpty()) QFile::remove(oldTemp);
    setWindowTitle(QString::fromUtf8("FOV 视场图 - %1").arg(QFileInfo(path).fileName()));
}

MainWindow::~MainWindow()
{
    // The browsers are editors for data owned by their respective views.
    // Return each editor to its owner *before* Qt tears down the two stacks.
    // Otherwise a browser may outlive its view and retain invalid PlotSettings.
    QWidget* views[] = {m_circle, m_line, m_arrow, m_cone, m_jones};
    for (int i = 0; i < 5; ++i) {
        QWidget* view = views[i];
        if (!view || !m_propertyStack || m_propertyStack->count() == 0) continue;
        // removeWidget shifts later indexes, so always take the first.
        QWidget* editor = m_propertyStack->widget(0);
        m_propertyStack->removeWidget(editor);
        // Restore the browser to its owning view; it can no longer
        // outlive PlotBase or PlotSettings when the MainWindow closes.
        editor->setParent(view);
        editor->hide();
    }

    // Destroy viewers before the central widget; Qt then destroys their
    // restored editor widgets along with each viewer.
    delete m_circle; m_circle = nullptr;
    delete m_line;   m_line = nullptr;
    delete m_arrow;  m_arrow = nullptr;
    delete m_cone;   m_cone = nullptr;
    delete m_jones; m_jones = nullptr;
    if (!m_arrowTemp.isEmpty()) QFile::remove(m_arrowTemp);
}
QString MainWindow::prepareArrowFile(const QString& path){
    QFile input(path);if(!input.open(QIODevice::ReadOnly|QIODevice::Text))return {};
    QTemporaryFile output(QDir::tempPath()+QString::fromUtf8("/fov_arrow_XXXXXX.txt"));
    output.setAutoRemove(false);if(!output.open())return {};
    QTextStream in(&input),out(&output);
    out<<"index X-Field Y-Field Magnitude Angle\n";
    while(!in.atEnd()){
        QStringList parts=in.readLine().trimmed().split(QRegularExpression(QString::fromUtf8("\\s+")),QString::SkipEmptyParts);
        if(parts.size()<4)continue;
        bool ok1=false,ok2=false,ok3=false,ok4=false;
        parts[0].toInt(&ok1);double x=parts[1].toDouble(&ok2);
        double y=parts[2].toDouble(&ok3);parts[3].toDouble(&ok4);
        if(!(ok1&&ok2&&ok3&&ok4))continue;
        double angle=parts.size()>=5?parts[4].toDouble():qRadiansToDegrees(std::atan2(y,x));
        out<<parts[0]<<' '<<parts[1]<<' '<<parts[2]<<' '<<parts[3]<<' '<<angle<<'\n';
    }
    out.flush();QString result=output.fileName();output.close();return result;
}

void MainWindow::setFieldType(int index)
{
    if (index < 0 || index >= m_plotStack->count()
        || index >= m_propertyStack->count()) return;
    m_plotStack->setCurrentIndex(index);
    m_propertyStack->setCurrentIndex(index);
}

void MainWindow::importJones() {
    const QString path=QFileDialog::getOpenFileName(this,QString::fromUtf8("导入 Jones 数据"),QString(),QString::fromUtf8("文本文件 (*.txt);;所有文件 (*)"));
    if(path.isEmpty())return;
    QString error;
	m_jones = new JonesViewWidget();

	m_jones->resize(1200, 800); // 设置宽屏比例，与原图一致
    if(!m_jones->loadDataFromFile(path)) {
        QMessageBox::warning(this,QString::fromUtf8("Jones 导入失败"),error);
        return;
    }

	m_jones->show();
    m_fieldTypeCombo->setCurrentIndex(4);
    setWindowTitle(QString::fromUtf8("FOV - Jones - %1").arg(QFileInfo(path).fileName()));
}
