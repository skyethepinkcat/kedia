#include "manager_utils.hpp"

using namespace kedia;
void kedia::setWidgetsEnabled(QList<QWidget*> widgets, bool set) {
    for (auto w : widgets) {
        w->setEnabled(set);
    }
}
