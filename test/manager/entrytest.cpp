#include "media.hpp"
#include "series.hpp"
#include "stageentry.hpp"
#include <QTest>

using namespace std;
using namespace kedia;

class EntryTest : public QObject {
    Q_OBJECT

  private slots:
    void initialEntry_data();
    void initialEntry();
};

void EntryTest::initialEntry_data() {
    QTest::addColumn<Series>("Media");
    QTest::addColumn<QString>("Result");

    Series testSeries1(1, "Cory in the House", QIcon());
}

void EntryTest::initialEntry() {
    QFETCH(Series, testSeries);
    QFETCH(QString, result);

    StageEntry rootEntry;
}

QTEST_MAIN(EntryTest)
#include "entrytest.moc"
