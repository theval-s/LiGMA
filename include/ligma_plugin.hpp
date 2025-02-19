#pragma once

#include <QtPlugin>
// #include <QString>
#include <filesystem>
#include <qobject.h>
#include <string>

// probably it's better to use std::string instead of QString despite everything
class LigmaPlugin {
  public:
    virtual ~LigmaPlugin() = default;

    virtual std::string pluginUUID() const = 0; // Plugin UUID

    virtual std::string gameName() const = 0; // Display name

    virtual int gameID() const = 0; // Steam ID of the game

    virtual std::string
    executeCommand() const = 0; // currently just for executable name,
    //^ should be restructurized for customizable args probably

    virtual std::string
    modPath() const = 0; // in case mods should be stored somewhere else
    // and not in the path of base game set this to return not null
    //(if first symbol is '/' it's treated as absolute path by std::filesystem

    virtual void initializeModState(std::filesystem::path modsDirPath)
        const = 0; // Actions to start out modding of the game
    // like installing modding API, specific mods, setting up file structure in
    // Ligma mod folder path to Ligma _mods folder that gets mounted upon game
    // folder is provided.
};
#define LigmaPluginInterface_iid "com.ligma.PluginInterface/1.0"
Q_DECLARE_INTERFACE(LigmaPlugin, LigmaPluginInterface_iid)
