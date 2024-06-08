#pragma once
#include "store.hpp"
#include <QDialog>
#include <QFileInfo>
namespace Ui {
class CreateDrive;
}
namespace kedia {
class CreateDrive : public QDialog {
    Q_OBJECT

  public:
    explicit CreateDrive(std::shared_ptr<kedia::Store> store, QString filePath,
                         QWidget* parent = nullptr);
    ~CreateDrive();

  public slots:
    void accept();
    void selectVideoPath();
    void verify();

  private slots:
    void networkChanged(int);

  private:
    Ui::CreateDrive* ui;
    QFileInfo drivePath;
    std::shared_ptr<Store> store;
};
} // namespace kedia
