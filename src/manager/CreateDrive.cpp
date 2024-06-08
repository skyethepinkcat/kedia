#include "CreateDrive.h"
#include "ui_CreateDrive.h"
#include <QFileDialog>
using namespace kedia;
using namespace std;

CreateDrive::CreateDrive(std::shared_ptr<Store> store_in, QString filePath_in,
                         QWidget* parent)
    : QDialog(parent), ui(new Ui::CreateDrive) {
    ui->setupUi(this);
    drivePath = QFileInfo(filePath_in);

    qDebug() << filePath_in;
    ui->filePath->setText(filePath_in);
    ui->videoPathEdit->setText(filePath_in + QDir::separator() + "Anime");
    store = store_in;
    connect(ui->networkBox, &QCheckBox::stateChanged, this,
            &CreateDrive::networkChanged);
    connect(ui->videoPathEdit, &QLineEdit::textChanged, this,
            &CreateDrive::verify);
    connect(ui->browseButton, &QPushButton::pressed, this,
            &CreateDrive::selectVideoPath);
}

CreateDrive::~CreateDrive() { delete ui; };

void CreateDrive::verify() {
    QString string_videopath = ui->videoPathEdit->text();
    QFileInfo file_videopath(string_videopath);
    QStorageInfo storage_videopath(string_videopath);
    QString string_drivepath = ui->filePath->text();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (!file_videopath.exists()) {
        ui->videoPathEdit->setToolTip("Path does not exist.");
        return;
    }
    if (storage_videopath.rootPath() != string_drivepath) {
        ui->videoPathEdit->setToolTip("Path is not on drive.");
        return;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void CreateDrive::networkChanged(int check) {
    if (check == Qt::CheckState::Checked) {
        ui->removeBox->setChecked(true);
        ui->removeBox->setEnabled(false);
    } else {
        ui->removeBox->setEnabled(true);
    }
}

void CreateDrive::accept() {
    store->AddDrive(drivePath, "", ui->nameEdit->text(),
                    ui->networkBox->isChecked(), ui->removeBox->isChecked(),
                    ui->rankSpin->value(), ui->videoPathEdit->text());
    done(Accepted);
}

void CreateDrive::selectVideoPath() {
    QString filename = QFileDialog::getExistingDirectory(
        this, tr("Open Video Directory"), drivePath.filePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->videoPathEdit->setText(filename);
}

#include "moc_CreateDrive.cpp"
