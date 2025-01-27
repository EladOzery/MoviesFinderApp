#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

// Struct to represent a movie
struct Movie {
    std::string title;
    std::string year;
};

// Global variables
std::string searchQuery = "";            // The user's search query
std::vector<Movie> movies;              // List of movies (search results)
std::mutex moviesMutex;                 // Mutex to protect access to `movies`

// Simulates an API search by populating the `movies` vector
void searchMovies(const std::string& query) {
    // Simulating API delay
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Dummy search results
    std::vector<Movie> results = {
            {"The Matrix", "1999"},
            {"Inception", "2010"},
            {"Interstellar", "2014"}
    };

    // Update `movies` safely using the mutex
    std::lock_guard<std::mutex> lock(moviesMutex);
    movies = results;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Movie Search", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::thread searchThread; // Thread for background API search
    bool isSearching = false; // Flag to indicate whether a search is ongoing

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set ImGui window to cover the entire screen
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        // Begin ImGui window
        ImGui::Begin("Movie Manager App", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // Search bar
        static char buffer[256] = ""; // Buffer for user input
        ImGui::InputText("Search Movies", buffer, sizeof(buffer));

        // Search button
        if (ImGui::Button("Search")) {
            if (!isSearching) {
                searchQuery = buffer; // Save the user's query
                isSearching = true; // Mark as searching

                // Start a thread for the API search
                searchThread = std::thread([&]() {
                    searchMovies(searchQuery);
                    isSearching = false; // Mark search as done
                });
            }
        }

        // Show "Searching..." if the search is ongoing
        if (isSearching) {
            ImGui::Text("Searching...");
        }

        // Display the results in a table
        if (!movies.empty()) {
            if (ImGui::BeginTable("Movies Table", 2)) {
                ImGui::TableSetupColumn("Title"); // Column for movie title
                ImGui::TableSetupColumn("Year");  // Column for movie year
                ImGui::TableHeadersRow();

                std::lock_guard<std::mutex> lock(moviesMutex); // Lock for safe access
                for (const auto& movie : movies) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", movie.title.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", movie.year.c_str());
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup resources
    if (searchThread.joinable()) {
        searchThread.join();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
