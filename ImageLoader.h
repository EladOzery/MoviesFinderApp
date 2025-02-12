#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <GL/gl.h>

bool DownloadImageFromURL(const std::string& imdbID, const std::string& savePath, const std::string& apiKey);
GLuint LoadTextureFromFile(const std::string& filename);

#endif // IMAGE_LOADER_H
