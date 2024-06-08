#include "MainWindow.h"
#include "store.hpp"
#include <QApplication>
#include <QFile>

using namespace std;
using namespace kedia;
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    std::shared_ptr<Store> store(new Store());
    MainWindow w(store);
    w.show();

    w.openMedia(-1);
    return app.exec();
}
