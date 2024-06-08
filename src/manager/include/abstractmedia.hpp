#pragma once
#include "media.hpp"
#include <QIcon>
#include <QString>
#include <stdexcept>

class AbstractMedia : public kedia::Media {

  public:
    AbstractMedia(int id_in)
        : Media(id_in, kedia::MediaType::NONE, kedia::MediaType::NONE,
                kedia::MediaType::ANY) {
        if (id_in != -1) {
            throw std::invalid_argument(
                "Why are you trying to create an abstract "
                "media that isn't the root?");
        }
    };

    QString getTitle() { return QString(""); };
    QIcon getPreview() { return QIcon(); };

  private:
    std::vector<std::shared_ptr<kedia::Media>> children;
};
