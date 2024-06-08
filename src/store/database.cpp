#include "database.hpp"
#include <QFile>
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <qsqlquery.h>
#include <stdexcept>

using namespace std;
using namespace kedia;

Database::Database(QString databaseName_in) {
    databaseName = databaseName_in;
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databaseName);
    if (!database.open()) {
        throw std::runtime_error("Couldn't open database.");
    }
    initDatabase();
}

bool Database::initDatabase() {

    if (database.tables().size() != 0) {
        QSqlQuery query;
        query.exec("PRAGMA foreign_keys = ON");
        return true;
    }

    // Pulling the init script for the qt resource.
    QFile init_script(":/init.sql");
    if (!init_script.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Couldn't get resource...");
    }

    QTextStream script_stream(&init_script);
    // Read the entire file, remove all extra whitespace, and split on ;.
    QStringList sql_queries =
        script_stream.readAll().simplified().split(u';', Qt::SkipEmptyParts);

    QSqlQuery query;
    for (QString& q : sql_queries) {
        if (!query.exec(q)) {
            qDebug() << query.lastError().text();
            throw query.lastError();
        }
    }

    return true;
}

#ifdef QT_DEBUG
void Database::TEST_QUERY() {

    QSqlQuery query(database);
    QStringList queries = {
        "INSERT INTO media (media_id) VALUES (1), (2), (3), (4), (5), (6), "
        "(7), "
        "(8), (9), (10)",
        "INSERT INTO series (media_id, series_title) VALUES (1, \"Yoyo\'s "
        "Weird "
        "Journey\"), (4, \"Two Piece\")",
        "INSERT INTO season (media_id, series_id, season_num, is_bonus, "
        "season_title) VALUES"
        "(2, 1, 1, False, \"Ruby is not Crash\"), (5, 4, 1, False, \"Green "
        "Ocean\")",
        "INSERT INTO episode (media_id, season_id, episode_title, episode_num) "
        "VALUES"
        "(3, 2, \"Yoyo finds a baby\", 1.0), (6, 5, \"Zororororo slashes "
        "man\", 2.0), "
        "(7, 2, \"The Enemy Sit is Among Us!\", 2.0)"};

    for (auto& qs : queries) {
        if (!query.exec(qs)) {
            qDebug() << qs;
            qDebug() << query.lastError().text();
        }
    }
}
#endif
