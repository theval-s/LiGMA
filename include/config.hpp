#pragma once

#include "steam_finder.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <filesystem>
#include <string>
#include <vector>

namespace LigmaCore {
namespace fs = std::filesystem;

class ConfigLoader {
  public:
    // probably I should make all members static and just save by path, yeah?...
    // sounds like a better design
    static void saveInstance(const fs::path &configPath,
                             const QJsonObject &config);
    static void saveInstance(const QString &configPath,
                             const QJsonObject &config);
    static QJsonObject loadInstance(const fs::path &);
    static fs::path getConfigPath();
    static QString getConfigPathQString();
    static void CheckCorrectness(const QJsonObject &config);
    static void CheckModListCorrectness(const QJsonArray &modList);
    static std::vector<std::string> getSavedInstanceNames();

  private:
    static void saveInstance(QFile &file, const QJsonObject &config);
};

/// Class for saving variables that can be changed by user after creating instance
/// This includes:
/// custom environment variables
///Proton version
/// Using SteamRuntime home isolation
class UserConfig {
    //maybe using this as a base class and make ProtonUserConfig?
    //also maybe I can add option of using mount instead of fuse-overlayfs, but using pkexec()?
private:
    ProtonVersion protonVersion = Hotfix;
    bool useHomeIsolation = false;
    std::vector<QString> environmentVariables = {};
    SteamRuntimeVersion steamRuntimeVersion = None;

  public:
    SteamRuntimeVersion getSteamRuntimeVersion() const {
        return steamRuntimeVersion;
    }
    void setSteamRuntimeVersion(SteamRuntimeVersion steam_runtime_version) {
        steamRuntimeVersion = steam_runtime_version;
    }
    UserConfig() = default;
    void setProtonVersion(const ProtonVersion proton_version) {
        protonVersion = proton_version;
    }
    void setUseHomeIsolation(const bool use_home_isolation) {
        useHomeIsolation = use_home_isolation;
    }
    void addEnvironmentVariable(const QString &variable) {
        environmentVariables.push_back(variable);
    }
    void modifyEnvironmentVariable(const int index, const QString &newValue) {
        if (index < 0 && index > environmentVariables.size()) { return; }
        if (index == environmentVariables.size()) {
            addEnvironmentVariable(newValue);
            return;
        }
        environmentVariables[index] = newValue;
    }

    [[nodiscard]] std::vector<QString> getEnvironmentVariables() const {
        return environmentVariables;
    }
    [[nodiscard]] ProtonVersion getProtonVersion() const { return protonVersion; }
    [[nodiscard]] bool getUseHomeIsolation() const { return useHomeIsolation; }

    QJsonObject toJson() const;
    void updateFromJson(const QJsonObject &json);
};

} // namespace LigmaCore

// use cbor or json?
//Proton usage is detected from plugin at runtime so saving it is not needed
//BUT edge case where plugin changes?
//probably should add checks inside a NativeInstanceFilesystem class if mod has type:prefix
/* JSON
 * {
 *  "instanceName": string,
 *  "pluginUUID" : string,
 *  "basePath": string,
 *  "gamePath": string,
 *  "mounted": bool,
 *  "modList"{
 *          {
 *          "name": string
 *          "path": string
 *          "enabled": bool
 *          "type": int
 *          },
 *          {
 *          "name": string
 *          ...
 *          },
 *      }
 *  }
 *
 *  optional values:
 *  "protonVersion" - if it's different than Hotfix
 *  "environmentVariables"{} - if there are any (including plugin listed ones)
 *  "steamRuntimeVersion" - 1, 2, 3 - corresponding to version
 *  "useHomeIsolation" - if it's true
 */
