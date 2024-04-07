#pragma once

#include <cstdint>
#include <string>
#include <vector>

// system() with va_args
int run(const char *text, ...);

// Download Music
int ytdlp(std::string search, std::string &id);

// Load Music From File
bool loadMusic(std::vector<uint8_t> &data, const char *path);

// Scan Files in Folder
void scanForFiles(std::vector<std::string> &files, std::string folder,
                  std::vector<std::string> exts = {});