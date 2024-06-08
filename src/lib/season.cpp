#include "season.hpp"

using namespace kedia;
using namespace std;

Season::Season(int id_in, QString title_in, double season_num_in, bool is_bonus,
               QImage icon_in)
    : Media(id_in, MediaType::SEASON, MediaType::SERIES, MediaType::EPISODE) {
    setTitle(title_in);
    setPreview(icon_in);
    season_num = season_num_in;
    bonus = is_bonus;
}

double Season::getNumber() { return season_num; }
