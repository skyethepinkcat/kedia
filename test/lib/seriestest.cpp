#include "series.hpp"
#include <QObject>
#include <QString>
#include <QTest>
using namespace std;
using namespace kedia;

class SeriesTest : public QObject {
    Q_OBJECT

  private slots:
    void initialization_data();
    void initialization();
};

void SeriesTest::initialization_data() {
    QTest::addColumn<QString>("title");
    QTest::addColumn<int>("id");
    QTest::newRow("Basic Test") << "Cory in the House" << 1;
    QTest::newRow("Basic Test") << "Cory in the House" << 1;
}

void SeriesTest::initialization() {
    QFETCH(QString, title);
    QFETCH(int, id);

    Series testSeries(id, title, QImage());
    QCOMPARE(testSeries.getTitle(), title);
}

QTEST_MAIN(SeriesTest)
#include "seriestest.moc"
