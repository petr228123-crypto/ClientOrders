#include "orderdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

OrderDialog::OrderDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Новый клиент и заказ");
    setMinimumWidth(430);
    nameEdit = new QLineEdit;
    phoneEdit = new QLineEdit;
    titleEdit = new QLineEdit;
    clientNotesEdit = new QTextEdit; clientNotesEdit->setMaximumHeight(65);
    detailsEdit = new QTextEdit; detailsEdit->setMaximumHeight(90);
    auto *form = new QFormLayout;
    form->addRow("Имя клиента *", nameEdit);
    form->addRow("Телефон", phoneEdit);
    form->addRow("Заметки о клиенте", clientNotesEdit);
    form->addRow("Заказ *", titleEdit);
    form->addRow("Описание заказа", detailsEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        if (nameEdit->text().trimmed().isEmpty() || titleEdit->text().trimmed().isEmpty()) {
            nameEdit->setPlaceholderText("Обязательно заполните имя");
            titleEdit->setPlaceholderText("Обязательно укажите заказ");
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Добавьте клиента и его первый заказ:"));
    layout->addLayout(form);
    layout->addWidget(buttons);
}
QString OrderDialog::clientName() const { return nameEdit->text().trimmed(); }
QString OrderDialog::phone() const { return phoneEdit->text().trimmed(); }
QString OrderDialog::clientNotes() const { return clientNotesEdit->toPlainText().trimmed(); }
QString OrderDialog::orderTitle() const { return titleEdit->text().trimmed(); }
QString OrderDialog::orderDetails() const { return detailsEdit->toPlainText().trimmed(); }
