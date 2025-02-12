#include "GuiManager.h"
#include "ImageLoader.h"
#include <algorithm>
#include <iostream>
#include <regex>


ImFont *iconFontRegular = nullptr;// Font for regular icons
ImFont *iconFontSolid = nullptr;  // Font for solid icons


/**
 * @brief Constructor for the GuiManager class.
 *
 * This constructor initializes the ImGui library and sets up the fonts and styles for the GUI.
 *
 * @param window A pointer to the GLFWwindow object.
 *
 * The constructor performs the following tasks:
 * - Checks the ImGui version.
 * - Creates an ImGui context.
 * - Adds a default font from a TTF file.
 * - Adds FontAwesome icons to the font atlas.
 * - Sets the ImGui style to light.
 * - Initializes ImGui for GLFW and OpenGL.
 */
GuiManager::GuiManager(GLFWwindow *window) : window(window) {
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImFont *defaultFont = io.Fonts->AddFontFromFileTTF("../external/imgui/misc/fonts/DroidSans.ttf", 30.0f);
    static const ImWchar icons_ranges[] = {0xf000, 0xf3ff, 0};// Unicode range for FontAwesome
    ImFontConfig config;
    config.MergeMode = true;// Merge with previous font
    config.PixelSnapH = true;
    iconFontRegular = io.Fonts->AddFontFromFileTTF("../external/imgui/misc/fonts/fa-regular-400.ttf", 18.0f, &config, icons_ranges);
    iconFontSolid = io.Fonts->AddFontFromFileTTF("../external/imgui/misc/fonts/fa-Solid-900.ttf", 18.0f, &config, icons_ranges);
    io.Fonts->Build();

    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

GuiManager::~GuiManager() {
    std::string cachePath = "cache/";

    try {
        if (fs::exists(cachePath) && fs::is_directory(cachePath)) {
            for (const auto &entry: fs::directory_iterator(cachePath)) {
                fs::remove(entry.path());
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Error deleting cache files: " << e.what() << std::endl;
    }
    // Cleanup ImGui

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
std::vector<Movie> favoriteMovies;// List of favorite movies
std::mutex fileMutex;             // Protects file access

// Function to save favorite movies to a file
void saveFavoritesToFile() {
    std::lock_guard<std::mutex> lock(fileMutex);
    std::ofstream outFile("favorites.txt");

    if (!outFile) {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        return;
    }

    for (const auto &movie: favoriteMovies) {
        outFile << movie.title << "," << movie.year << std::endl;
    }

    outFile.close();
}


// Function to load favorite movies from file
void loadFavoritesFromFile() {
    std::lock_guard<std::mutex> lock(fileMutex);
    std::ifstream inFile("favorites.txt");

    if (!inFile) {
        std::cerr << "Warning: No existing favorites file found." << std::endl;
        return;
    }

    favoriteMovies.clear();// Clear the list before loading
    std::string line;
    while (std::getline(inFile, line)) {
        size_t commaPos = line.find(",");
        if (commaPos != std::string::npos) {
            std::string title = line.substr(0, commaPos);
            std::string year = line.substr(commaPos + 1);
            favoriteMovies.push_back({title, year});
        }
    }

    inFile.close();
}
void addMovieToFavorites(const Movie &movie) {
    for (const auto &fav: favoriteMovies) {
        if (fav.title == movie.title)
            return;
    }

    favoriteMovies.push_back(movie);
    saveFavoritesToFile();
}

void removeMovieFromFavorites(const Movie &movieToRemove) {
    // Find and remove the movie that matches both title and year
    auto it = std::remove_if(favoriteMovies.begin(), favoriteMovies.end(),
                             [&movieToRemove](const Movie &movie) {
                                 return (movie.title == movieToRemove.title && movie.year == movieToRemove.year);
                             });

    if (it != favoriteMovies.end()) {
        favoriteMovies.erase(it, favoriteMovies.end());// Remove matching elements
        saveFavoritesToFile();                         // Save updated list to file
        std::cout << "Movie removed from favorites: " << movieToRemove.title << " (" << movieToRemove.year << ")" << std::endl;
    } else {
        std::cout << "Movie not found in favorites: " << movieToRemove.title << " (" << movieToRemove.year << ")" << std::endl;
    }
}


enum SortColumn { None,
                  Title,
                  Year,
                  Rating };
SortColumn currentSortColumn = None;
bool sortAscending = true;// Track ascending/descending state

// Sorting function
void sortMovies(std::vector<Movie> &movies) {
    if (currentSortColumn == None) return;

    std::sort(movies.begin(), movies.end(), [](const Movie &a, const Movie &b) {
        if (currentSortColumn == Title) return (sortAscending) ? (a.title < b.title) : (a.title > b.title);
        if (currentSortColumn == Year) return (sortAscending) ? (a.year < b.year) : (a.year > b.year);
        if (currentSortColumn == Rating) return (sortAscending) ? (a.imdbRating > b.imdbRating) : (a.imdbRating < b.imdbRating);
        return false;
    });
}

/**
 * @brief Renders the GUI for the Movie Manager App.
 *
 * This function handles the rendering of the main GUI window, including the search bar, search button,
 * search results table, and favorite movies table. It also manages the search query input and the display
 * of search results and favorite movies.
 *
 * @param searchQuery Reference to the search query string.
 * @param movies Reference to the vector of movies to display.
 * @param moviesMutex Mutex to protect access to the movies vector.
 * @param isSearching Atomic boolean indicating if a search is in progress.
 * @param queryCopy Copy of the search query string.
 * @param apiKey API key for downloading movie posters.
 */
void GuiManager::render(std::string &searchQuery, std::vector<Movie> &movies, std::mutex &moviesMutex,
                        std::atomic<bool> &isSearching,
                        std::string queryCopy, const std::string &apiKey) {
    // Load favorite movies from file
    loadFavoritesFromFile();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float centerX = windowSize.x * 0.5f;
    // Main Window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(windowSize);
    // Main window with no title bar, no resize, no move
    ImGui::Begin("Movie Manager App", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Centered Title
    ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize("Movie Search").x * 0.5f);
    ImGui::Text("Movie Search");

    // Centered Search Bar
    float searchBarWidth = 300.0f;
    ImGui::SetCursorPosX(centerX - searchBarWidth * 0.5f);
    static char buffer[128] = "";
    ImGui::PushItemWidth(searchBarWidth);
    ImGui::InputText("##search", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll);
    ImGui::PopItemWidth();
    // flag to check if the search query is empty
    bool flag = false;

    // Centered Search Button
    ImGui::SetCursorPosX(centerX - 40);
    if (ImGui::Button("Search", ImVec2(80, 30))) {
        if (!isSearching) {
            searchQuery = buffer;
            isSearching = true;
        }
    }

    bool searching = isSearching.load(std::memory_order_relaxed);
    if (!searching) flag = true;

    // Display search query in center
    if (searching) {
        ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize("Searching...").x * 0.5f);
        ImGui::Text("Searching...");
    }

    // Display search results with sorting
    if (!movies.empty()) {
        ImGui::SetCursorPosX(5);                                                      // Set cursor position to left
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));         // Set spacing between items
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));          // Set padding for frames
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));       // Set header color
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));       // Set border color
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));       // Set border color
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));     // Set window background color
        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));// Set table row background color
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));       // Set button color
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));// Set button hover color
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.8f, 1.0f)); // Set button active color

        // Display table with search results
        if (ImGui::BeginTable("Movies Table", 6, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter))// Table with 6 columns and sortable
        {
            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch, 600.0f, Title);// Default sort by title
            ImGui::TableSetupColumn("Year", ImGuiTableColumnFlags_DefaultSort, 100.0f, Year);                                       // Default sort by year
            ImGui::TableSetupColumn("Genre", ImGuiTableColumnFlags_WidthStretch, 300.0f);                                           // Stretch to fill available space
            ImGui::TableSetupColumn("IMDB Rating", ImGuiTableColumnFlags_DefaultSort, 100.0f, Rating);                              // Default sort by rating
            ImGui::TableSetupColumn("Poster", ImGuiTableColumnFlags_WidthFixed, 100.0f);                                            // Fixed width for poster
            ImGui::TableSetupColumn("");                                                                                            // Empty column for like button
            ImGui::TableHeadersRow();                                                                                               // Headers Row

            // Sorting Logic
            if (ImGuiTableSortSpecs *sortSpecs = ImGui::TableGetSortSpecs()) {
                if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                    const ImGuiTableColumnSortSpecs *sortSpec = &sortSpecs->Specs[0];
                    // Set current sort column and direction
                    currentSortColumn = static_cast<SortColumn>(sortSpec->ColumnUserID);
                    sortAscending = (sortSpec->SortDirection == ImGuiSortDirection_Ascending);
                    // Sort the movies vector
                    sortMovies(movies);
                    sortSpecs->SpecsDirty = false;
                }
            }

            std::lock_guard<std::mutex> lock(moviesMutex);// Lock the movies vector while reading
            // Display each movie in a row of the table with title, year, genre, rating, poster, and like button columns
            for (const auto &movie: movies) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", movie.title.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", movie.year.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", movie.genre.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", movie.imdbRating.c_str());
                ImGui::TableSetColumnIndex(4);
                //Path of the poster image
                std::string posterPath = "cache/" + movie.title + ".jpg";
                // Download the poster image if it does not exist
                if (!std::ifstream(posterPath)) {
                    std::cout << "Downloading poster for: " << movie.title << std::endl;
                    DownloadImageFromURL(movie.posterUrl, posterPath, apiKey);
                }

                //loading poster from local file
                GLuint texture = LoadTextureFromFile(posterPath);
                if (texture) {
                    ImGui::Image((ImTextureID) (uintptr_t) texture, ImVec2(50, 75));// Display the poster image
                } else {
                    ImGui::Text("No Image");// Display text if image not found
                }

                ImGui::TableSetColumnIndex(5);
                if (iconFontRegular) ImGui::PushFont(iconFontRegular);// Set font for like button

                std::string buttonID = "##Like" + movie.title + movie.year;// Button ID for like button

                ImVec2 buttonSize(30, 30);// Size of the like button
                // Invisible button for like button
                if (ImGui::InvisibleButton(buttonID.c_str(), buttonSize)) {
                    addMovieToFavorites(movie);// Add movie to favorites if button is clicked
                }

                ImVec2 buttonMin = ImGui::GetItemRectMin();                                                         // Get minimum position of the button
                ImVec2 iconPos = ImVec2(buttonMin.x + (buttonSize.x / 2) - 8, buttonMin.y + (buttonSize.y / 2) - 8);// Position of the icon
                ImGui::SetCursorScreenPos(iconPos);                                                                 // Set cursor position to icon position
                ImGui::Text("\xef\x80\x85");                                                                        // Display the like icon
                // Pop font after displaying the icon
                if (iconFontRegular) ImGui::PopFont();
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleColor(8);
        ImGui::PopStyleVar(2);
    } else {
        // Display message if no results found and search query is not empty
        if (flag == true && queryCopy[0] != '\0') {
            ImGui::NewLine();
            ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize("No Results Found").x * 0.5f);
            ImGui::Text("No Results Found");
        }
    }

    ImGui::NewLine();
    ImGui::NewLine();

    // Display favorite movies
    ImGui::Separator();                                                                                                                                                                                                             // Separator line
    ImGui::Text("Favorite Movies:");                                                                                                                                                                                                // Title for favorite movies
    if (ImGui::BeginTable("Favorites Table", 3, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter))// Table with 3 columns
    {
        ImGui::TableSetupColumn("Title");
        ImGui::TableSetupColumn("Year");
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();
        // Display each favorite movie in a row of the table with title, year, and dislike button columns
        for (const auto &movie: favoriteMovies) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", movie.title.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", movie.year.c_str());

            ImGui::TableSetColumnIndex(2);

            if (iconFontSolid) ImGui::PushFont(iconFontSolid);// Set font for dislike button
            // Button ID for dislike button
            std::string buttonID = "##Dislike" + movie.title + movie.year;

            ImVec2 buttonSize(30, 30);
            // Invisible button for dislike button
            if (ImGui::InvisibleButton(buttonID.c_str(), buttonSize)) {
                removeMovieFromFavorites(movie);
            }

            ImVec2 buttonMin = ImGui::GetItemRectMin();
            ImVec2 iconPos = ImVec2(buttonMin.x + (buttonSize.x / 2) - 8, buttonMin.y + (buttonSize.y / 2) - 8);
            ImGui::SetCursorScreenPos(iconPos);
            ImGui::Text("\xef\x80\x85");

            if (iconFontSolid) ImGui::PopFont();
        }
        // End table for favorite movies
        ImGui::EndTable();
    }

    // End main window
    ImGui::End();

    // Render GUI and swap buffers
    ImGui::Render();
    int display_w, display_h;                              // Display width and height
    glfwGetFramebufferSize(window, &display_w, &display_h);// Get framebuffer size
    glViewport(0, 0, display_w, display_h);                // Set viewport
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);                  // Clear color
    glClear(GL_COLOR_BUFFER_BIT);                          // Clear color buffer
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());// Render draw data

    glfwSwapBuffers(window);// Swap buffers
}
