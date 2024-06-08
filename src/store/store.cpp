#include "store.hpp"
#include "episode.hpp"
#include "media.hpp"
#include <QBuffer>
#include <QSqlError>
#include <QSqlQuery>
#include <QStorageInfo>
#include <qsqlquery.h>

using namespace kedia;
using namespace std;

Store::Store(QString db_name) {
    db = unique_ptr<Database>(new Database(db_name));
    ReadDatabase();
}

shared_ptr<Media> Store::GetMedia(int id) {
    if (media.find(id) == media.end()) {
        throw std::runtime_error("Media not found");
    }
    return media[id];
}

shared_ptr<Media> Store::readMedia(unsigned int media_id) {

    shared_ptr<Media> out;
    auto found = media.find(media_id);

    if (found != media.end()) {
        return found->second;
    }

    out = readSeries(media_id);
    if (out != nullptr) {
        return out;
    }

    out = readSeason(media_id);
    if (out != nullptr) {
        return out;
    }

    out = readEpisode(media_id);
    if (out != nullptr) {
        return out;
    }

    return nullptr;
}

QImage Store::readIcon(unsigned int id) {
    QSqlQuery query;
    query.prepare("SELECT icon FROM media WHERE media_id = :id");
    query.bindValue(":id", id);
    query.exec();
    if (query.first()) {
        QByteArray bytes = query.value(0).toByteArray();
        if (bytes.isEmpty()) {
            return QImage();
        } else {
            return QImage::fromData(bytes, "PNG");
        }
    }
    return QImage();
}

shared_ptr<Series> Store::readSeries(unsigned int media_id) {
    QSqlQuery query;

    query.prepare("SELECT series_title FROM series WHERE media_id = :id");
    query.bindValue(":id", media_id);

    if (!query.exec()) {
        throw query.lastError();
    }

    if (!query.first()) {
        return nullptr;
    }

    shared_ptr<Series> new_series(
        new Series(media_id, query.value(0).toString(), readIcon(media_id)));

    query.prepare("SELECT media_id FROM season WHERE series_id = :id");
    query.bindValue(":id", media_id);
    query.exec();

    if (query.first()) {
        do {
            shared_ptr<Season> season = readSeason(query.value(0).toUInt());
            new_series->appendChild(season);
            season->appendParent(new_series);
        } while (query.next());
    }

    media[media_id] = new_series;
    return new_series;
}

shared_ptr<Season> Store::readSeason(unsigned int media_id) {
    QSqlQuery query;

    query.prepare("SELECT season_title, season_num, is_bonus FROM season WHERE "
                  "media_id = :id");
    query.bindValue(":id", media_id);

    if (!query.exec()) {
        throw query.lastError();
    }

    if (!query.first()) {
        return nullptr;
    }

    shared_ptr<Season> new_season(
        new Season(media_id, query.value(0).toString(), query.value(1).toInt(),
                   query.value(2).toBool(), readIcon(media_id)));

    query.prepare("SELECT media_id FROM episode WHERE season_id = :id");
    query.bindValue(":id", media_id);

    if (!query.exec()) {
        throw query.lastError();
    }

    if (query.first()) {
        do {
            shared_ptr<Episode> episode = readEpisode(query.value(0).toUInt());
            episode->appendParent(new_season);
            new_season->appendChild(episode);
        } while (query.next());
    }

    media[media_id] = new_season;
    return new_season;
}

shared_ptr<Episode> Store::readEpisode(unsigned int media_id) {
    QSqlQuery query;

    query.prepare(
        "SELECT episode_title, episode_num FROM episode WHERE media_id = :id");
    query.bindValue(":id", media_id);

    if (!query.exec()) {
        throw query.lastError();
    }

    if (!query.first()) {
        return nullptr;
    }
    query.first();
    shared_ptr<Episode> new_episode(
        new Episode(media_id, query.value(0).toString(),
                    query.value(1).toDouble(), readIcon(media_id)));
    media[media_id] = new_episode;
    return new_episode;
}

void Store::ReadDatabase() {
    QSqlQuery query;
    query.exec("SELECT media_id FROM media");
    if (!query.first()) {
        return;
    }
    do {
        readMedia(query.value(0).toUInt());
    } while (query.next());

    query.exec("SELECT drive_id FROM drive");
    if (!query.first()) {
        return;
    }
    do {
        readDrive(query.value(0).toUInt());
    } while (query.next());

    query.exec("SELECT file_id FROM file");
    if (!query.first()) {
        return;
    }
    do {
        readFile(query.value(0).toUInt());
    } while (query.next());
}

int Store::newMediaId() {
    // This *should* take the next not in use id, but I haven't gotten around to
    // implementing that.
    QSqlQuery query;
    query.exec("INSERT INTO media (in_use) VALUES (False)");
    return query.lastInsertId().toInt();
}

shared_ptr<Series> Store::AddSeries(QString title, QImage icon) {
    QSqlQuery query;
    int id = newMediaId();
    shared_ptr<Series> new_series(new Series(id, title, icon));

    query.prepare(
        "INSERT INTO series (media_id, series_title) VALUES (:id,:title)");
    query.bindValue(":id", id);
    query.bindValue(":title", title);
    if (!query.exec()) {
        throw query.lastError();
    }

    query.prepare("UPDATE media SET in_use = True WHERE media_id = :id");
    query.bindValue(":id", id);
    query.exec();
    media[id] = new_series;
    SetIcon(id, icon);

    return new_series;
}

shared_ptr<Season> Store::AddSeason(int series_id, QString title,
                                    double season_num, bool is_bonus,

                                    QImage icon) {
    return AddSeason(dynamic_pointer_cast<Series>(GetMedia(series_id)), title,
                     season_num, is_bonus, icon);
}
shared_ptr<Season> Store::AddSeason(std::weak_ptr<Series> weak_series,
                                    QString title, double season_num,
                                    bool is_bonus, QImage icon) {

    std::shared_ptr<Series> series = weak_series.lock();
    QSqlQuery query;
    int id = newMediaId();
    shared_ptr<Season> new_season(
        new Season(id, title, season_num, is_bonus, icon));

    query.prepare(
        "INSERT INTO season (media_id, series_id, season_title, season_num, "
        "is_bonus) VALUES (:id, :series_id, :title, :num, :bonus)");
    query.bindValue(":id", id);
    query.bindValue(":series_id", series->getId());
    query.bindValue(":title", title);
    query.bindValue(":num", season_num);
    query.bindValue(":bonus", is_bonus);

    if (!query.exec()) {
        throw query.lastError();
    }

    series->appendChild(new_season);
    new_season->appendParent(series);
    query.prepare("UPDATE media SET in_use = True WHERE media_id = :id");
    query.bindValue(":id", id);
    query.exec();
    media[id] = new_season;
    SetIcon(id, icon);

    return new_season;
}

shared_ptr<Episode> Store::AddEpisode(int season_id, QString title,
                                      double episode_num, QImage icon) {
    return AddEpisode(dynamic_pointer_cast<Season>(GetMedia(season_id)), title,
                      episode_num, icon);
}
shared_ptr<Episode> Store::AddEpisode(std::weak_ptr<Season> weak_season,
                                      QString title, double episode_num,
                                      QImage icon) {

    std::shared_ptr<Season> season = weak_season.lock();
    QSqlQuery query;
    int id = newMediaId();
    shared_ptr<Episode> new_episode(new Episode(id, title, episode_num));
    query.prepare("INSERT INTO episode (media_id, season_id, episode_title, "
                  "episode_num) VALUES (:id, :season_id, :title, :num)");
    query.bindValue(":id", id);
    query.bindValue(":season_id", season->getId());
    query.bindValue(":title", title);
    query.bindValue(":num", episode_num);

    if (!query.exec()) {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError().text();
        throw query.lastError();
    }

    new_episode->appendParent(season);
    season->appendChild(new_episode);
    query.prepare("UPDATE media SET in_use = True WHERE media_id = :id");
    query.bindValue(":id", id);
    query.exec();
    media[id] = new_episode;
    SetIcon(id, icon);

    return new_episode;
}

void Store::SetIcon(int id, QImage icon) {
    QSqlQuery query;
    if (media.find(id) == media.end()) {
        throw std::runtime_error("No such ID");
    }
    media[id]->setPreview(icon);

    query.prepare("UPDATE media SET icon = :blob WHERE media_id = :id");

    QByteArray icon_bytes;
    QBuffer buffer(&icon_bytes);
    buffer.open(QIODevice::WriteOnly);
    icon.save(&buffer, "PNG");
    buffer.close();
    query.bindValue(":id", id);
    query.bindValue(":blob", icon_bytes);
    if (!query.exec()) {
        throw query.lastError();
    }
}

// Helper funtion that just checks if two media types are the same, or one of
// them is "ANY"
bool matchMedia(MediaType lhs, MediaType rhs) {
    if (lhs == ANY || rhs == ANY) {
        return true;
    }
    return (lhs == rhs);
}

vector<unsigned int> Store::GetIds(kedia::MediaType mediaType) {
    vector<unsigned int> out;
    for (auto it = media.begin(); it != media.end(); it++) {
        if (matchMedia(it->second->getType(), mediaType)) {
            out.push_back(it->first);
        }
    }

    return out;
}

bool Store::IsChild(int parent_id, int child_id) {
    shared_ptr<Media> child = media[child_id];
    if (parent_id == -1) {
        return (child->getParentType() == kedia::NONE);
    }

    shared_ptr<Media> parent = media[parent_id];

    return parent->hasChild(child);
}

std::shared_ptr<Drive> Store::AddDrive(QFileInfo directory, QString uuid,
                                       QString common_name, bool network,
                                       bool removeable, int rank,
                                       QString video_directory) {
    QSqlQuery query;
    query.prepare(
        "INSERT INTO drive (drive_uuid, drive_name, drive_path, "
        "drive_removeable, drive_remote, drive_rank, drive_video_directory) "
        "VALUES (:uuid, "
        ":name, :path, :removeable, :remote, :rank, :videodir)");
    query.bindValue(":uuid", uuid);
    query.bindValue(":name", common_name);
    query.bindValue(":path", directory.filePath());
    query.bindValue(":removeable", removeable);
    query.bindValue(":remote", network);
    query.bindValue(":rank", rank);
    query.bindValue(":videodir", video_directory);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }

    int id = query.lastInsertId().toInt();
    std::shared_ptr<Drive> drive(
        new Drive(id, directory, uuid, common_name, network, removeable, rank));
    drive->setVideoDirectory(QDir(video_directory));
    drives[id] = drive;

    return drive;
}

bool Store::linkFileMedia(int file_id, int media_id) {
    QSqlQuery query;
    query.prepare("INSERT INTO media_file (media_id, file_id) VALUES "
                  "(:mediaid, :fileid)");
    query.bindValue(":mediaid", media_id);
    query.bindValue(":fileid", file_id);
    if (!query.exec()) {
        throw query.lastError();
    }
    return true;
}

std::shared_ptr<File> Store::AddFile(QFileInfo fileInfo, QString checksum) {
    QSqlQuery query;

    qDebug() << fileInfo.filePath();
    QStorageInfo storage(fileInfo.filePath());
    int drive_id = identifyDrive(storage);
    if (drive_id == -1) {
        throw std::runtime_error("Drive not found");
    }

    int id = getFile(checksum);
    if (id == -1) {
        query.prepare("INSERT INTO file (file_checksum) VALUES (:checksum)");
        query.bindValue(":checksum", checksum);
        if (!query.exec()) {
            throw query.lastError();
        }
        id = query.lastInsertId().toInt();
    }

    query.prepare("INSERT INTO file_location (file_id, drive_id, "
                  "file_location_path) VALUES (:fileid, "
                  ":driveid, :path)");
    query.bindValue(":fileid", id);
    query.bindValue(":driveid", drive_id);
    query.bindValue(":path", fileInfo.filePath());
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }
    files[id] = std::shared_ptr<File>(new File(id, checksum, drives[drive_id]));
    return files[id];
}

int Store::identifyDrive(const QStorageInfo& storage) {
    QSqlQuery query;
    query.prepare("SELECT drive_id FROM drive WHERE drive_path = :path");
    query.bindValue(":path", storage.rootPath());
    if (!query.exec()) {
        throw query.lastError();
    }

    if (!query.first()) {
        return -1;
    }
    return query.value(0).toInt();
}

bool Store::episodeIsAccessible(int episode_id) {
    QFileInfo file_info = getFileLocationFromEpisode(episode_id);
    if (file_info.filePath() == "") {
        return false;
    }
    return file_info.exists();
}

QFileInfo Store::getFileLocationFromEpisode(int episode_id) {
    QSqlQuery query;

    query.prepare("SELECT drive_id, file_location_path FROM (media_file INNER "
                  "JOIN file_location USING (file_id)) WHERE media_id = :id");
    query.bindValue(":id", episode_id);

    if (!query.exec()) {
        throw query.lastError();
    }

    if (!query.first()) {
        throw std::runtime_error("No such episode.");
    }
    vector<pair<int, QString>> paths;
    do {
        paths.push_back(pair<int, QString>(query.value(0).toInt(),
                                           query.value(1).toString()));
    } while (query.next());
    return bestLocation(paths);
}

QFileInfo Store::bestLocation(vector<pair<int, QString>>& locations) {
    int bestRank = 0;

    QFileInfo bestPath;

    for (auto& loc : locations) {
        auto d = drives[loc.first];
        if (bestPath.filePath() == "" || d->getRank() > bestRank) {
            QFileInfo path(loc.second);
            if (d->connected() && path.exists()) {
                bestPath = path;
            }
        }
    }
    return bestPath;
}

int Store::getFile(QString checksum) {
    QSqlQuery query;
    query.prepare("SELECT file_id FROM file WHERE file_checksum = :checksum");
    if (!query.exec()) {
        throw query.lastError();
    }

    if (!query.first()) {
        return -1;
    }
    return query.value(0).toInt();
}

shared_ptr<Drive> Store::readDrive(unsigned int drive_id) {
    QSqlQuery query;
    query.prepare(
        "SELECT drive_uuid, drive_name, drive_path, drive_removeable, "
        "drive_remote, drive_rank, drive_video_directory FROM drive WHERE "
        "drive_id = :id");
    query.bindValue(":id", drive_id);
    if (!query.exec()) {
        throw query.lastError();
    }
    if (!query.first()) {
        throw std::runtime_error(
            (QString("No such drive ") + QString::number(drive_id))
                .toStdString());
    }
    QString uuid = query.value(0).toString();
    QString name = query.value(1).toString();
    QString path = query.value(2).toString();
    bool removeable = query.value(3).toBool();
    bool remote = query.value(4).toBool();
    int rank = query.value(5).toInt();
    QDir video_directory = QDir(query.value(6).toString());

    shared_ptr<Drive> drive(new Drive(drive_id, QFileInfo(path), uuid, name,
                                      remote, removeable, rank));
    drive->setVideoDirectory(video_directory);
    drives[drive_id] = drive;
    return drive;
}

shared_ptr<File> Store::readFile(unsigned int file_id) {
    QSqlQuery query;
    query.prepare("SELECT file_checksum, drive_id FROM file INNER "
                  "JOIN file_location USING (file_id) WHERE file_id = :id");
    query.bindValue(":id", file_id);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }
    if (!query.first()) {
        return nullptr;
    }

    QString checksum = query.value(0).toString();
    int drive_id = query.value(1).toInt();
    shared_ptr<Drive> drive = readDrive(drive_id);

    shared_ptr<File> file(new File(file_id, checksum, drive));
    while (query.next()) {
        drive = readDrive(query.value(1).toInt());
        file->appendDrive(drive->getId());
    }
    files[file_id] = file;
    return file;
}

int Store::getFileIdFromEpisode(int episode_id) {
    QSqlQuery query;
    query.prepare("SELECT file_id FROM media_file WHERE media_id = :id");
    query.bindValue(":id", episode_id);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }
    if (!query.first()) {
        return -1;
    }
    return query.value(0).toInt();
}

vector<int> Store::whichDrives(int episode_id) {
    QSqlQuery query;
    query.prepare("SELECT drive_id FROM file_location INNER JOIN media_file "
                  "USING (file_id) WHERE media_id = :id");
    query.bindValue(":id", episode_id);

    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }
    if (!query.first()) {
        return {};
    }

    vector<int> out;

    do {
        out.push_back(query.value(0).toInt());
    } while (query.next());

    return out;
}

bool Store::addFileLocation(int file_id, int drive_id, QString path) {
    QSqlQuery query;
    query.prepare(
        "SELECT file_id FROM file_location WHERE file_location_path = :path");
    query.bindValue(":path", path);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }
    if (query.first()) {
        return false;
    }
    query.prepare("INSERT INTO file_location (file_id, drive_id, "
                  "file_location_path) VALUES (:fid, :did, :path)");
    query.bindValue(":fid", file_id);
    query.bindValue(":did", drive_id);
    query.bindValue(":path", path);
    if (!query.exec()) {
        return false;
    }
    files[file_id]->appendDrive(drive_id);
    QFileInfo file_info(path);
    drives[drive_id]->addFile(files[file_id], file_info);
    return true;
}

std::vector<int> Store::getDriveIds() {
    vector<int> out;
    for (auto& drive_pair : drives) {
        out.push_back(drive_pair.first);
    }
    return out;
}

bool Store::episodeOnDrive(int episode_id, int drive_id) {
    QSqlQuery query;
    int file_id = getFileIdFromEpisode(episode_id);
    query.prepare("SELECT drive_id FROM file_location WHERE file_id = :id AND "
                  "drive_id = :did");
    query.bindValue(":id", file_id);
    query.bindValue(":did", drive_id);

    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw query.lastError();
    }
    if (query.first()) {
        return true;
    }

    return false;
}

bool Store::childrenAccessible(int media_id) {
    auto media = GetMedia(media_id);
    auto child_ids = media->getChildIds();
    if (media->getType() == EPISODE) {
        return episodeIsAccessible(media_id);
    }
    if (media->getType() == SERIES) {
        for (auto child : child_ids) {
            for (auto grandchild : GetMedia(child)->getChildIds()) {
                if (episodeIsAccessible(grandchild)) {
                    return true;
                }
            }
        }
    } else {
        for (auto child : child_ids) {
            if (episodeIsAccessible(child)) {
                return true;
            }
        }
    }
    return false;
}

bool Store::deleteMedia(int media_id) {
    QSqlQuery query;
    query.prepare("DELETE FROM media WHERE media_id = :id");
    query.bindValue(":id", media_id);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return false;
    }

    // Reset the application state
    media.clear();
    files.clear();
    drives.clear();

    ReadDatabase();
    return true;
}
