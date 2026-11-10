#include "database.h"
#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>

bool Database::open(QString *error) {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(dir)) {
        errorText = "Не удалось создать папку для базы данных.";
        if (error) *error = errorText;
        return false;
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dir + "/clients.sqlite");
    if (!db.open()) {
        errorText = db.lastError().text();
        if (error) *error = errorText;
        return false;
    }
    QSqlQuery q(db);
    const char *schema =
        "CREATE TABLE IF NOT EXISTS clients ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, phone TEXT, notes TEXT);"
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, client_id INTEGER NOT NULL,"
        "title TEXT NOT NULL, details TEXT, created_at TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
        "completed INTEGER NOT NULL DEFAULT 0, issue_status TEXT NOT NULL DEFAULT 'Ожидает выдачи',"
        "FOREIGN KEY(client_id) REFERENCES clients(id) ON DELETE CASCADE);";
    // Execute statements separately because QSQLITE does not reliably accept multi-statement exec.
    if (!q.exec("PRAGMA foreign_keys = ON") ||
        !q.exec("CREATE TABLE IF NOT EXISTS clients (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, phone TEXT, notes TEXT)") ||
        !q.exec("CREATE TABLE IF NOT EXISTS orders (id INTEGER PRIMARY KEY AUTOINCREMENT, client_id INTEGER NOT NULL, title TEXT NOT NULL, details TEXT, created_at TEXT NOT NULL DEFAULT (datetime('now','localtime')), completed INTEGER NOT NULL DEFAULT 0, issue_status TEXT NOT NULL DEFAULT 'Ожидает выдачи', FOREIGN KEY(client_id) REFERENCES clients(id) ON DELETE CASCADE)")) {
        Q_UNUSED(schema);
        errorText = q.lastError().text();
        if (error) *error = errorText;
        return false;
    }
    return true;
}
