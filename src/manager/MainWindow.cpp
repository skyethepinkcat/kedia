#include "MainWindow.h"
#include "CreateDrive.h"
#include "CreateEpisode.h"
#include "CreateSeason.h"
#include "CreateSeries.h"
#include "media.hpp"
#include "ui_MainWindow.h"
#include <QAccessible>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFuture>
#include <QIcon>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>
#include <QtCore>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <qpoint.h>
using namespace std;
using namespace kedia;

MainWindow::MainWindow(std::shared_ptr<kedia::Store> store_in, QWidget* parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    m_ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    store = store_in;
    setupActions();

    // Set timer for auto-refresh to 1000ms.
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::refresh);

    timer.start();
    m_ui->menuFile->addAction(&create_series);
    m_ui->menuFile->addAction(&parse_dir);
    connect(m_ui->listWidget, &QListWidget::itemActivated, this,
            &MainWindow::activateItem);
    connect(m_ui->actionCreateSeries, &QAction::triggered, this,
            &MainWindow::showCreateSeriesMenu);
    connect(m_ui->listWidget, &QListWidget::customContextMenuRequested, this,
            &MainWindow::showContextMenu);
    build_toolbar();
    current_parent = -1;
    nav_top.setIcon(QIcon::fromTheme("go-home"));
}

void MainWindow::setupActions() {
    nav_top.setIcon(QIcon::fromTheme("go-top"));
    nav_top.setText(tr("Top"));
    nav_top.setToolTip(tr("Return to the list of series."));
    connect(&nav_top, &QAction::triggered, this, &MainWindow::top);

    nav_back.setIcon(QIcon::fromTheme("go-previous"));
    nav_back.setText(tr("Back"));
    connect(&nav_back, &QAction::triggered, this, &MainWindow::back);
    create_series.setShortcut(QKeySequence::Back);
    nav_back.setToolTip(tr("Go back to the last page."));

    create_series.setIcon(QIcon::fromTheme("document-new"));
    create_series.setText(tr("Create Series"));
    create_series.setShortcut(QKeySequence::New);
    connect(&create_series, &QAction::triggered, this,
            &MainWindow::showCreateSeriesMenu);
    create_series.setToolTip(tr("Create a new series."));

    create_season.setIcon(QIcon::fromTheme("list-add"));
    create_season.setText(tr("Create Season"));

    reload.setIcon(QIcon::fromTheme("view-refresh"));
    reload.setText(tr("Refresh"));
    reload.setToolTip(tr("Refresh the page."));
    reload.setShortcut(QKeySequence::Refresh);
    connect(&reload, &QAction::triggered, this, &MainWindow::full_refresh);

    parse_dir.setText(tr("Scan a Directory"));
    parse_dir.setIcon(QIcon::fromTheme("folder-open"));
    connect(&parse_dir, &QAction::triggered, this,
            &MainWindow::parse_existing_dir);
    mwexit.setText(tr("Exit"));
    mwexit.setIcon(QIcon::fromTheme("window-close"));
    mwexit.setShortcut(QKeySequence::Close);
    connect(&mwexit, &QAction::triggered, this, [this]() { close(); });

    create_drive_action.setText("New Drive");
    create_drive_action.setIcon(QIcon::fromTheme("drive-harddisk"));
    create_drive_action.setToolTip("Add a new drive to kedia.");
    connect(&create_drive_action, &QAction::triggered, this,
            &MainWindow::create_drive);

    delete_media_action.setText("Delete");
    delete_media_action.setIcon(QIcon::fromTheme("edit-delete"));
}

void MainWindow::create_drive() {
    QString user_path =
        QFileDialog::getExistingDirectory(this, "Choose Drive Directory", "/");
    if (store->identifyDrive(QStorageInfo(user_path)) != -1) {
        QMessageBox::information(
            this, "Drive Exists",
            "This drive already exists, no need to create one.");
    }
    CreateDrive cd(store, QStorageInfo(user_path).rootPath(), this);
    cd.exec();
}

bool MainWindow::copyItem(int media_id, int drive_id) {
    shared_ptr<Episode> episode =
        dynamic_pointer_cast<Episode>(store->GetMedia(media_id));
    shared_ptr<Drive> drive = store->getDrive(drive_id);
    QFileInfo episode_file_info = store->getFileLocationFromEpisode(media_id);
    int source_drive_id =
        store->identifyDrive(QStorageInfo(episode_file_info.filePath()));
    shared_ptr<Drive> source_drive = store->getDrive(source_drive_id);
    QString rel_episode_directory_path =
        source_drive->getVideoDirectory().relativeFilePath(
            episode_file_info.path());

    QDir video_path = drive->getVideoDirectory();

    QString newpath =
        (video_path.path() + QDir::separator() + rel_episode_directory_path);

    if (!video_path.mkpath(video_path.path() + QDir::separator() +
                           rel_episode_directory_path)) {
        return false;
    }

    QFile episode_file = QFile(episode_file_info.filePath());
    this->setCursor(Qt::BusyCursor);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    setBusy(true);
    QThreadPool::globalInstance()->start([this, episode_file_info, newpath] {
        QFile(episode_file_info.filePath())
            .copy(newpath + QDir::separator() + episode_file_info.fileName());
        this->setBusy(false);
    });
    QApplication::restoreOverrideCursor();
    this->unsetCursor();
    QFileInfo newfile_file_info(newpath + QDir::separator() +
                                episode_file_info.fileName());
    store->addFileLocation(store->getFileIdFromEpisode(media_id), drive_id,
                           newfile_file_info.filePath());

    return true;
}

void MainWindow::setBusy(bool status) {
    if (!status) {
        this->setEnabled(true);
        this->unsetCursor();
        QApplication::restoreOverrideCursor();
    } else {
        this->setEnabled(false);
        this->setCursor(Qt::BusyCursor);
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }
}
void MainWindow::refresh() {
    for (int item_id = 0; item_id < m_ui->listWidget->count(); item_id++) {
        auto item = m_ui->listWidget->item(item_id);
        int id = item->data(Qt::UserRole).toInt();
        auto media = store->GetMedia(id);
        if (media->getType() == EPISODE) {
            if (!store->episodeIsAccessible(id)) {
                item->setFlags(item->flags() &
                               ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
                item->setToolTip(
                    "Episode is not accessible, drive may be removed.");
            } else {
                item->setFlags(item->flags() |
                               (Qt::ItemIsEnabled | Qt::ItemIsSelectable));
                item->setToolTip("");
            }
        } else {
            if (!store->childrenAccessible(id)) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                item->setToolTip("Items not accessible, drive may be removed.");
            } else {
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
                item->setToolTip("");
            }
        }
    }
}

void MainWindow::build_toolbar() {
    auto tool = addToolBar(tr("Toolbar"));
    tool->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    tool->setMovable(false);
    tool->addAction(&nav_top);
    tool->addAction(&nav_back);
    tool->addAction(&create_series);
    tool->addAction(&reload);
    tool->addAction(&create_drive_action);

    m_ui->menuFile->addAction(&reload);
    m_ui->menuFile->addAction(&mwexit);
}

void MainWindow::showCreateSeriesMenu() {
    CreateSeries dialog(store, this);
    auto result = dialog.exec();
    if (result == QDialog::Accepted) {
        openMedia(current_parent);
    }
}

void MainWindow::back() {
    if (history.empty()) {
        return;
    }
    openMedia(history.pop(), false);
}

void MainWindow::top() { openMedia(-1); }

void MainWindow::add_item(shared_ptr<Media> media_in) {
    add_item(media_in->getId());
}
void MainWindow::add_item(int id) {
    auto media = store->GetMedia(id);
    auto item = new QListWidgetItem(media->getTitle(), m_ui->listWidget);
    item->setData(Qt::DecorationRole, media->getPreview());
    item->setData(Qt::UserRole, media->getId());
    if (media->getType() == EPISODE && !store->episodeIsAccessible(id)) {
        item->setFlags(item->flags() ^
                       (Qt::ItemIsEnabled | Qt::ItemIsSelectable));
        item->setToolTip("Episode is not accessible, drive may be removed.");
    }
    if (media->getType() == EPISODE) {
        item->setData(1001, dynamic_pointer_cast<Episode>(media)->getNumber());
    } else if (media->getType() == SEASON) {
        item->setData(1001, dynamic_pointer_cast<Season>(media)->getNumber());
    }
    if (!store->childrenAccessible(id)) {
        item->setFlags(item->flags() ^ Qt::ItemIsEnabled);
    }
}

void MainWindow::activateItem(QListWidgetItem* item) {
    auto m = store->GetMedia(item->data(Qt::UserRole).toInt());
    openMedia(m->getId());
}

void MainWindow::openMedia(int parent, bool forward) {
    if (parent != -1) {
        auto m = store->GetMedia(parent);
        if (m->getChildType() == MediaType::NONE || m->getType() == EPISODE) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(
                store->getFileLocationFromEpisode(parent).filePath()));
            return;
        }
    }
    m_ui->listWidget->clear();
    if (forward) {
        history.push(current_parent);
    }
    current_parent = parent;
    vector<uint> ids;
    if (parent == -1) {
        ids = store->GetIds(kedia::SERIES);
    } else {
        ids = store->GetIds(kedia::ANY);
    }
    for (uint i : ids) {
        if (store->IsChild(parent, i)) {
            add_item(i);
        }
    }
    refresh();
}

void MainWindow::showContextMenu(const QPoint& point) {
    QListWidgetItem* item = m_ui->listWidget->itemAt(point);
    if (item == nullptr) {
        return;
    }
    QPoint globalPos = m_ui->listWidget->mapToGlobal(point);
    int id = item->data(Qt::UserRole).toInt();
    shared_ptr<Media> m = store->GetMedia(id);

    QMenu context;
    QAction open;
    if (m->getChildType() != NONE) {
        open.setIcon(QIcon::fromTheme("folder-open"));
        open.setText("Open");
    } else {
        // Set up specific episode contexts
        open.setIcon(QIcon::fromTheme("media-playback-start"));
        open.setText(tr("Play"));
        QMenu* copyMenu = context.addMenu(tr("Copy To..."));
        auto drives = store->getDriveIds();
        for (auto drive_id : drives) {
            auto drive = store->getDrive(drive_id);
            auto act = copyMenu->addAction(drive->getName());

            // Set default icons for drives based on type
            if (drive->getRemoveable()) {
                act->setIcon(QIcon::fromTheme("drive-removable-media-usb"));
            } else {
                act->setIcon(QIcon::fromTheme("drive-harddisk"));
            }

            // If the drive is connected
            if (drive->connected()) {
                if (store->episodeOnDrive(id, drive_id)) {
                    // Set the icon to be a checkmark if the episode is
                    // connected and on the drive , and make the action
                    // disabled.
                    act->setIcon(QIcon::fromTheme("answer-correct"));
                    act->setEnabled(false);
                } else {
                    // Set the action to be enabled and connect it to a copy
                    // item call.
                    connect(act, &QAction::triggered, this,
                            [this, drive_id, id]() {
                                this->copyItem(id, drive_id);
                            });
                }
                // If the drive is not connected
            } else {
                // Always set the action to disabled
                act->setEnabled(false);
                if (store->episodeOnDrive(id, drive_id) &&
                    drive->getRemoveable()) {
                    // If the file is supposed to be on the drive and the drive
                    // is supposed to be removeable, set the icon to a
                    // checkmark.
                    act->setIcon(QIcon::fromTheme("answer-correct"));
                }
                if (drive->getRemoveable()) {
                    // Set the text to prepend "ejected" for a removable drive
                    act->setText("(Ejected) " + drive->getName());
                } else {
                    // Set the text to prepend "missing" for a non-removeable
                    // drive, and add an error icon.
                    act->setText("(Missing) " + drive->getName());
                    act->setIcon(QIcon::fromTheme("error"));
                }
            }
        }
    }

    // Open will be shown as "play" for an episode.
    context.addAction(&open);
    connect(&open, &QAction::triggered, this, [this, id]() { openMedia(id); });

    // Make a menu option to create a child media.
    QAction newChild("New " + MediaTypeToString(m->getChildType()));
    newChild.setIcon(QIcon::fromTheme(""));

    // If the media doesn't have a child (such as an episode), don't bother.
    if (m->getChildType() != MediaType::NONE &&
        m->getChildType() != MediaType::ANY) {
        context.addAction(&newChild);
    }
    if (m->getChildType() == SEASON) {
        connect(&newChild, &QAction::triggered, this,
                [this, id]() { showCreateSeasonMenu(id); });
    } else if (m->getChildType() == EPISODE) {
        connect(&newChild, &QAction::triggered, this,
                [this, id]() { showCreateEpisodeMenu(id); });
    }

    // Add an action to delete the media.
    context.addAction(&delete_media_action);
    connect(&delete_media_action, &QAction::triggered, this, [this, id]() {
        store->deleteMedia(id);
        full_refresh();
    });

    // Finally execute the text menu
    context.exec(globalPos);
}

void MainWindow::showCreateSeasonMenu(int id) {
    CreateSeason seasonMenu(store, id, this);
    seasonMenu.exec();
}

void MainWindow::showCreateEpisodeMenu(int id) {
    CreateEpisode episodeMenu(store, id, this);
    episodeMenu.exec();
}

MainWindow::~MainWindow() = default;

pair<double, QString> parseSeasonName(QString dirName) {
    static QRegularExpression noTitle("Season ([\\d.]+)");
    static QRegularExpression title("Season ([\\d.]+) - (.*)");

    auto tRes = title.match(dirName);
    if (tRes.hasMatch()) {
        qDebug() << "Number: " << tRes.captured(1)
                 << ", Title: " << tRes.captured(2);
        double num = tRes.captured(1).toDouble();
        QString episodeT = tRes.captured(2);
        return pair<double, QString>(
            tRes.captured(1).toDouble(),
            "Season " +
                QStringLiteral("%1").arg(num, 2, 'g', 2, QLatin1Char('0')) +
                ": " + episodeT);
    } else {
        auto ntRes = noTitle.match(dirName);
        return pair<double, QString>(
            ntRes.captured(1).toDouble(),
            "Season " + QString::number(ntRes.captured(1).toDouble()));
    }
}

pair<double, QString> parseEpisodeName(QString dirName) {
    static QRegularExpression noTitle("E([\\d\\.]+)\\.");
    static QRegularExpression title("E([\\d.]+) - (.*)\\.(mkv|mp4)");

    auto tRes = title.match(dirName);
    if (tRes.hasMatch()) {
        qDebug() << "Double: " << tRes.captured(1)
                 << ", Title: " << tRes.captured(2);
        double num = tRes.captured(1).toDouble();
        QString episodeT = tRes.captured(2);
        return pair<double, QString>(
            num,
            "Episode " +
                QStringLiteral("%1").arg(num, 2, 'g', 2, QLatin1Char('0')) +
                ": " + episodeT);
    } else {
        auto ntRes = noTitle.match(dirName);
        return pair<double, QString>(ntRes.captured(1).toDouble(),
                                     "Episode " + ntRes.captured(1));
    }
}

void ImportFiles(std::shared_ptr<kedia::Store> store, QDir& baseDir,
                 MainWindow* parent) {
    QStringList series = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto& s : series) {
        qDebug() << s;
        auto se = store->AddSeries(
            s, QImage(baseDir.path() + "/" + s + "/thumbnail.png"));

        QStringList seasons = QDir(baseDir.path() + "/" + s)
                                  .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (auto& season : seasons) {
            auto seasondetail = parseSeasonName(season);
            bool bonus = (trunc(seasondetail.first) != seasondetail.first);

            auto seasono =
                store->AddSeason(se->getId(), seasondetail.second,
                                 seasondetail.first, bonus, se->getImage());
            QString seasonpath = baseDir.path() + "/" + s + "/" + season + "/";
            qDebug() << seasonpath;
            QDir seasondir(seasonpath);
            for (auto& episode : seasondir.entryList(QDir::Files)) {
                if (episode.endsWith(".mkv") or episode.endsWith(".mp4")) {
                    QString episodepath = seasonpath + episode;
                    auto episodedetail = parseEpisodeName(episode);
                    // qDebug() << "Checksum for " << episodepath;
                    auto f = store->AddFile(QFileInfo(episodepath), "");
                    auto e = store->AddEpisode(
                        seasono->getId(), episodedetail.second,
                        episodedetail.first,
                        CreateEpisode::getThumbnail(QFileInfo(episodepath),
                                                    parent));
                    store->linkFileMedia(f->getId(), e->getId());
                }
            }
        }
    }
}

void parse(QString dir_path, std::shared_ptr<kedia::Store> store,
           MainWindow* parent) {

    QDir dir(dir_path);

    ImportFiles(store, dir, parent);
}

void MainWindow::parse_existing_dir() {
    QString dir = QFileDialog::getExistingDirectory(this, "Anime Directory",
                                                    QDir::home().path());

    if (dir != "") {
        parse(dir, store, this);
    }
    full_refresh();
}

void MainWindow::full_refresh() { openMedia(current_parent); }

#include "moc_MainWindow.cpp"
