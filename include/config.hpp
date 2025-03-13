#pragma once

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <filesystem>
#include <string>
#include <vector>

namespace LigmaCore {
namespace fs = std::filesystem;

class ConfigManager {
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
 */
