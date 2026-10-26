// StringEditorFactory.h
#pragma once
#include "PropertyBrowser/qtpropertymanager.h"
#include "PropertyBrowser/qtbuttonpropertybrowser.h"

#include <QMap>

class StringEditorFactory : public QtAbstractEditorFactory<QtStringPropertyManager>
{
    Q_OBJECT
public:
    StringEditorFactory(QObject* parent = nullptr);

protected:
    QWidget* createEditor(QtStringPropertyManager* manager, QtProperty* property,
        QWidget* parent) override;
    void connectPropertyManager(QtStringPropertyManager* manager) override;
    void disconnectPropertyManager(QtStringPropertyManager* manager) override;

private slots:
    void slotTextChanged(QtProperty* property, const QString& value);
    void slotEditorDestroyed(QObject* object);

private:
    // 手动维护属性 -> 编辑器列表的映射
    QMap<QtProperty*, QList<QWidget*>> m_editorsMap;
};