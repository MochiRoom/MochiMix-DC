#include <cstdlib>
#include <cstring>
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/presence.h>
#include <string>

#include <dpp/dpp.h>

#include <util.hpp>

int main(int argc, char const *argv[]) {
    // Set Random Seed
    srand(time(0));

    // Attempt to update yt-dlp
    system("../yt-dlp -U");

    // Start Discord Bot
    dpp::cluster bot(getenv("TOKEN_MOCHIMIX"),
                     dpp::i_default_intents | dpp::i_message_content);

    bot.on_log(dpp::utility::cout_logger());

    // Slash Command Callback
    bot.on_slashcommand([&](const dpp::slashcommand_t &event) {
        const char *command[] = {"help", "join",  "play",   "skip",
                                 "stop", "pause", "unpause"};
        const size_t commands = sizeof(command) / sizeof(*command);

        // Get Command ID
        size_t commandID = 0;
        for (size_t i = 0; i < commands; i++) {
            if (!strcmp(event.command.get_command_name().c_str(), command[i])) {
                commandID = i;
                break;
            }
        }

        const auto &guild_id = event.command.guild_id;
        dpp::voiceconn *v = event.from->get_voice(guild_id);

        // Attempt to Connect
        auto join = [&] {
            if (v)
                return;

            if (dpp::guild *g = dpp::find_guild(guild_id)) {
                if (!g->connect_member_voice(event.command.usr.id))
                    event.reply("You don't seem to be on a voice channel! :(");
            } else
                event.reply("You are not in a server!");
        };

        // Process Command
        switch (commandID) {
        case 0: { // help
            dpp::message msg;
            msg.set_content(
                "I am MochiMix-DC, the Discord client for MochiMix!");
            msg.set_flags(dpp::m_ephemeral);
            event.reply(msg);
        } break;

        case 1: { // join
            join();

            const char *response[] = {"Joined!", "OK", "Done!", "As you say...",
                                      "Certainly!"};
            const size_t responses = sizeof(response) / sizeof(*response);

            event.reply(response[rand() % responses]);
        } break;

        case 2: { // play
            if (!v)
                join();

            std::string name =
                std::get<std::string>(event.get_parameter("name"));

            // Download Music
            event.reply("Downloading...");
            run("rm -r audio/music.mp3");
            run("../yt-dlp -x --audio-format %s --output '%s' ytsearch:'%s'",
                "mp3", "audio/music", name.c_str());

            std::vector<uint8_t> pcmdata;
            loadMusic(pcmdata, "audio/music.mp3");

            event.edit_original_response("'" + name +
                                         "' is added to the queue");

            if (v && v->voiceclient && v->voiceclient->is_ready()) {
                v->voiceclient->insert_marker(name);
                v->voiceclient->send_audio_raw((uint16_t *)pcmdata.data(),
                                               pcmdata.size());
                v->voiceclient->insert_marker("{end}");
            }
        } break;

        case 3: { // skip
            if (!v)
                join();

            if (v && v->voiceclient && v->voiceclient->is_ready())
                v->voiceclient->skip_to_next_marker();
        } break;

        case 4: { // stop
            if (!v)
                join();

            if (v && v->voiceclient && v->voiceclient->is_ready())
                v->voiceclient->stop_audio();
        } break;

        case 5: { // pause
                  // if (!v)
                  //     join();

            // uint id = 0, count = 0;
            // if (sscanf(data, "%i", &id) < 1)
            //     id = !v->voiceclient->is_paused();

            // if (v && v->voiceclient && v->voiceclient->is_ready())
            v->voiceclient->pause_audio(true);
        } break;

        case 6: { // unpause
            v->voiceclient->pause_audio(false);
        } break;
        }
    });

    // Register Commands
    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_bulk_command_delete();

            bot.global_command_create({"help", "Show help menu", bot.me.id});

            bot.global_command_create({"join", "Join Voice Chat", bot.me.id});

            dpp::slashcommand play("play", "Play Music", bot.me.id);
            play.add_option(dpp::command_option(dpp::co_string, "name",
                                                "Name of the Music", true));
            bot.global_command_create(play);

            bot.global_command_create({"skip", "Skip Music", bot.me.id});
            bot.global_command_create({"stop", "Stop Music", bot.me.id});

            bot.global_command_create({"pause", "Pause Music", bot.me.id});
            bot.global_command_create({"unpause", "Unpause Music", bot.me.id});

            bot.set_presence(
                dpp::presence(dpp::ps_idle, dpp::at_listening, "nothing"));
        }
    });

    bot.on_voice_track_marker([&bot](const dpp::voice_track_marker_t &event) {
        if (event.track_meta == "{end}")
            bot.set_presence(
                dpp::presence(dpp::ps_idle, dpp::at_listening, "nothing"));
        else
            bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_listening,
                                           event.track_meta));
    });

    bot.start(dpp::st_wait);

    return 0;
}

// Database fun

// #include <DataBase.hpp>

// Init DataBases
// DataBase tracks("tracks.db"); // Database of every downloaded music
// tracks.run("CREATE TABLE IF NOT EXISTS music ("
//            "id TEXT PRIMARY KEY,"   // // Youtube ID (also file name)
//            "creator TEXT NOT NULL," // User who added it
//            "title TEXT NOT NULL"    // Title
//            ");");

// DataBase playlists("playlists.db"); // Database of playlists
// playlists.run("CREATE TABLE IF NOT EXISTS playlists ("
//               "id INTEGER PRIMARY KEY," // Playlist ID
//               "owner TEXT NOT NULL,"    // User who owns playlist
//               "name TEXT NOT NULL,"     // Name of the playlist
//               ");");

// Get Music
// if (!strcmp(cmd, ".get")) {
//     // Download Music

//     std::string id;
//     int err = ytdlp(data, id);

//     if (!err)
//         err = !tracks.run("INSERT INTO music (id, creator, title)"
//                           "VALUES ('%s', '%s', '%s');",
//                           author.username.c_str(), "todo",
//                           id.c_str());

//     if (err)
//         event.reply("Sorry, i couldn't get this one!");
//     else
//         event.reply("Done!");
// }

// // List Musics
// if (!strcmp(cmd, ".list")) {
//     scanForFiles(files);

//     if (!strcmp(data, "number"))
//         event.reply("There are " + std::to_string(files.size()) +
//                     " Audio Files Available");
//     else {
//         std::string list;

//         for (size_t f = 0; f < files.size(); f++)
//             list += std::to_string(f) + ": " + files[f] + "\n";

//         event.reply("Available Audio Files:\n" + list);
//     }
// }