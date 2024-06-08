#pragma once
#include "file.hpp"
#include <QDir>
#include <QFileInfo>
#include <QString>
#define CHECK_REMOVE_DRIVE                                                     \
    if (!connected()) {                                                        \
        throw DriveDisconnected();                                             \
    }

namespace kedia {
class File;
class Drive {
  public:
    // Drive id is a unique drive identifier, directory refers to the directory
    // the drive is mounted on, uuid is the filesystem uuid, common_name is the
    // optional user-facing name for the drive, network should be true if it is
    // a network drive, removeable if the drive can be removed, such as a
    // thumbdrive, and rank allows certain drives to be preferred over others, a
    // higher number is better and the default is 0.
    Drive(int id, QFileInfo directory, QString uuid, QString common_name = "",
          bool network = false, bool removeable = false, int rank = 0);

    // Returns true if the drive is currently mounted. This is determined by the
    // existence of the directory it is mounted on, under the assumption it is
    // mounted in a place like /run/media/user/ where the directory is
    // automatically removed when unmounted. Eventually this will be done
    // through the filesystem uuid.
    bool connected();

    // Returns the path the drive is mounted on.
    QString path();

    // Adds the given file to the drive at the given location.
    void addFile(std::shared_ptr<kedia::File> kedia_file, QFileInfo& fileinfo);

    // Returns the drive's unique id.
    int getId();

    // Sets the directory videos are stored at.
    void setVideoDirectory(QDir video_directory_in) {
        video_directory = video_directory_in;
    }

    // Returns the directory videos are stored at.
    QDir getVideoDirectory() { return video_directory; }

    // Returns all files on the drive as a pair, the first value being the file
    // id and the second being the path.
    std::vector<std::pair<int, const QFileInfo&>> getFiles();

    // Returns the rank of the drive.
    int getRank() { return rank; };

    // Returns the drive's given name, or its path if none exists.
    QString getName() {
        if (common_name == "") {
            return directory.filePath();
        } else {
            return common_name;
        }
    };

    // Returns true if the drive can be expected to be removed sometimes.
    bool getRemoveable() { return removeable; }

    class DriveDisconnected : public std::exception {
      public:
        std::string what() { return "This drive is disconneted."; }
    };

  private:
    int id;
    QFileInfo directory;
    bool network;
    bool removeable;
    std::unordered_map<int, QFileInfo> files;
    int rank;
    QString common_name;
    QString uuid;
    QDir video_directory;
};
} // namespace kedia
