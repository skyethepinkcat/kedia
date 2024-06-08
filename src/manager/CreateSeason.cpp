#include "CreateSeason.h"
#include "CreateSeries.h"
#include "manager_utils.hpp"
#include "ui_CreateSeason.h"
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QMessageBox>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qnamespace.h>
#include <qvalidator.h>

using namespace std;
using namespace kedia;
CreateSeason::CreateSeason(shared_ptr<Store> store_in, int parent_id,
                           QWidget* parent)
    : QDialog(parent), ui(new Ui::CreateSeason) {
    ui->setupUi(this);
    store = store_in;

    ui->TitleEdit->setValidator(NOT_EMPTY_VALIDATOR);
    ui->imagePreview->setScene(new QGraphicsScene());
    shared_ptr<Media> parent_media;
    // If we don't have a parent, there's no reason to show the image preview
    // yet.
    if (parent_id == -1) {
        ui->imagePreview->setHidden(true);
        ui->resetButton->setHidden(true);
    } else {
        // Get the parent's icon and set it as the current image.
        parent_media = store->GetMedia(parent_id);
        SetImage(parent_media->getImage());

        ui->FileAdd->setText("From Parent...");
    }

    connect(ui->FileAdd, &QPushButton::pressed, this,
            &CreateSeason::FileAddPressed);
    connect(ui->TitleEdit, &QLineEdit::textEdited, this, &CreateSeason::verify);
    connect(ui->seriesList, &QComboBox::currentIndexChanged, this,
            &CreateSeason::ListChanged);
    connect(ui->seriesList, &QComboBox::currentIndexChanged, this,
            &CreateSeason::verify);
    connect(ui->isBonus, &QCheckBox::stateChanged, this,
            &CreateSeason::BonusChanged);
    connect(ui->resetButton, &QPushButton::pressed, this,
            &CreateSeason::ResetImage);
    connect(ui->numberSpin, &QDoubleSpinBox::valueChanged, this,
            &CreateSeason::SpinBoxChanged);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    if (parent_media != nullptr) {
        int seasonNumber = parent_media->getChildren().size() + 1;
        ui->numberSpin->setValue(parent_media->getChildren().size() + 1);
    }

    setupList(parent_id);
    verify();
}

void CreateSeason::SetImage(QImage img) {
    ui->imagePreview->scene()->clear();
    ui->imagePreview->scene()->addPixmap(QPixmap::fromImage(img));
}

void CreateSeason::setupList(int media_parent) {

    auto ids = store->GetIds(SERIES);

    int selected_index = -1;
    for (int id : ids) {
        auto media = store->GetMedia(id);
        ui->seriesList->addItem(media->getTitle(), media->getId());
        if (id == media_parent) {
            selected_index = ui->seriesList->findData(media->getId());
        }
    }
    ui->seriesList->setCurrentIndex(selected_index);
}

CreateSeason::~CreateSeason() { delete ui; }

void CreateSeason::TextChanged() { verify(); }

void CreateSeason::accept() {
    QImage img;
    if (file.path().isEmpty()) {
        img =
            store->GetMedia(ui->seriesList->currentData().toInt())->getImage();
    } else {
        img = QImage(file.filePath());
    }
    shared_ptr<Media> parent =
        store->GetMedia(ui->seriesList->currentData().toInt());

    store->AddSeason(dynamic_pointer_cast<Series>(parent),
                     ui->TitleEdit->text(), ui->numberSpin->value(),
                     ui->isBonus->isChecked(), img);
    done(Accepted);
}

void CreateSeason::verify() {
    if (ui->TitleEdit->hasAcceptableInput() &&
        ui->seriesList->currentIndex() != -1) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else if (!ui->TitleEdit->isModified()) {
        ui->TitleEdit->setText(
            QString("Season ") +
            QString::number(ui->numberSpin->value(), 'g', 10));
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}
void CreateSeason::FileAddPressed() {
    QString openPath;
    if (file.filePath().isEmpty()) {
        openPath = QDir::homePath();
    } else {
        openPath = file.dir().path();
    }
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Image"), openPath,
                                     tr("Image Files (*.png *.jpg *.bmp)"));
    file = QFileInfo(filename);
    QImage img = QImage(file.filePath());
    if (img.size() != QSize(460, 690)) {
        QMessageBox::warning(this, tr("Incorrect Icon Size"),
                             tr("This image does not seem to be 460 by 690, "
                                "this may cause bugs."));
    }
    SetImage(img);
    manualimg = true;
    ui->FileAdd->setText(file.fileName());
    ui->imagePreview->setHidden(false);
}

void CreateSeason::ListChanged(int i) {
    QList<QWidget*> widgetsToChange = {ui->FileAdd, ui->IconLabel,
                                       ui->TitleLabel, ui->TitleEdit,
                                       ui->resetButton};
    if (i != -1) {
        setWidgetsEnabled(widgetsToChange);
        if (!manualimg) {
            SetImage(
                getMediaFromIndex(ui->seriesList->currentIndex())->getImage());
        }
    } else {
        setWidgetsEnabled(widgetsToChange, false);
    }
}

void CreateSeason::SpinBoxChanged(double value) {
    if (!manualtext) {
        QString number;
        ui->TitleEdit->setText(QString("Season ") +
                               number.setNum(value, 'g', 10));
    }
}
shared_ptr<Media> CreateSeason::getMediaFromIndex(int item) {
    if (item == -1) {
        throw std::runtime_error("No such item.");
    }
    int id = ui->seriesList->currentData().toInt();
    return store->GetMedia(id);
}

void CreateSeason::BonusChanged(int bonus) {
    if (bonus == Qt::Checked) {
        ui->numberSpin->setDecimals(1);
    } else {
        ui->numberSpin->setDecimals(0);
    }
}

void CreateSeason::ResetImage() {
    manualimg = false;
    ui->FileAdd->setText("From Parent...");
    file = QFileInfo();
    SetImage(getMediaFromIndex(ui->seriesList->currentIndex())->getImage());
}

#include "moc_CreateSeason.cpp"
