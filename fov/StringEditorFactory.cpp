// StringEditorFactory.cpp
#include "StringEditorFactory.h"
#include "StringEditWithButton.h"

StringEditorFactory::StringEditorFactory(QObject* parent)
    : QtAbstractEditorFactory<QtStringPropertyManager>(parent)
{
}

QWidget* StringEditorFactory::createEditor(QtStringPropertyManager* manager,
    QtProperty* property, QWidget* parent)
{
    StringEditWithButton* editor = new StringEditWithButton(parent);
    editor->setText(manager->value(property));

    // 将编辑器添加到手动映射中
    m_editorsMap[property].append(editor);

    // 当编辑器内容改变时，更新属性值
    connect(editor, &StringEditWithButton::textChanged,
        [this, manager, property](const QString& text) {
        manager->setValue(property, text);
    });

    // 编辑器销毁时，从手动映射中移除
    connect(editor, &QObject::destroyed, this, &StringEditorFactory::slotEditorDestroyed);

    return editor;
}

void StringEditorFactory::connectPropertyManager(QtStringPropertyManager* manager)
{
    // 当属性值改变时，同步更新所有对应的编辑器
    connect(manager, &QtStringPropertyManager::valueChanged,
        this, &StringEditorFactory::slotTextChanged);
}

void StringEditorFactory::disconnectPropertyManager(QtStringPropertyManager* manager)
{
    disconnect(manager, &QtStringPropertyManager::valueChanged,
        this, &StringEditorFactory::slotTextChanged);
}

void StringEditorFactory::slotTextChanged(QtProperty* property, const QString& value)
{
    // 从手动映射中取出该属性关联的所有编辑器，更新显示
    if (m_editorsMap.contains(property)) {
        const auto editors = m_editorsMap.value(property);
        for (QWidget* w : editors) {
            if (StringEditWithButton* editor = qobject_cast<StringEditWithButton*>(w))
                editor->setText(value);
        }
    }
}

void StringEditorFactory::slotEditorDestroyed(QObject* object)
{
    // 查找该编辑器属于哪个属性，并移除映射
    for (auto it = m_editorsMap.begin(); it != m_editorsMap.end(); ++it) {
        QList<QWidget*>& editors = it.value();
        for (int i = 0; i < editors.size(); ++i) {
            if (editors[i] == object) {
                editors.removeAt(i);
                // 如果该属性下没有编辑器了，可以清理空列表（可选）
                if (editors.isEmpty()) {
                    m_editorsMap.erase(it);
                }
                return;
            }
        }
    }
}