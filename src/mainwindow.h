#pragma once
#include <QMainWindow>
#include <QTableView>
#include <QSqlQueryModel>
#include <QTabWidget>
#include "database.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    Database database;
    QTabWidget *tabs;
    QTableView *activeTable, *completedTable;
    QSqlQueryModel *activeModel, *completedModel;
    void refresh();
    void addClientOrder();
    void addOrderToSelectedClient();
    void markCompleted();
    void changeIssueStatus();
    void deleteSelected();
    qlonglong selectedId(QTableView *view) const;
};
