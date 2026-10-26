// StringEditWithButton.h
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>
#include <QTextEdit>
#include <QDialogButtonBox>

class StringEditWithButton : public QWidget
{
    Q_OBJECT
public:
    explicit StringEditWithButton(QWidget* parent = nullptr);
    QString text() const;
    void setText(const QString& text);

signals:
    void textChanged(const QString& text);

private slots:
    void onButtonClicked();

private:
    QLineEdit* lineEdit;
    QPushButton* button;
};