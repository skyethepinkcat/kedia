#include "store.hpp"
#include "database.hpp"
#include <QObject>
#include <QString>
#include <QTest>
#include <iostream>
#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
using namespace std;
using namespace kedia;

class StoreTest : public QObject {
    Q_OBJECT

  private slots:
    void add_series_data();
    void add_series();
};

void StoreTest::add_series_data() {
    QTest::addColumn<QString>("title");
    QTest::addColumn<int>("id");
    QTest::newRow("Basic Series") << "Yoyo's Weird World" << 1;
}

void StoreTest::add_series() {
    QFETCH(QString, title);
    QFETCH(int, id);

    QFile file("/tmp/kedia.sqlite3");
    if (file.exists()) {
        file.remove();
    }
    try {
        Store store("/tmp/kedia.sqlite3");
        shared_ptr<Series> series = store.AddSeries(title);
        shared_ptr<Series> found_series =
            dynamic_pointer_cast<Series>(store.GetMedia(id));
        // QCOMPARE(series, found_series);
        QCOMPARE(series->getTitle(), title);
        QCOMPARE(series->getId(), id);

        QSqlQuery query;
        query.prepare(
            "SELECT media_id, series_title FROM series WHERE media_id = :id");
        query.bindValue(":id", id);
        QVERIFY(query.exec());
        QCOMPARE(query.value(0).toInt(), series->getId());
        QCOMPARE(query.value(1).toString(), series->getTitle());
    } catch (QSqlError e) {
        qDebug() << e.text();
    }
}

QTEST_MAIN(StoreTest)
#include "storetest.moc"
