#include "episode.hpp"

using namespace kedia;
using namespace std;

Episode::Episode(int id_in, QString title_in, double episode_number_in,
                 QImage icon_in)
    : Media(id_in, MediaType::EPISODE, MediaType::SEASON, MediaType::NONE) {
    setTitle(title_in);
    setPreview(icon_in);
    episode_number = episode_number_in;
}

double Episode::getNumber() { return episode_number; }
