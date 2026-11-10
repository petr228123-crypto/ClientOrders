#pragma once
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QString>

class Database {
public:
    bool open(QString *error = nullptr);
    QSqlDatabase connection() const { return db; }
    QString lastError() const { return errorText; }
private:
    QSqlDatabase db;
    QString errorText;
};
