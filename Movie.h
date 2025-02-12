#ifndef MOVIE_H
#define MOVIE_H

#include <string>

struct Movie {
    std::string title;
    std::string year;
    std::string genre;
    std::string imdbRating;
    std::string posterUrl;  // Store poster URL
};

#endif // MOVIE_H
