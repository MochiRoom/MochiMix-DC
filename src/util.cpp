#include <util.hpp>

#include <mpg123.h>
#include <out123.h>

#include <dirent.h>

#include <cstdarg>
#include <cstring>

void scanForFiles(std::vector<std::string> &files, std::string folder,
                  std::vector<std::string> exts) {
    files.clear();

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder.c_str())) != NULL) {

        // check all files
        while ((ent = readdir(dir)) != NULL) {

            if (ent->d_type == DT_REG) {
                const char *ext = strrchr(ent->d_name, '.');

                // Check all extensions
                if (exts.size() > 0) {
                    for (auto e : exts)
                        if (!strcmp(ext, ("." + e).c_str()))
                            files.push_back(ent->d_name);
                } else
                    files.push_back(ent->d_name);
            }
        }

        closedir(dir);
    }
}

bool loadMusic(std::vector<uint8_t> &data, const char *path) {
    data.clear();

    mpg123_init();

    int err = 0;
    unsigned char *buffer;
    size_t buffer_size, done;
    int channels, encoding;
    long rate;

    /* Note it is important to force the frequency to 48000 for
     * Discord compatibility */
    mpg123_handle *mh = mpg123_new(NULL, &err);
    mpg123_param(mh, MPG123_FORCE_RATE, 48000, 48000.0);

    /* Decode entire file into a vector. You could do this on
     * the fly, but if you do that you may get timing issues if
     * your CPU is busy at the time and you are streaming to a
     * lot of channels/guilds.
     */
    buffer_size = mpg123_outblock(mh);
    buffer = new unsigned char[buffer_size];

    /* Note: In a real world bot, this should have some error
     * logging */
    mpg123_open(mh, path);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    unsigned int counter = 0;
    for (int totalBytes = 0;
         mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK;) {
        for (auto i = 0; i < buffer_size; i++) {
            data.push_back(buffer[i]);
        }
        counter += buffer_size;
        totalBytes += done;
    }

    delete buffer;
    mpg123_close(mh);
    mpg123_delete(mh);

    // TODO: cut off silence from beginning & end

    return true;
}

int run(const char *text, ...) {
    char *command;

    va_list args;
    va_start(args, text);
    vasprintf(&command, text, args);
    va_end(args);

    return system(command);
}