#pragma once
#include "media.hpp"
#include <QIcon>
#include <QString>

namespace kedia {
class Episode : public Media {
  public:
    //! Default constructor
    Episode(int id_in, QString title, double episode_number,
            QImage icon = QImage());

    // Returns the episode number.
    double getNumber();

  protected:
    double episode_number;
};

} // namespace kedia
