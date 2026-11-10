#include "mainwindow.h"
#include "orderdialog.h"
#include <QStatusBar>
#include <QMessageBox>
#include <QToolBar>
#include <QAction>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QInputDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Клиенты и заказы");
    resize(1050, 650);
    QString error;
    if (!database.open(&error)) {
        QMessageBox::critical(this, "Ошибка базы данных", error);
    }

    auto *bar = addToolBar("Действия");
    bar->setMovable(false);
    auto add = bar->addAction("＋ Новый клиент и заказ");
    auto extra = bar->addAction("＋ Заказ выбранному клиенту");
    auto done = bar->addAction("✓ Заказ выполнен");
    auto status = bar->addAction("Изменить статус выдачи");
    auto remove = bar->addAction("Удалить запись");
    connect(add, &QAction::triggered, this, &MainWindow::addClientOrder);
    connect(extra, &QAction::triggered, this, &MainWindow::addOrderToSelectedClient);
    connect(done, &QAction::triggered, this, &MainWindow::markCompleted);
    connect(status, &QAction::triggered, this, &MainWindow::changeIssueStatus);
    connect(remove, &QAction::triggered, this, &MainWindow::deleteSelected);

    tabs = new QTabWidget(this);
    activeTable = new QTableView;
    completedTable = new QTableView;
    activeModel = new QSqlQueryModel(this);
    completedModel = new QSqlQueryModel(this);
    activeTable->setModel(activeModel);
    completedTable->setModel(completedModel);
    for (auto *table : {activeTable, completedTable}) {
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setStretchLastSection(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->verticalHeader()->hide();
    }
    auto *activePage = new QWidget;
    auto *activeLayout = new QVBoxLayout(activePage);
    activeLayout->addWidget(new QLabel("Текущие клиенты и заказы. Выберите строку, чтобы добавить заказ клиенту или отметить выполнение."));
    activeLayout->addWidget(activeTable);
    auto *completedPage = new QWidget;
    auto *completedLayout = new QVBoxLayout(completedPage);
    completedLayout->addWidget(new QLabel("Выполненные заказы. Для каждого заказа можно установить статус «Ожидает выдачи» или «Выдан»."));
    completedLayout->addWidget(completedTable);
    tabs->addTab(activePage, "Клиенты и заказы");
    tabs->addTab(completedPage, "Выполненные заказы");
    setCentralWidget(tabs);
    statusBar()->showMessage("Данные автоматически сохраняются в локальную базу SQLite.");
    refresh();
}

void MainWindow::refresh() {
    activeModel->setQuery("SELECT o.id AS '№', c.name AS 'Клиент', c.phone AS 'Телефон', o.title AS 'Заказ', o.details AS 'Описание', o.created_at AS 'Дата' FROM orders o JOIN clients c ON c.id=o.client_id WHERE o.completed=0 ORDER BY o.id DESC", database.connection());
    completedModel->setQuery("SELECT o.id AS '№', c.name AS 'Клиент', c.phone AS 'Телефон', o.title AS 'Заказ', o.details AS 'Описание', o.created_at AS 'Дата', o.issue_status AS 'Статус выдачи' FROM orders o JOIN clients c ON c.id=o.client_id WHERE o.completed=1 ORDER BY o.id DESC", database.connection());
    activeTable->resizeColumnsToContents();
    completedTable->resizeColumnsToContents();
}
qlonglong MainWindow::selectedId(QTableView *view) const {
    auto index = view->currentIndex();
    if (!index.isValid()) return -1;
    return view->model()->data(view->model()->index(index.row(), 0)).toLongLong();
}
void MainWindow::addClientOrder() {
    OrderDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;
    QSqlQuery q(database.connection());
    q.prepare("INSERT INTO clients(name,phone,notes) VALUES(?,?,?)");
    q.addBindValue(dialog.clientName()); q.addBindValue(dialog.phone()); q.addBindValue(dialog.clientNotes());
    if (!q.exec()) { QMessageBox::critical(this,"Ошибка",q.lastError().text()); return; }
    const auto clientId = q.lastInsertId().toLongLong();
    q.prepare("INSERT INTO orders(client_id,title,details) VALUES(?,?,?)");
    q.addBindValue(clientId); q.addBindValue(dialog.orderTitle()); q.addBindValue(dialog.orderDetails());
    if (!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
    refresh();
}
void MainWindow::addOrderToSelectedClient() {
    QTableView *view = tabs->currentIndex()==0 ? activeTable : completedTable;
    const auto id = selectedId(view);
    if (id < 0) { QMessageBox::information(this,"Выберите запись","Сначала выберите заказ клиента."); return; }
    QSqlQuery find(database.connection());
    find.prepare("SELECT c.id,c.name FROM orders o JOIN clients c ON c.id=o.client_id WHERE o.id=?");
    find.addBindValue(id);
    if (!find.exec() || !find.next()) return;
    const auto clientId=find.value(0).toLongLong();
    const auto clientName=find.value(1).toString();
    bool ok=false;
    const QString title=QInputDialog::getText(this,"Новый заказ","Новый заказ для клиента "+clientName+":",QLineEdit::Normal,"",&ok);
    if (!ok || title.trimmed().isEmpty()) return;
    const QString details=QInputDialog::getMultiLineText(this,"Описание заказа","Описание:", "", &ok);
    if (!ok) return;
    QSqlQuery q(database.connection()); q.prepare("INSERT INTO orders(client_id,title,details) VALUES(?,?,?)");
    q.addBindValue(clientId); q.addBindValue(title.trimmed()); q.addBindValue(details);
    if (!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
    refresh();
}
void MainWindow::markCompleted() {
    if (tabs->currentIndex()!=0) { QMessageBox::information(this,"Подсказка","Выберите заказ на вкладке «Клиенты и заказы»."); return; }
    const auto id=selectedId(activeTable);
    if (id<0) { QMessageBox::information(this,"Выберите заказ","Сначала выберите заказ."); return; }
    QSqlQuery q(database.connection()); q.prepare("UPDATE orders SET completed=1, issue_status='Ожидает выдачи' WHERE id=?"); q.addBindValue(id);
    if (!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
    refresh();
}
void MainWindow::changeIssueStatus() {
    if (tabs->currentIndex()!=1) { QMessageBox::information(this,"Подсказка","Перейдите на вкладку «Выполненные заказы»."); return; }
    const auto id=selectedId(completedTable);
    if (id<0) { QMessageBox::information(this,"Выберите заказ","Сначала выберите выполненный заказ."); return; }
    QStringList options{"Ожидает выдачи","Выдан"};
    bool ok=false;
    const QString choice=QInputDialog::getItem(this,"Статус выдачи","Выберите статус:",options,0,false,&ok);
    if (!ok) return;
    QSqlQuery q(database.connection()); q.prepare("UPDATE orders SET issue_status=? WHERE id=?"); q.addBindValue(choice); q.addBindValue(id);
    if (!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
    refresh();
}
void MainWindow::deleteSelected() {
    QTableView *view=tabs->currentIndex()==0 ? activeTable : completedTable;
    const auto id=selectedId(view);
    if (id<0) { QMessageBox::information(this,"Выберите запись","Сначала выберите заказ."); return; }
    if (QMessageBox::question(this,"Подтверждение","Удалить выбранный заказ?")!=QMessageBox::Yes) return;
    QSqlQuery q(database.connection()); q.prepare("DELETE FROM orders WHERE id=?"); q.addBindValue(id);
    if (!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
    refresh();
}
