#include "OMDbApi.h"
#include "ImageLoader.h"
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>
// JSON alias
using json = nlohmann::json;

// Constructor: Initialize API client
OMDbApi::OMDbApi(const std::string &apiKey)
    : apiKey(apiKey), client("www.omdbapi.com", 80) {}

/**
 * @brief Searches for movies using the OMDb API based on the provided query.
 *
 * This function sends a search request to the OMDb API with the given query,
 * retrieves the search results, and then fetches additional details for each
 * movie, including genre and IMDb rating. It also attempts to download the
 * movie poster and save it to the cache directory.
 *
 * @param query The search query string.
 * @return A vector of strings containing movie titles along with their year,
 *         genre, IMDb rating, and IMDb ID.
 *
 * The format of each string in the returned vector is:
 * "Title (Year) (Genre) (IMDb Rating) (IMDb ID)"
 *
 * @note The function logs debug information and errors to the standard output
 *       and standard error streams, respectively.
 * @note The function uses the `client` object to send HTTP requests and the
 *       `json` library to parse JSON responses.
 * @note The function assumes that the `apiKey` member variable is set with a
 *       valid OMDb API key.
 */
std::vector<std::string> OMDbApi::searchMovies(const std::string &query) {
    std::vector<std::string> movieTitles;// Vector to store movie titles

    std::string formattedQuery = query;                                  // Replace spaces with '+' in query
    std::replace(formattedQuery.begin(), formattedQuery.end(), ' ', '+');// Replace spaces with '+'
    // Build the endpoint URL
    std::string endpoint = "/?apikey=" + apiKey + "&s=" + formattedQuery;
    // Send the search request to the OMDb API
    auto res = client.Get(endpoint.c_str());
    // Check if the request was successful
    if (!res) {
        std::cerr << "ERROR: Failed to connect to OMDb API." << std::endl;
        return movieTitles;
    }

    // Check if the API returned an error
    if (res->status != 200) {
        std::cerr << "ERROR: OMDb API returned HTTP " << res->status << std::endl;
        return movieTitles;
    }
    // Parse the JSON response
    try {
        json response = json::parse(res->body);
        std::cout << "DEBUG: Raw API Response: " << response.dump(2) << std::endl;
        // Check if the response contains the 'Search' field
        if (response.contains("Search")) {
            // Iterate over each movie in the search results
            for (const auto &movie: response["Search"]) {
                std::string title = movie.value("Title", "Unknown");// Get movie title
                std::string year = movie.value("Year", "Unknown");  // Get movie year
                std::string imdbID = movie.value("imdbID", "");     // Get IMDb ID

                std::string genre = "Unknown";     // Initialize genres
                std::string imdbRating = "Unknown";// Initialize IMDb rating
                // Fetch additional details for the movie
                if (!imdbID.empty()) {
                    std::string detailsEndpoint = "/?apikey=" + apiKey + "&i=" + imdbID;// Build the details endpoint URL
                    std::cout << "DEBUG: Fetching details for " << title << " using " << detailsEndpoint << std::endl;
                    // Send the request to fetch movie details
                    auto detailsRes = client.Get(detailsEndpoint.c_str());
                    // Check if the request was successful
                    if (detailsRes && detailsRes->status == 200) {
                        json detailsResponse = json::parse(detailsRes->body);       // Parse the JSON response
                        genre = detailsResponse.value("Genre", "Unknown");          // Get movie genre
                        imdbRating = detailsResponse.value("imdbRating", "Unknown");// Get IMDb rating
                    } else {
                        std::cerr << "ERROR: Failed to fetch details for IMDb ID: " << imdbID << std::endl;
                    }
                }

                // Download the movie poster and save it to the cache directory
                std::string savePath = "cache/" + title + ".jpg";     // Build the save path
                if (!DownloadImageFromURL(imdbID, savePath, apiKey)) {// Download the poster
                    std::cerr << "ERROR: Failed to download poster for " << title << std::endl;
                }
                // Add the movie title to the vector
                movieTitles.push_back(title + " (" + year + ")" + " (" + genre + ")" + " (" + imdbRating + ")" + "(" + imdbID + ")");
            }
        } else {
            std::cerr << "ERROR: No 'Search' field in response. Full response: " << res->body << std::endl;
        }
        // Check if the response contains the 'Error' field
    } catch (const std::exception &e) {
        std::cerr << "ERROR: Failed to parse JSON response: " << e.what() << std::endl;
    }
    // Return the vector of movie titles
    return movieTitles;
}
