#pragma once
#include "store.hpp"
#include <QDialog>
#include <QFileInfo>
#include <QPointer>
#include <QScopedPointer>
#include <qaction.h>
#include <qgraphicsitem.h>
#include <qlistwidget.h>
#include <qmenu.h>
#include <qregularexpression.h>
#include <qvalidator.h>
namespace Ui {
class CreateSeason;
}
namespace kedia {
class CreateSeason : public QDialog {
    Q_OBJECT

  public:
    explicit CreateSeason(std::shared_ptr<kedia::Store> store, int media_parent,
                          QWidget* parent = nullptr);
    ~CreateSeason();

  private slots:
    // When a file is added, this slot is triggered and it sets the file.
    void FileAddPressed();
    // When the text is changed, this slot is triggered and the text is checked
    // to make sure it is valid.
    void TextChanged();
    // When OK is pressed, this is triggered and it does final checks and add
    // the season to kedia store.
    void accept();
    // When the list of series is changed, various things need to be updated.
    void ListChanged(int index);
    // When the bonus checkbox is changed, the season number spinbox becomes
    // decimal.
    void BonusChanged(int bonus);
    // When the reset image button is triggered, the image preview should be
    // reset to the parent.
    void ResetImage();
    // When the season number spinbox is, the season title is updated with the
    // correct number.
    void SpinBoxChanged(double value);

    // Verifies that everything is set correctly and then enables or disables
    // the OK button.
    void verify();

  private:
    Ui::CreateSeason* ui;
    QFileInfo file;
    std::shared_ptr<kedia::Store> store;
    void setupList(int media_parent = -1);
    QMenu* contextMenu = new QMenu(this);
    void SetImage(QImage img);
    bool manualimg = false;
    bool manualtext = false;
    std::shared_ptr<kedia::Media> getMediaFromIndex(int item);
};

} // namespace kedia
