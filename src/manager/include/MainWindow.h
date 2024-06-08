#pragma once
#include "media.hpp"
#include "store.hpp"
#include <QMainWindow>
#include <QScopedPointer>
#include <QStack>
#include <QTimer>
#include <qlistwidget.h>

#include <QActionGroup>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(std::shared_ptr<kedia::Store> store_in,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

    // openMedia sets the current parent to the given id,
    // going "into" that media. If going forward, the current
    // parent is put into the history.
    void openMedia(int parent = -1, bool forward = true);

  private slots:
    // Slot for when an item in the main list is "clicked."
    void activateItem(QListWidgetItem* item);

    // Slot for when an item in the main list is "right clicked."
    void showContextMenu(const QPoint& point);

    // Slot for when a series should be created.
    void showCreateSeriesMenu();

    // Slot for when a season should be created, with the given id as the
    // series.
    void showCreateSeasonMenu(int id);

    // Slot for when an episode should be created, with the given id as the
    // season.
    void showCreateEpisodeMenu(int id);

    // Go back in the navigation list.
    void back();

    // Go to the series list.
    void top();

    // Refresh the view with changes in entry status.
    void refresh();

    // Fully reload the view, adding any new entries.
    void full_refresh();

    // Set whether the application is busy
    void setBusy(bool status);

    // Opens a file dialog and parses the found path for files
    void parse_existing_dir();

    // Opens a dialog to create a new drive.
    void create_drive();

  private:
    // QActions are used with the respective slots to give multiple ways of
    // running them such as menu buttons and keyboard shortcuts.
    QAction nav_top;
    QAction nav_back;
    QAction create_series;
    QAction create_episode;
    QAction create_season;
    QAction reload;
    QAction parse_dir;
    QAction mwexit;
    QAction create_drive_action;
    QAction delete_media_action;

    // Sets up the QActions above.
    void setupActions();

    // Copies a given episode by its id to a drive, given its id.
    bool copyItem(int media_id, int drive_id);

    // Builds the main window toolbar.
    void build_toolbar();

    // Adds the given item to the list.
    void add_item(std::shared_ptr<kedia::Media> media_in);

    // Adds the given item to the list.
    void add_item(int id);

    QScopedPointer<Ui::MainWindow> m_ui;
    std::shared_ptr<kedia::Store> store;
    int current_parent;
    QTimer timer;
    QStack<int> history;
};
