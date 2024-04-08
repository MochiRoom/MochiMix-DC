#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Scan Files in Folder
void scanForFiles(std::vector<std::string> &files, std::string folder,
                  std::vector<std::string> exts = {});

// Load Music From File
bool loadMusic(std::vector<uint8_t> &data, const char *path);

// system() with va_args
int run(const char *text, ...);