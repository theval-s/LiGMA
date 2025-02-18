#pragma once

#include <QJsonObject>
#include <filesystem>
#include <string>
#include <vector>

namespace LigmaCore {
namespace fs = std::filesystem;

class ConfigManager {
public:
    //probably I should make all members static and just save by path, yeah?...
    //sounds like a better design
    static void saveInstance(const fs::path &configPath, const QJsonObject &config);
    static QJsonObject loadInstance(const fs::path &);
    static fs::path getConfigPath();
    static bool isCorrect(QJsonObject config);
    static std::vector<std::string> getSavedInstanceNames();
private:

};
} // namespace LigmaCore

// use cbor or json?


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
 *          },
 *          {
 *          "name": string
 *          "path": string
 *          },
 *      }
 *  }
 */
