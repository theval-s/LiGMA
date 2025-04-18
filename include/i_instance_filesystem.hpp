//
// Created by val on 3/10/25.
//

#pragma once

#include "config.hpp"
#include "ligma_plugin.hpp"

#include <QDir>
#include <QJsonObject>
#include <QString>
#include <memory>

namespace LigmaCore {

enum ModType { GameRoot, Prefix };
struct ModInfo {
    QString name;
    fs::path path;
    bool enabled;
    ModType type;
};

// I'm thinking of making it changeable during runtime but not right now
// this way it has a bad scope though...
constexpr std::string LIGMA_GAME_WORK_DIR = ".work_game";
constexpr std::string LIGMA_GAME_MERGED_DIR = "game";
constexpr std::string LIGMA_GAME_UPPER_DIR = "upper";
constexpr std::string LIGMA_MOD_FILES_DIR = "mod_files";
constexpr std::string LIGMA_PREFIX_WORK_DIR = ".work_prefix";
constexpr std::string LIGMA_PREFIX_MERGED_DIR = "prefix";
constexpr std::string LIGMA_PREFIX_MODS_DIR = "prefix";
constexpr std::string LIGMA_PREFIX_UPPER_DIR = "prefix_upper";

QString sanitizeForPath(const QString &);

class IInstanceFilesystem {
  public:
    virtual ~IInstanceFilesystem() = default;
    virtual void mountGameFilesystem() = 0;
    virtual void unmountGameFilesystem() = 0;

    /// @brief Adds mod into the instance
    /// Copies mod into its respective folder and adds it to modList, saves in
    /// config
    /// @param modPath Path to mod
    /// @param modName Mod name that will be saved in modList
    /// @param destPathString Destination where in game root mod should be placed if it's not properly structured by itself
    virtual void addMod(const fs::path &modPath, const QString &modName,
                        const QString &destPathString) = 0;

    /// @brief Deletes mod files and removes mod at id pos from modList
    /// @param id the index of mod in modList to remove
    virtual void removeMod(const int id) = 0;

    /// @brief Saves instance state into corresponding config file
    virtual void saveState() const = 0;

    /// @brief Mounts overlayfs and launches game
    /// Mounts fuse-overlayfs if it's not mounted, and then opens the game
    virtual void runGame() = 0;

    /// @brief Returns mod installation paths from plugin
    [[nodiscard]] virtual std::vector<QString> getModPaths() const = 0;

    virtual bool isMounted() const = 0;
    [[nodiscard]] virtual QJsonObject toJson() const = 0;
    [[nodiscard]] virtual QString getInstanceName() const = 0;
    [[nodiscard]] virtual std::vector<ModInfo> getModList() const = 0;
    virtual void swapMods(int first, int second) = 0;
    virtual void putToBack(int index) = 0;
    virtual void setEnabled(const int index, bool state) = 0;
    [[nodiscard]] virtual UserConfig &getUserConfigRef() = 0;
    virtual bool isUsingProton() const = 0;


};

class BaseInstanceFilesystem : public virtual IInstanceFilesystem {
  protected:
    fs::path m_basePath;
    fs::path m_gamePath;
    QString m_instanceName;
    UserConfig m_userConfig;
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> m_gamePlugin;
    bool m_mounted = false;
    std::vector<ModInfo> m_modList;

    //! @brief Path to config file for that specific instance, ending with .json
    QString m_configPath;

    static QString resolveMacros(const QString &);
    void copyMod(const QDir &modPath, const QDir &destPath);
    void copyMod(const fs::path &modPath, const fs::path &destPath);

    /// @brief Fully deletes instance
    //currently protected because I'm not actually allowing it in UI
    //but might use it for tests
    void cleanState();

  public:
    BaseInstanceFilesystem() = delete;
    BaseInstanceFilesystem(const BaseInstanceFilesystem &) = delete;
    BaseInstanceFilesystem &operator=(const BaseInstanceFilesystem &) = delete;
    ~BaseInstanceFilesystem() override = default;
    //BaseInstanceFilesystem(BaseInstanceFilesystem &&) = default;
    //BaseInstanceFilesystem &operator=(BaseInstanceFilesystem &&) = default;

    /// Creates new instance, creating new directories in basePath and saving
    /// appropriate config
    /// @brief Constructor of a new instance
    /// @param instanceName Name of instance for saving in .json
    /// @param basePath Path to instance folder
    /// @param gamePath Path to the game
    /// @param gamePlugin Pointer to plugin associated with the game
    BaseInstanceFilesystem(
        const QString &instanceName, const fs::path &basePath,
        fs::path gamePath,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin);

    /// @brief Creates instance from saved config
    /// @param config Configuration stored in Json
    /// @param pathToConfig Path to the file that has saved config
    /// @param gamePlugin Pointer to plugin associated with the game
    BaseInstanceFilesystem(
        const QJsonObject &config, const fs::path &pathToConfig,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin);

    void removeMod(const int id) override;
    void saveState() const override {
        ConfigLoader::saveInstance(m_configPath, this->toJson());
    }

    bool isMounted() const override { return m_mounted; }
    [[nodiscard]] QJsonObject toJson() const override;
    [[nodiscard]] QString getInstanceName() const override {
        return m_instanceName;
    }
    [[nodiscard]] std::vector<ModInfo> getModList() const override {
        return m_modList;
    }
    void swapMods(const int first,const int second) override {
        std::swap(m_modList.at(first), m_modList.at(second));
        if (m_mounted) unmountGameFilesystem();
    }
    void putToBack(const int index) override {
        std::rotate(m_modList.begin() + index, m_modList.begin() + index + 1, m_modList.end());
        if (m_mounted) unmountGameFilesystem();
    }
    void setEnabled(const int index, bool state) override {
        if (m_mounted) unmountGameFilesystem();
        m_modList.at(index).enabled = state;
    }
    [[nodiscard]] std::vector<QString> getModPaths() const override {
        std::vector<QString> paths = m_gamePlugin->modPaths();
        paths.insert(paths.begin(), "Game root");
        return paths;
    }
    [[nodiscard]] UserConfig &getUserConfigRef() override { return m_userConfig; }
    bool isUsingProton() const override { return m_gamePlugin->usesProton(); }

    /// @brief Returns string for use in overlayfs mounting
    /// Gets all mod dirs from modList with specified type and returns a string with format
    /// dir1:dir2:...:dirn. Need to add game path or prefix path after this.
    /// @param type ModType::GameRoot or ModType::Prefix
    [[nodiscard]] QString getModsLowerDirsString(const ModType &type) const;
};

} // namespace LigmaCore
