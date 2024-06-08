#include "store.hpp"
#include <QLatin1Char>
#include <QProcess>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QTemporaryFile>
#include <iostream>
#include <qapplication.h>

using namespace std;
using namespace kedia;

QTextStream& qStdOut() {
    static QTextStream ts(stdout);
    return ts;
}
QImage getThumbnail(QFileInfo video_file) {
    QTemporaryFile tempFile("XXXXXX.png");
    QImage img;
    if (tempFile.open()) {
        QString ffmpegPath;
#ifdef FFMPEG
        ffmpegPath = FFMPEG;
#else
        qWarning("ffmpeg path wasn't provided by cmake, using /usr/bin/ffmpeg");
        ffmpegPath = "/usr/bin/ffmpeg";
#endif
        QStringList arguments = {"-i",
                                 video_file.filePath(),
                                 "-vf",
                                 "thumbnail,scale=512:288",
                                 "-hide_banner",
                                 "-loglevel",
                                 "panic",
                                 "-frames:v",
                                 "1",
                                 QDir::tempPath() + QDir::separator() +
                                     tempFile.fileName()};
        QProcess* process = new QProcess();
        int exitCode = process->execute(ffmpegPath, arguments);
        if (exitCode != 0) {
            throw std::runtime_error((QString("ffmpeg crashed with code: ") +
                                      QString::number(exitCode))
                                         .toStdString());
        }
        img =
            QImage(QDir::tempPath() + QDir::separator() + tempFile.fileName());
    }
    tempFile.close();
    return img;
}

pair<double, QString> parseSeasonName(QString dirName) {
    static QRegularExpression noTitle("Season ([\\d.]+)");
    static QRegularExpression title("Season ([\\d.]+) - (.*)");

    auto tRes = title.match(dirName);
    if (tRes.hasMatch()) {
        qDebug() << "Double: " << tRes.captured(1)
                 << ", Title: " << tRes.captured(2);
        return pair<double, QString>(tRes.captured(1).toDouble(),
                                     tRes.captured(2));
    } else {
        auto ntRes = noTitle.match(dirName);
        return pair<double, QString>(
            ntRes.captured(1).toDouble(),
            "Season " + QString::number(ntRes.captured(1).toDouble()));
    }
}

pair<double, QString> parseEpisodeName(QString dirName) {
    static QRegularExpression noTitle("E([\\d.]+)");
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
                QStringLiteral("%1").arg(num, 3, 'g', 2, QLatin1Char('0')) +
                ": " + episodeT);
    } else {
        auto ntRes = noTitle.match(dirName);
        return pair<double, QString>(ntRes.captured(1).toDouble(),
                                     "Episode " + ntRes.captured(1));
    }
}

void ImportFiles(Store& store, QDir& baseDir) {
    QStringList series = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto& s : series) {
        qDebug() << s;
        auto se = store.AddSeries(
            s, QImage(baseDir.path() + "/" + s + "/thumbnail.png"));

        QStringList seasons = QDir(baseDir.path() + "/" + s)
                                  .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (auto& season : seasons) {
            auto seasondetail = parseSeasonName(season);
            bool bonus = (trunc(seasondetail.first) != seasondetail.first);

            auto seasono =
                store.AddSeason(se->getId(), seasondetail.second,
                                seasondetail.first, bonus, se->getImage());
            QString seasonpath = baseDir.path() + "/" + s + "/" + season + "/";
            qDebug() << seasonpath;
            QDir seasondir(seasonpath);
            for (auto& episode : seasondir.entryList(QDir::Files)) {
                if (episode.endsWith(".mkv") or episode.endsWith(".mp4")) {
                    QString episodepath = seasonpath + episode;
                    auto episodedetail = parseEpisodeName(episode);
                    // qDebug() << "Checksum for " << episodepath;
                    auto f = store.AddFile(QFileInfo(episodepath), "");
                    auto e =
                        store.AddEpisode(seasono->getId(), episodedetail.second,
                                         episodedetail.first,
                                         getThumbnail(QFileInfo(episodepath)));
                    store.linkFileMedia(f->getId(), e->getId());
                }
            }
        }
    }
}

void parse(QString dir_path, kedia::Store& store) {

    store.AddDrive(QFileInfo(QStorageInfo(dir_path).rootPath()), "", "root",
                   false, false, 10, dir_path[2]);
    // false, true, 10, argv[2]);
    QDir dir(dir_path);

    ImportFiles(store, dir);
}

int listcmd(Store& store, int argc, char** argv) {
    if (argc == 2 || QString(argv[2]) == QString("series")) {
        // Print all series
        vector<unsigned int> seriesids = store.GetIds(SERIES);
        qStdOut() << "ID"
                  << "\t"
                  << "Title"
                  << "\n";
        for (auto s_id : seriesids) {
            auto s = store.GetMedia(s_id);
            qStdOut() << s->getId() << "\t" << s->getTitle() << "\n";
        }
        return 0;
    }
    int media_id = std::stoi(argv[2]);
    // Print children of whatever media id is given.
    auto m = store.GetMedia(media_id);
    if (m->hasChildren()) {
        auto child_ids = m->getChildIds();
        for (auto child_id : child_ids) {
            auto child = store.GetMedia(child_id);
            qStdOut() << child->getId() << "\t" << child->getTitle() << "\n";
        }
    }
    return 0;
}

int episodecmd(kedia::Store& store, int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }
    int episode_id = stoi(argv[2]);
    auto episode = store.GetMedia(episode_id);
    if (episode->getType() == EPISODE) {
        qStdOut() << episode->getTitle() << "\n";
        if (store.episodeIsAccessible(episode_id)) {
            qStdOut() << store.getFileLocationFromEpisode(episode_id).filePath()
                      << "\n";
        } else {
            vector<int> drive_ids = store.whichDrives(episode_id);
            for (auto drive_id : drive_ids) {
                auto drive = store.getDrive(drive_id);
                if (drive->connected()) {
                    qStdOut()
                        << "The episode is supposed to be on "
                        << drive->getName() << " but it couldn't be found.\n";
                } else {
                    qStdOut() << "Please plug in " << drive->getName()
                              << " in order to view this file.\n";
                }
            }
        }
        return 0;
    }

    qStdOut() << "No such episode " << episode_id;
    return 1;
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    kedia::Store store;
    if (argc > 2) {
        if (QString(argv[1]) == "parse") {
            for (int i = 2; i < argc; i++) {
                parse(argv[i], store);
            }
            return 0;
        }
    }
    if (argc > 1) {
        if (QString(argv[1]) == "list") {
            return listcmd(store, argc, argv);
        }
        if (QString(argv[1]) == "episode") {
            return episodecmd(store, argc, argv);
        }
    }
    return 1;
}
