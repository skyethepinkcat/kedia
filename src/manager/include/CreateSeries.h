#pragma once
#include "store.hpp"
#include <QDialog>
#include <QFileInfo>
#include <QScopedPointer>
#include <qlistwidget.h>
#include <qregularexpression.h>
#include <qvalidator.h>

namespace Ui {
class CreateSeries;
}
namespace kedia {
// Creates a global regex valiadtor instance that accepts any non-empty string.
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpressionValidator, NOT_EMPTY_VALIDATOR,
                          (QRegularExpression("^.+$")));

class CreateSeries : public QDialog {
    Q_OBJECT

  public:
    explicit CreateSeries(std::shared_ptr<kedia::Store> store,
                          QWidget* parent = nullptr);
    ~CreateSeries();

  private slots:
    // When a file is added, this slot is triggered and it sets the file.
    void FileAddPressed();
    // When the text is changed, this slot is triggered and the text is checked
    // to make sure it is valid.
    void TextChanged();
    // When OK is pressed, this is triggered and it does final checks and add
    // the series to kedia store.
    void accept();

  private:
    Ui::CreateSeries* ui;
    QFileInfo file;
    std::shared_ptr<kedia::Store> store;
};

} // namespace kedia
