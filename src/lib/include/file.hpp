#pragma once
#include "drive.hpp"
#include <QFileInfo>
#include <QString>
#include <set>
namespace kedia {

// Static function to get a file's checksum. progress_count and total can be
// used to check the checksum progress.
QString sgetChecksum(QFileInfo fileInfo, qint64* progress_count = nullptr,
                     qint64* total = nullptr);
class Drive;
class File {
  public:
    //! Default constructor
    File(int id, QString checksum, std::shared_ptr<kedia::Drive> initial_drive);

    // Returns true if the file is present on the given drive.
    bool onDrive(std::weak_ptr<Drive> drive);

    // Intended to be run in a seperate thread. If progress count and total are
    // not nullptrs, they are used to send the progress for loading
    // bars.
    static bool verifyChecksum(QString checksum, QFileInfo fileInfo,
                               qint64* progress_count = nullptr,
                               qint64* total = nullptr);

    // Returns the stored checksum of the file.
    QString getChecksum();

    // Add a drive this file is present on.
    void appendDrive(int drive_id);

    // Return the id of this file.
    int getId();

  protected:
    int id;
    QString checksum;
    std::set<int> drives;
};

} // namespace kedia
