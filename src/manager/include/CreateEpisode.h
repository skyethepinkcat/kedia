#pragma once
#include "file.hpp"
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
class CreateEpisode;
}
namespace kedia {
class CreateEpisode : public QDialog {
    Q_OBJECT

  public:
    explicit CreateEpisode(std::shared_ptr<kedia::Store> store,
                           int media_parent, QWidget* parent = nullptr);
    ~CreateEpisode();

    static QImage getThumbnail(QFileInfo video_file, QWidget* parent = nullptr);
  private slots:
    // When a file is added, this slot is triggered and it sets the file.
    void FileAddPressed();

    // When the text is changed, this slot is triggered and the text is checked
    // to make sure it is valid.
    void TextChanged();

    // When OK is pressed, this is triggered and it does final checks and add
    // the episode to kedia store.
    void accept();

    // When the list of series or seasons is changed, various things need to be
    // updated.
    void ListChanged(int index);

    // When the bonus checkbox is changed, the episode number spinbox becomes
    // decimal.
    void BonusChanged(int bonus);

    // When the episode number spinbox is, the episode title is updated with the
    // correct number.
    void SpinBoxChanged(double value);

    // Verifies that everything is set correctly and then enables or disables
    // the OK button.
    void verify();

    // Updates the progress bar. Currently unused.
    void updateBar(long long newVal);

  private:
    Ui::CreateEpisode* ui;
    QFileInfo file;
    std::shared_ptr<kedia::Store> store;
    void setupList(int media_parent = -1);
    QMenu* contextMenu = new QMenu(this);
    void SetImage(QImage img);
    QImage curr_image;
    bool manualtext = false;
    std::shared_ptr<kedia::Media> getMediaFromIndex(int item);
};
} // namespace kedia
