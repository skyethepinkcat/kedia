#pragma once
#include "media.hpp"
#include <QString>

namespace kedia {
class Series : public Media {
  public:
    //! Title Constructor
    Series(int id, QString title = "Default", QImage icon = QImage());
};

} // namespace kedia
