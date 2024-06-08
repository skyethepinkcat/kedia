#include "file.hpp"
#include <QCryptographicHash>
#include <QFileInfo>
#include <functional>

using namespace std;
using namespace kedia;

File::File(int id_in, QString checksum_in,
           std::shared_ptr<Drive> initial_drive) {
    checksum = checksum_in;
    id = id_in;
    drives.insert(initial_drive->getId());
}

int File::getId() { return id; }

QString File::getChecksum() { return checksum; }

QString kedia::sgetChecksum(QFileInfo fileInfo, qint64* progress_count,
                            qint64* total) {

    // Load the file
    QFile file(fileInfo.filePath());

    if (file.open(QFile::ReadOnly)) {
        // Use QT's sha256 implementation.
        QCryptographicHash hash(QCryptographicHash::Sha256);
        // If we were given progress ints, we need to update them as we run.
        if (progress_count != nullptr && total != nullptr) {
            *total = (file.size() / 5120) + 1;
            for ((*progress_count) = 0; *progress_count < *total;
                 (*progress_count)++) {
                hash.addData(file.read(5120));
            }
        } else {
            // Otherwise, we can just use the regular function.
            hash.addData(&file);
        }
        // Returns the checksum.
        QString check = hash.result().toHex();
        return check;
    }
    throw QFile::FileError();
}

bool File::verifyChecksum(QString checksum_in, QFileInfo fileInfo,
                          qint64* progress_count, qint64* total) {
    return (checksum_in == (sgetChecksum(fileInfo, progress_count, total)));
}

void File::appendDrive(int drive_id) {
    if (drives.find(drive_id) == drives.end()) {
        drives.insert(drive_id);
    }
}
