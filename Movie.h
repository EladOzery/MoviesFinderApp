#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <GL/gl.h>


struct Movie {
    std::string title;
    std::string year;
    std::string genre;
    std::string imdbRating;
    std::string posterUrl;  // Store poster URL
    GLuint texture=-1;
};

#endif // MOVIE_H
