#include "GuiManager.h"
#include "OMDbApi.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

// Global variables
std::string searchQuery = "";        // Search query string
std::vector<Movie> movies;           // Vector to store movie data
std::mutex moviesMutex;              // Mutex to protect access to movie data
std::atomic<bool> isSearching(false);// Atomic flag to prevent race conditions

// API Key (Replace with your real OMDb API key)
const std::string API_KEY = "133d7f7e";


/**
 * @brief Searches for movies using the OMDb API and updates the provided movie list.
 *
 * This function performs a search for movies based on the given query string using the OMDb API.
 * It updates the provided vector of movies with the search results and ensures thread safety
 * using a mutex. The function also uses an atomic boolean to indicate the search status.
 *
 * @param api Reference to the OMDbApi object used to perform the search.
 * @param query The search query string used to find movies.
 * @param movies Reference to a vector of Movie objects to be updated with the search results.
 * @param moviesMutex Reference to a mutex used to ensure thread-safe access to the movies vector.
 * @param isSearching Reference to an atomic boolean used to indicate whether a search is in progress.
 */
void searchMovies(OMDbApi &api, std::string query, std::vector<Movie> &movies, std::mutex &moviesMutex, std::atomic<bool> &isSearching) {
    // Mark search as active to prevent concurrent searches from main thread
    isSearching.store(true, std::memory_order_relaxed);
    // Perform movie search using the OMDb API
    std::vector<std::string> movieResults = api.searchMovies(query);
    // Check if any movies were found
    if (movieResults.empty()) {
        std::cerr << " No movies found for query: " << query << std::endl;
    } else {
        std::cout << " Received " << movieResults.size() << " movies from API" << std::endl;
    }
    // Parse movie results and update the movie list
    std::vector<Movie> results;
    // Regular expression pattern to match movie data
    std::regex moviePattern(R"(^(.+?)\s*\((\d{4})\)\s*\((.*?)\)\s*\(([\d.]+)\)\s*\((\w+)\))");
    // Regular expression match object
    std::smatch match;
    // Iterate over each movie string and parse the data
    for (const auto &movie: movieResults) {
        // Remove newline and carriage return characters from the movie string
        std::string cleanMovieStr = movie;
        cleanMovieStr.erase(std::remove(cleanMovieStr.begin(), cleanMovieStr.end(), '\n'), cleanMovieStr.end());
        cleanMovieStr.erase(std::remove(cleanMovieStr.begin(), cleanMovieStr.end(), '\r'), cleanMovieStr.end());

        // Remove leading and trailing whitespace from the movie string
//        cleanMovieStr = std::regex_replace(cleanMovieStr, std::regex("^\\s+|\\s+$"), "");
        // Check if the movie string matches the expected pattern
        if (std::regex_match(cleanMovieStr, match, moviePattern)) {
            std::string title = match[1].str();     // Group 1: Movie Title
            std::string year = match[2].str();      // Group 2: Year
            std::string genre = match[3].str();     // Group 3: Genre
            std::string imdbRating = match[4].str();// Group 4: IMDb Rating
            std::string posterUrl = match[5].str(); // Group 5: Poster URL
            // Add the movie data to the results vector
            results.push_back({title, year, genre, imdbRating, posterUrl});
        } else {
            // Print an error message if the movie string does not match the expected pattern
            std::cerr << "ERROR: Failed to parse movie string: " << movie << std::endl;
        }
    }

    // Lock and update movie data
    {
        std::lock_guard<std::mutex> lock(moviesMutex);
        movies = results;
    }

    // Mark search as complete
    isSearching.store(false, std::memory_order_relaxed);
}

/**
 * @brief Entry point of the application.
 *
 * This function initializes the GLFW library, creates a window, and sets up the main loop
 * for the application. It also initializes the GUI manager and the OMDb API, and handles
 * the search functionality in a separate thread.
 *
 * @return int Returns 0 on successful execution, or -1 if initialization fails.
 *
 * The main loop performs the following tasks:
 * - Polls for window events.
 * - Checks if a new search query is available and no search is currently running.
 * - If conditions are met, starts a new search thread to fetch movie data from the OMDb API.
 * - Renders the GUI with the current state of the application.
 * - Sleeps for a short duration to prevent excessive CPU usage.
 *
 * Before exiting, the function ensures that any running search thread is joined, and cleans up
 * the GLFW resources.
 */
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // Create a window with a size of 1280x720 and a title
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Movie Search", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    // Make the window's context current
    glfwMakeContextCurrent(window);
    // Enable vertical sync (V-Sync)
    glfwSwapInterval(1);

    // Initialize GUI Manager
    GuiManager gui(window);

    // Initialize OMDb API
    OMDbApi api(API_KEY);
    // Search thread and query variables
    std::thread searchThread;
    // Copy of the search query for display purposes in the GUI (to prevent flickering)
    std::string queryCopy = "\0";
    // Main loop for the application
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();// Poll for window events
        // Check if searchQuery is updated and no search is running
        if (!searchQuery.empty() && isSearching.load(std::memory_order_relaxed)) {
            // Join the previous search thread if it is still running
            if (searchThread.joinable()) {
                searchThread.join();
            }
            queryCopy = searchQuery;                           // Copy before clearing
            isSearching.store(true, std::memory_order_relaxed);// Mark search as active
            searchThread = std::thread([&]() {
                searchMovies(api, queryCopy, movies, moviesMutex, isSearching);
            });

            if (!searchThread.joinable()) {
            } else {
            }

            searchQuery.clear();// Reset after starting the thread
        }


        // Render GUI
        gui.render(searchQuery, movies, moviesMutex, isSearching, queryCopy, API_KEY);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));// Prevent excessive CPU usage
    }

    // Cleanup before exit
    if (searchThread.joinable()) {
        searchThread.join();
    }
    // Destroy window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}