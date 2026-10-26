// StringEditWithButton.cpp
#include "StringEditWithButton.h"

StringEditWithButton::StringEditWithButton(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    lineEdit = new QLineEdit(this);
    lineEdit->setReadOnly(true); // 让用户只能通过对话框编辑
    button = new QPushButton("...", this);
    button->setFixedWidth(30);

    layout->addWidget(lineEdit);
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, &StringEditWithButton::onButtonClicked);
}

QString StringEditWithButton::text() const
{
    return lineEdit->text();
}

void StringEditWithButton::setText(const QString& text)
{
    lineEdit->setText(text);
}

void StringEditWithButton::onButtonClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Edit String"));
    dialog.resize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setPlainText(lineEdit->text());

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    layout->addWidget(textEdit);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString newText = textEdit->toPlainText();
        lineEdit->setText(newText);
        emit textChanged(newText);
    }
}