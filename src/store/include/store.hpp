#pragma once
#include "database.hpp"
#include "drive.hpp"
#include "episode.hpp"
#include "file.hpp"
#include "media.hpp"
#include "season.hpp"
#include "series.hpp"
#include <QDir>
#include <QStorageInfo>

namespace kedia {

class Store {
    friend class Database;

  public:
    // Initalizes the store with the given database.
    Store(QString db_name = "kedia.sqlite3");

    // Returns a pointer to the media with the given id.
    std::shared_ptr<Media> GetMedia(int id);

    // Reads the database, reseting the current cached data.
    void ReadDatabase();

    // Creates a series and adds it to the database.
    std::shared_ptr<Series> AddSeries(QString title, QImage icon = QImage());

    // Creates a season and adds it to the database.
    std::shared_ptr<Season> AddSeason(std::weak_ptr<Series> series,
                                      QString title, double season_num,
                                      bool is_bonus, QImage icon = QImage());

    // Creates a season and adds it to the database.
    std::shared_ptr<Season> AddSeason(int series, QString title,
                                      double season_num, bool is_bonus,
                                      QImage icon = QImage());

    // Creates an episode and adds it to the database.
    std::shared_ptr<Episode> AddEpisode(std::weak_ptr<Season> season,
                                        QString title, double episode_num,
                                        QImage icon = QImage());

    // Creates an episode and adds it to the database.
    std::shared_ptr<Episode> AddEpisode(int season, QString title,
                                        double episode_num,
                                        QImage icon = QImage());

    // Creates a drive and adds it to the database.
    std::shared_ptr<Drive> AddDrive(QFileInfo directory, QString uuid,
                                    QString common_name = "",
                                    bool network = false,
                                    bool removeable = false, int rank = 0,
                                    QString video_directory = "");

    // The media icon, give an id, and store it in the database.
    void SetIcon(int id, QImage icon);

    // Adds a file to the database.
    std::shared_ptr<File> AddFile(QFileInfo fileInfo, QString checksum);

    // Returns all ids of the given media type.
    std::vector<unsigned int>
    GetIds(kedia::MediaType type = kedia::MediaType::ANY);

    // Returns true if the given child media id is a child of the given parent
    // media id.
    bool IsChild(int parent_id, int child_id);

    // Searches for a file by its checksum. Returns -1 if it cannot be found,
    // otherwise returns the file id.
    int getFile(QString checksum);

    // Returns the drive identified by drive_id. Returns a nullptr if it cannot
    // be found.
    std::shared_ptr<kedia::Drive> getDrive(int drive_id) {
        try {
            return drives[drive_id];
        } catch (std::exception) {
            return nullptr;
        };
    };

    // Attempts to find the drive from a storage info object. Returns -1 if it
    // cannot be found.
    int identifyDrive(const QStorageInfo& storage);

    // Creates a new connection between a file and connected media.
    bool linkFileMedia(int file_id, int media_id);

    // Returns the file location of an episode by its id.
    QFileInfo getFileLocationFromEpisode(int episode_id);

    // Returns true if any file location of the episode exists.
    bool episodeIsAccessible(int episode_id);

    // Returns true if an episode has a copy on the given drive.
    bool episodeOnDrive(int episode_id, int drive_id);

    // Get the file id associated with an episode.
    int getFileIdFromEpisode(int episode);

    // Adds a new location for a file.
    bool addFileLocation(int file_id, int drive_id, QString path);

    // Returns a vector of all the drives the episode is on.
    std::vector<int> whichDrives(int episode_id);

    // Returns a vector of all drive ids.
    std::vector<int> getDriveIds();

    // Returns true if any single child is acessible.
    bool childrenAccessible(int media_id);

    // Deletes the given media by its id, and all of it its children. Returns -1
    // if no media is found.
    bool deleteMedia(int media_id);

  private:
    // Reads some given media from the database by its id.
    std::shared_ptr<Media> readMedia(unsigned int media_id);

    // Reads a given series from the database by its id.
    std::shared_ptr<Series> readSeries(unsigned int media_id);

    // Reads a given season from the database by its id.
    std::shared_ptr<Season> readSeason(unsigned int media_id);

    // Reads a given episode from the database by its id.
    std::shared_ptr<Episode> readEpisode(unsigned int media_id);

    // Reads a given drive from the database by its id.
    std::shared_ptr<Drive> readDrive(unsigned int drive_id);

    // Reads a given file from the database by its id.
    std::shared_ptr<File> readFile(unsigned int file_id);

    // Returns the best file location from a list of locations, with the
    // location being a pair of a drive id and the file path.
    QFileInfo bestLocation(std::vector<std::pair<int, QString>>& locations);

    // Loads the icon for a certain media identified by its id.
    QImage readIcon(unsigned int media_id);

    // Creates a new unique media id.
    int newMediaId();

    std::unique_ptr<Database> db;
    std::unordered_map<unsigned int, std::shared_ptr<Media>> media;
    std::unordered_map<unsigned int, std::shared_ptr<Drive>> drives;
    std::unordered_map<unsigned int, std::shared_ptr<File>> files;
};

} // namespace kedia
