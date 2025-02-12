#ifndef OMDB_API_H
#define OMDB_API_H

#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


/**
 * @class OMDbApi
 * @brief A class to interact with the OMDb API for movie information.
 *
 * This class provides methods to search for movies using the OMDb API.
 */

/**
 * @brief Constructs an OMDbApi object with the given API key.
 *
 * @param apiKey The API key for accessing the OMDb API.
 */

/**
 * @brief Searches for movies based on the given query.
 *
 * @param query The search query string.
 * @return A vector of movie titles that match the search query.
 */
class OMDbApi {
public:
    explicit OMDbApi(const std::string& apiKey);
    std::vector<std::string> searchMovies(const std::string& query);

private:
    std::string apiKey;
    httplib::Client client; // Using HTTP (no SSL needed)
};

#endif // OMDB_API_H
