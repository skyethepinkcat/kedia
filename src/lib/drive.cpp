#include "drive.hpp"
#include <QFileInfo>

using namespace std;
using namespace kedia;

Drive::Drive(int id_in, QFileInfo directory_in, QString uuid,
             QString common_name_in, bool network_in, bool removeable_in,
             int rank_in) {
    id = id_in;
    directory = directory_in;
    network = network_in;
    removeable = removeable_in;
    common_name = common_name_in;
    rank = rank_in;
}

void Drive::addFile(std::shared_ptr<File> kedia_file, QFileInfo& fileInfo) {
    int file_id = kedia_file->getId();

    if (files.find(file_id) != files.end()) {
        return;
    }
    files[file_id] = fileInfo;
    return;
}

int Drive::getId() { return id; }

vector<pair<int, const QFileInfo&>> Drive::getFiles() {
    vector<pair<int, const QFileInfo&>> out;
    for (auto p : files) {
        out.push_back(pair<int, const QFileInfo&>(p.first, p.second));
    }
    return out;
}

bool Drive::connected() { return directory.exists(); }
