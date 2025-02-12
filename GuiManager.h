#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include "Movie.h"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;





/** Class to manage the GUI for the Movie Manager App using ImGui and GLFW libraries
for rendering and input handling.**/
class GuiManager {
public:
    GuiManager(GLFWwindow* window);
    ~GuiManager();

    void render(std::string &searchQuery, std::vector<Movie> &movies, std::mutex &moviesMutex,
                      std::atomic<bool> &isSearching, std::string queryCopy, const std::string& apiKey);

private:
    GLFWwindow* window;
};

#endif // GUI_MANAGER_H
