#include "series.hpp"

using namespace kedia;
using namespace std;

Series::Series(int id_in, QString title_in, QImage icon_in)
    : Media(id_in, MediaType::SERIES, MediaType::NONE, MediaType::SEASON) {
    setTitle(title_in);
    setPreview(icon_in);
}
