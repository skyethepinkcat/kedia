#pragma once
#include <QSqlDatabase>

namespace kedia {

class Database {
    friend class Store;

  public:
    // Initalizes an sqlite3 database at the given file name.
    Database(QString database_name = "kedia.sqlite3");

#ifdef QT_DEBUG
    // Initalizes a basic database for early testing.
    void TEST_QUERY();
#endif

    QSqlDatabase database;

  private:
    // Initializes the database, if no tables exists.
    bool initDatabase();
    QString databaseName;
};

}; // namespace kedia
