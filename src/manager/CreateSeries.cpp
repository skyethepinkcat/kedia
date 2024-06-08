#include "CreateSeries.h"
#include "ui_CreateSeries.h"
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <qvalidator.h>

using namespace std;
using namespace kedia;

CreateSeries::CreateSeries(shared_ptr<Store> store_in, QWidget* parent)
    : QDialog(parent), ui(new Ui::CreateSeries) {
    ui->setupUi(this);
    store = store_in;

    ui->TitleEdit->setValidator(NOT_EMPTY_VALIDATOR);
    ui->buttonBox->setDisabled(true);
    connect(ui->FileAdd, &QPushButton::pressed, this,
            &CreateSeries::FileAddPressed);
    connect(ui->TitleEdit, &QLineEdit::textEdited, this,
            &CreateSeries::TextChanged);
}

CreateSeries::~CreateSeries() { delete ui; }

void CreateSeries::TextChanged() {
    if (ui->TitleEdit->hasAcceptableInput()) {
        ui->buttonBox->setEnabled(true);
    } else {
        ui->buttonBox->setDisabled(true);
    }
}

void CreateSeries::accept() {
    QImage img;
    if (file.path().isEmpty()) {
        img = QImage();
    } else {
        img = QImage(file.filePath());
    }
    store->AddSeries(ui->TitleEdit->text(), img);
    done(Accepted);
}

void CreateSeries::FileAddPressed() {
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
    ui->FileAdd->setText(file.fileName());
    QImage img = QImage(file.filePath());
    if (img.size() != QSize(460, 690)) {
        QMessageBox::warning(this, tr("Incorrect Icon Size"),
                             tr("This image does not seem to be 460 by 690, "
                                "this may cause bugs."));
    }
}

#include "moc_CreateSeries.cpp"
