#pragma once

#include <QString>
#include <QtPlugin>
#include <filesystem>
#include <qobject.h>
#include <vector>

// probably it's better to use std::string instead of QString despite everything
class LigmaPlugin {
  public:
    virtual ~LigmaPlugin() = default;

    /// @brief Returns plugin's UUID
    virtual QString pluginUUID() const = 0; // Plugin UUID

    /// @brief Returns name of the game that the plugin is made for
    virtual QString gameName() const = 0;

    /// @brief Returns Steam ID of the game
    virtual int gameID() const = 0;

    /// @brief Game executable file name
    virtual QString executableName() const = 0;

    /// @brief Return true if game should use Proton
    virtual bool usesProton() const = 0;

    /// @brief Returns environment variables that need to be set for game
    /// launching Should not add STEAM_COMPAT_DATA_PATH,
    /// STEAM_COMPAT_CLIENT_INSTALL_DIR or SteamGameId/SteamAppId, as those will
    /// be ignored
    virtual std::vector<QString> environmentVariables() const = 0;

    /// @brief Returns paths where mods should  be stored other than game root
    /// Should return directories where mods should be put, except game root.
    /// Directories starting with '/' are treated as absolute paths,
    /// Other - as subdirectories of game root
    /// Supports macros at the beginning, starting with $:
    /// $PREFIX - substitutes to resulting overlayed Proton Prefix (pfx dir)
    /// Example: $PREFIX/drive_c/users/steamuser/AppData/Roaming/Balatro/
    virtual std::vector<QString> modPaths() const = 0;

    // to be removed I guess
    virtual void
    initializeModState(std::filesystem::path modsDirPath) const = 0;

    /// @brief Initial message to show when new instance is created
    virtual QString initialMessage() const = 0;
};
#define LigmaPluginInterface_iid "com.ligma.PluginInterface/1.0"
Q_DECLARE_INTERFACE(LigmaPlugin, LigmaPluginInterface_iid)
