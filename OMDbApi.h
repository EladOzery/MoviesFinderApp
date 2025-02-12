#ifndef TMDB_API_H
#define TMDB_API_H

#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class TMDBApi {
public:
    explicit TMDBApi(const std::string& apiKey);
    std::vector<std::string> getPopularMovies();

private:
    std::string apiKey;
    httplib::SSLClient client;
};

#endif // TMDB_API_H
