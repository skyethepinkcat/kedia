#include "CreateEpisode.h"
#include "CreateDrive.h"
#include "CreateSeries.h"
#include "file.hpp"
#include "manager_utils.hpp"
#include "ui_CreateEpisode.h"
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qnamespace.h>
#include <qvalidator.h>
#include <unistd.h>

using namespace std;
using namespace kedia;
CreateEpisode::CreateEpisode(shared_ptr<Store> store_in, int parent_id,
                             QWidget* parent)
    : QDialog(parent), ui(new Ui::CreateEpisode) {
    ui->setupUi(this);
    store = store_in;

    ui->progressBar->setHidden(true);
    ui->TitleEdit->setValidator(NOT_EMPTY_VALIDATOR);
    ui->imagePreview->setScene(new QGraphicsScene());
    shared_ptr<Media> parent_media = store->GetMedia(parent_id);

    connect(ui->FileAdd, &QPushButton::pressed, this,
            &CreateEpisode::FileAddPressed);
    connect(ui->TitleEdit, &QLineEdit::textEdited, this,
            &CreateEpisode::verify);
    connect(ui->seriesList, &QComboBox::currentIndexChanged, this,
            &CreateEpisode::ListChanged);
    connect(ui->seriesList, &QComboBox::currentIndexChanged, this,
            &CreateEpisode::verify);
    connect(ui->isBonus, &QCheckBox::stateChanged, this,
            &CreateEpisode::BonusChanged);
    connect(ui->numberSpin, &QDoubleSpinBox::valueChanged, this,
            &CreateEpisode::SpinBoxChanged);

    if (parent_media != nullptr) {
        int episodeNumber = parent_media->getChildren().size() + 1;
        ui->numberSpin->setValue(parent_media->getChildren().size() + 1);
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    setupList(parent_id);
    verify();
}

void CreateEpisode::SetImage(QImage img) {
    ui->imagePreview->scene()->clear();
    ui->imagePreview->scene()->addPixmap(QPixmap::fromImage(img));
    curr_image = img;
}

void CreateEpisode::setupList(int media_parent) {

    auto season = store->GetMedia(media_parent);
    auto series = season->getParent().lock();

    auto series_ids = store->GetIds(SERIES);
    auto season_ids = season->getSiblingIds();

    int selected_index = -1;

    // Add all the seasons
    for (int id : season_ids) {
        auto media = store->GetMedia(id);
        ui->seasonList->addItem(media->getTitle(), media->getId());
        if (id == season->getId()) {
            selected_index = ui->seasonList->findData(media->getId());
        }
    }
    ui->seasonList->setCurrentIndex(selected_index);
    selected_index = -1;

    // Add all the series
    for (int id : series_ids) {
        auto media = store->GetMedia(id);
        ui->seriesList->addItem(media->getTitle(), media->getId());
        if (id == series->getId()) {
            selected_index = ui->seriesList->findData(media->getId());
        }
    }

    ui->seriesList->setCurrentIndex(selected_index);
}

CreateEpisode::~CreateEpisode() { delete ui; }

void CreateEpisode::TextChanged() { verify(); }

void CreateEpisode::updateBar(long long newVal) {
    ui->progressBar->setValue(newVal);
}

void CreateEpisode::accept() {
    QString checksum;

    if (ui->checksumBox->isChecked()) {
        // Hypothetically, this should make the window look busy. In practice,
        // this was incorrectly implemented.
        setCursor(Qt::BusyCursor);
        this->setToolTip("Computing checksum, please wait.");
        checksum = sgetChecksum(file);
        unsetCursor();
        this->setToolTip("");
    } else {
        checksum = "";
    }

    shared_ptr<File> f;

    try {
        f = store->AddFile(file, checksum);
    } catch (std::runtime_error e) {
        // If the error we catch is that a drive couldn't be found, we need to
        // create a drive.
        if (strcmp(e.what(), "Drive not found") == 0) {
            CreateDrive cd(store, QStorageInfo(file.filePath()).rootPath(),
                           this);
            auto res = cd.exec();
            if (res == QDialog::Accepted) {
                f = store->AddFile(file, checksum);
            } else {
                return;
            }
        }
    }
    auto ep = store->AddEpisode(ui->seasonList->currentData().toInt(),
                                ui->TitleEdit->text(), ui->numberSpin->value(),
                                curr_image);
    store->linkFileMedia(f->getId(), ep->getId());
    done(Accepted);
}

void CreateEpisode::verify() {
    if (ui->TitleEdit->hasAcceptableInput() &&
        ui->seriesList->currentIndex() != -1 && curr_image != QImage() &&
        file.filePath() != "") {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else if (!ui->TitleEdit->isModified()) {
        ui->TitleEdit->setText(
            QString("Episode ") +
            QString::number(ui->numberSpin->value(), 'g', 10));
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void CreateEpisode::FileAddPressed() {
    QString openPath;
    if (file.filePath().isEmpty()) {
        openPath = QDir::homePath();
    } else {
        openPath = file.dir().path();
    }
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open Image"), openPath,
        tr("Video Files (*.mkv *.mp4 *.webm *.mpeg)"));
    file = QFileInfo(filename);
    QImage img;
    try {
        img = getThumbnail(file);
    } catch (std::runtime_error e) {
        return;
    }
    SetImage(img);
    QFontMetrics fontm(ui->FileAdd->fontMetrics());

    QString displayFilename = file.fileName();

    qDebug() << ui->FileAdd->width();
    while (fontm.size(Qt::TextSingleLine, displayFilename).width() >
           ui->FileAdd->width()) {
        displayFilename.chop(6);
        displayFilename.append("...");
    }
    ui->FileAdd->setText(displayFilename);
    verify();
}

void CreateEpisode::ListChanged(int i) {
    QList<QWidget*> widgetsToChange = {ui->FileAdd, ui->IconLabel,
                                       ui->TitleLabel, ui->TitleEdit};
    if (i != -1) {
        setWidgetsEnabled(widgetsToChange);
    } else {
        setWidgetsEnabled(widgetsToChange, false);
    }
}

void CreateEpisode::SpinBoxChanged(double value) {
    if (!manualtext) {
        QString number;
        ui->TitleEdit->setText(QString("Episode ") +
                               number.setNum(value, 'g', 10));
    }
}
shared_ptr<Media> CreateEpisode::getMediaFromIndex(int item) {
    if (item == -1) {
        throw std::runtime_error("No such item.");
    }
    int id = ui->seriesList->currentData().toInt();
    return store->GetMedia(id);
}

void CreateEpisode::BonusChanged(int bonus) {
    if (bonus == Qt::Checked) {
        ui->numberSpin->setDecimals(1);
    } else {
        ui->numberSpin->setDecimals(0);
    }
}

QImage CreateEpisode::getThumbnail(QFileInfo video_file, QWidget* parent) {
    // Creates a temporary file to store the image.
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

        QStringList arguments = {
            // Seek 4 minutes into the video first
            "-ss", "00:04:00",
            // Input the given video file.
            "-i", video_file.filePath(),
            // Apply filters thumbnail and scale to make an image of size
            // 512x288
            // and
            // to select the most representative image out of 100 frames.
            "-vf", "thumbnail,scale=512:288",
            // Hide the banner output.
            "-hide_banner",
            // Hide logs of less than panic level.
            "-loglevel", "panic",
            // Only output one frame.
            "-frames:v", "1",
            // Output the single frame to a temporary file.
            QDir::tempPath() + QDir::separator() + tempFile.fileName()};

        QProcess* process = new QProcess(parent);
        int exitCode = process->execute(ffmpegPath, arguments);

        if (exitCode != 0) {
            QMessageBox::warning(parent, "ffmpeg crash",
                                 (QString(tr("ffmpeg crashed with code: ")) +
                                  QString::number(exitCode)));
            tempFile.close();
            return QImage();
        }

        img =
            QImage(QDir::tempPath() + QDir::separator() + tempFile.fileName());
    }
    tempFile.close();
    return img;
}

#include "moc_CreateEpisode.cpp"
