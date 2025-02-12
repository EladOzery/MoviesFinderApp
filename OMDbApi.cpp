#include "TMDBApi.h"
#include <iostream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

TMDBApi::TMDBApi(const std::string& apiKey)
        : apiKey(apiKey), client("api.themoviedb.org", 443) {  // Use SSLClient for HTTPS
}



// Function to fetch popular movies
std::vector<std::string> TMDBApi::getPopularMovies() {
    std::vector<std::string> movieTitles;
    std::string endpoint = "/3/movie/popular?api_key=" + apiKey;

    auto res = client.Get(endpoint.c_str());

    if (res && res->status == 200) {
        json response = json::parse(res->body);

        // Extract movie titles from JSON response
        for (const auto& movie : response["results"]) {
            movieTitles.push_back(movie["title"]);
        }
    } else {
        std::cerr << "Failed to fetch data. HTTP Status: "
                  << (res ? res->status : 0) << std::endl;
    }

    return movieTitles;
}
