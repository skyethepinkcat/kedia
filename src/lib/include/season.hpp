#pragma once
#include "media.hpp"
#include <QIcon>
#include <QString>

namespace kedia {
class Season : public Media {
  public:
    //! Default constructor
    Season(int id_in, QString title, double season_num, bool is_bonus = false,
           QImage icon = QImage());

    // Returns the season number.
    double getNumber();

  protected:
    double season_num;
    bool bonus;
};

} // namespace kedia
