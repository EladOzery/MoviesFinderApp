#include "ImageLoader.h"
// Define STB_IMAGE_IMPLEMENTATION before including stb_image.h to include implementation in this file
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"// Include stb_image.h for image loading
#include <GL/gl.h>             // Include OpenGL header
#include <direct.h>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * @brief Loads a texture from a file and creates an OpenGL texture object.
 *
 * This function loads an image from the specified file using the stb_image library,
 * creates an OpenGL texture object, and sets the texture parameters for minification
 * and magnification filters to GL_LINEAR.
 *
 * @param filename The path to the image file to be loaded.
 * @return The OpenGL texture ID of the loaded texture. Returns 0 if the image fails to load.
 */
GLuint LoadTextureFromFile(const std::string &filename) {
    int width, height, channels;// Variables to store image dimensions and number of channels
    // Force 4 channels for RGBA image data
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    // Check if image data is loaded successfully
    if (!data) {
        std::cerr << "ERROR: Failed to load image " << filename << std::endl;
        return 0;
    }
    GLuint texture;// Texture ID to return
    glGenTextures(1, &texture);
    // Bind texture to target GL_TEXTURE_2D
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set texture parameters for minification and magnification filters
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// Set minification filter to GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// Set magnification filter to GL_LINEAR

    stbi_image_free(data);// Free image data after loading to OpenGL texture
    return texture;       // Return OpenGL texture ID
}


/**
 * @brief Downloads an image from a URL and saves it to a specified path.
 *
 * This function downloads a poster image from the OMDb API using the provided IMDb ID and API key.
 * The image is saved to the specified file path. If the cache directory does not exist, it is created.
 *
 * @param imdbID The IMDb ID of the movie for which the poster is to be downloaded.
 * @param savePath The file path where the downloaded image will be saved.
 * @param apiKey The API key for accessing the OMDb API.
 * @return true if the image was downloaded and saved successfully, false otherwise.
 */
bool DownloadImageFromURL(const std::string &imdbID, const std::string &savePath, const std::string &apiKey) {
    std::cout << "Downloading poster for IMDb ID: " << imdbID << " -> " << savePath << std::endl;

    struct stat info;
    // Check if cache directory exists, create if not found
    if (stat("cache", &info) != 0) {
        std::cout << "Cache directory not found, creating..." << std::endl;
        _mkdir("cache");
    }
    // Create a client object for making HTTP requests
    httplib::Client cli("img.omdbapi.com");
    // Construct the URL for downloading the image using the IMDb ID and API key
    std::string imageUrl = "/?apikey=" + apiKey + "&i=" + imdbID;

    auto res = cli.Get(imageUrl.c_str());
    // Check if response was successful
    if (!res) {
        std::cerr << "ERROR: No response from server." << std::endl;
        return false;
    }


    if (res->status != 200) {
        std::cerr << "ERROR: Server returned status " << res->status << std::endl;
        return false;
    }
    // Open the file for writing in binary mode
    std::ofstream file(savePath, std::ios::binary);
    if (!file) {
        std::cerr << "ERROR: Could not open file for writing: " << savePath << std::endl;
        return false;
    }
    // Write the downloaded image data to the file
    file.write(res->body.c_str(), res->body.size());
    // Close the file after writing
    file.close();

    return true;
}
