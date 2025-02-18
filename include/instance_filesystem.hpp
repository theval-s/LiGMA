#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include "ligma_plugin.hpp"
#include "plugin_handler.hpp"
#include "config.hpp"
#include <iostream>

namespace LigmaCore {
    namespace fs = std::filesystem;

    //I'm thinking of making it changeable during runtime but not right now
    //this way it has a bad scope though...
    constexpr std::string_view GAME_WORK_DIR = ".work_game";
    constexpr std::string_view MERGED_DIR = "game";
    constexpr std::string_view UPPER_DIR = "upper";
    constexpr std::string_view MOD_FILES_DIR = "mod_files";
    constexpr std::string_view MODS_WORK_DIR = ".work_mods";
    class InstanceFilesystem {


    private:
        fs::path m_basePath;
        fs::path m_gamePath;
        std::string m_instanceName;
        //std::string sanitizedName;
        std::string m_pluginUUID;
        LigmaPlugin *m_gamePlugin;

        bool gameMounted = false;
        //bool modMounted = false;
        std::vector<std::pair<std::string, fs::path> > m_modList;
        //modList[order].first = name
        //modList[order].second = path
        //using struct with more values might be a good idea
        //because bool for enabled mod is needed

        static std::string sanitizeForPath(const std::string &);

        fs::path m_configPath;

    public:
        /// Creates new instance, creating new directories in basePath and saving appropriate config
        /// @brief Constructor of a new instance
        /// @param instanceName Name of instance for saving in .json
        /// @param basePath Path to instance folder
        /// @param gamePath Path to the game
        /// @param gamePlugin Pointer to plugin associated with the game
        InstanceFilesystem(const std::string &instanceName, const fs::path &basePath,
                           const fs::path &gamePath, LigmaPlugin *gamePlugin);

        /// @brief Creates instance from saved config
        /// @param config Configuration stored in Json
        /// @param pathToConfig Path to the file that has saved config
        //or maybe just make it construct from path and getting json in initialization?
        InstanceFilesystem(const QJsonObject &config, const fs::path &pathToConfig);

        ~InstanceFilesystem() {
            std::cerr << "InstanceFilesystem deconstructed\n";
            if (gameMounted) unmount(m_basePath / MERGED_DIR);
            PluginHandler::getInstance().unloadPlugin(m_gamePlugin);
        }

        //delete copying
        InstanceFilesystem(const InstanceFilesystem &) = delete;
        InstanceFilesystem &operator=(const InstanceFilesystem &) = delete;

        InstanceFilesystem(InstanceFilesystem &&) = default;

        //InstanceFilesystem& operator=(const InstanceFilesystem&&) = default;

        //void saveInstance(const QJsonObject &config, const fs::path &);
        //QJsonObject loadInstance(const fs::path &);
        QJsonObject toJson() const;

        const std::string &getInstanceName() { return m_instanceName; }
        const std::vector<std::pair<std::string, fs::path> > &getModList() const { return m_modList; }
        bool isMounted() const { return gameMounted; }

        void mountGameFilesystem();

        /// @brief Adds mod into the instance
        /// Copies mod into its respective folder and adds it to modList, saves in config
        /// @param modPath Path to mod
        /// @param modName Mod name that will be saved in modList
        void addMod(const fs::path &modPath, const std::string &modName);

        /// @brief Returns string for use in overlayfs mounting
        /// Gets all mod dirs from modList and returns a string with format dir1:dir2:...:dirn
        std::string getLowerDirsString();

        /// @brief removes mod from modlist and removes its files
        void removeMod(const size_t &id);


        void unmount(const fs::path &dir);

        /// @brief Mounts overlayfs and launches game
        /// Mounts fuse-overlayfs if it's not mounted, and then opens the game
        void openGame();

        /// @brief Fully deletes instance
        void cleanState();

        void saveState() const;
    };
} //namespace LigmaCore
