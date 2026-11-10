#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>

class OrderDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderDialog(QWidget *parent = nullptr);
    QString clientName() const;
    QString phone() const;
    QString clientNotes() const;
    QString orderTitle() const;
    QString orderDetails() const;
private:
    QLineEdit *nameEdit, *phoneEdit, *titleEdit;
    QTextEdit *clientNotesEdit, *detailsEdit;
};
