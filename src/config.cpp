#include "config.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <format>
#include <iostream>

namespace LigmaCore {
void ConfigLoader::saveInstance(const fs::path &configPath,
                                 const QJsonObject &config) {
    QFile file(configPath);
    saveInstance(file, config);
}
void ConfigLoader::saveInstance(const QString &configPath,
                                 const QJsonObject &config) {
    QFile file(configPath);
    saveInstance(file, config);
}
void ConfigLoader::saveInstance(QFile &file, const QJsonObject &config) {
    // TODO: proper error handling (check for .json etc etc

    //truncates by default
    if (!file.open(QIODevice::WriteOnly)) {
        QFileInfo file_info(file);
        std::cerr << std::format("Error creating/modifying file at {}\n",
                                 file_info.filePath().toStdString());
        throw(std::ios_base::failure(std::format(
            // most likely dir not existing
            "Error creating/modifying file at {}\n",
            file_info.filePath().toStdString())));
    }
    file.write(QJsonDocument(config).toJson());
}
QJsonObject UserConfig::toJson() const {
    QJsonObject config;
    if (useHomeIsolation) {
        config["useHomeIsolation"] = useHomeIsolation;
    }
    if (protonVersion != Hotfix) {
        config["protonVersion"] = protonVersion;
    }
    if (!environmentVariables.empty()) {
        QJsonArray env_array;
        for (const auto &var : environmentVariables) {
            env_array.append(var);
        }
        config["environmentVariables"] = env_array;
    }
    if (steamRuntimeVersion != None) {
        config["steamRuntimeVersion"] = steamRuntimeVersion;
    }
    return config;
}
void UserConfig::updateFromJson(const QJsonObject &json) {
    //check types for possible errors?
    if (json.contains("useHomeIsolation")) {
        useHomeIsolation = json["useHomeIsolation"].toBool();
    }
    if (json.contains("protonVersion")) {
        protonVersion = static_cast<ProtonVersion>(json["protonVersion"].toInt());
    }
    if (json.contains("environmentVariables")) {
        QJsonArray env_array = json["environmentVariables"].toArray();
        for (int i = 0; i < env_array.size(); i++) {
            environmentVariables.emplace_back(env_array[i].toString());
        }
    }
    if (json.contains("steamRuntimeVersion")) {
        steamRuntimeVersion = static_cast<SteamRuntimeVersion>(json["steamRuntimeVersion"].toInt());
    }
}

QJsonObject ConfigLoader::loadInstance(const fs::path &jsonPath) {
    // TODO: proper error handling
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << std::format("Error opening file at {}\n",
                                 jsonPath.string());
        throw(std::ios_base::failure(
            std::format("Error opening file at {}\n", jsonPath.string())));
    }
    QByteArray fileData = file.readAll();
    QJsonDocument loadedFile(QJsonDocument::fromJson(fileData));
    return loadedFile.object();
}

std::filesystem::path ConfigLoader::getConfigPath() {
    return fs::canonical("/proc/self/exe").parent_path() / "config";
}
QString ConfigLoader::getConfigPathQString() {
    // Use QSettings to get /home/user/.config/appname or maybe use XDG_CONFIG_HOME?
    QDir result("/proc/self/exe");
    QDir exePath = result.canonicalPath();
    exePath.cd("..");
    return exePath.absolutePath() + "/config/";
}
void ConfigLoader::CheckCorrectness(const QJsonObject &config) {
    if (config.isEmpty()) {
        throw std::invalid_argument(
            "ConfigManager::CheckCorrectness: empty config");
    }
    const std::vector<std::pair<QString, QJsonValue::Type>> required_keys = {
        {"instanceName", QJsonValue::String},
        {  "pluginUUID", QJsonValue::String},
        {    "basePath", QJsonValue::String},
        {    "gamePath", QJsonValue::String},
        {     "mounted",   QJsonValue::Bool},
        {     "modList",  QJsonValue::Array}
    };

    for (const auto &[key, expected_type] : required_keys) {
        if (!config.contains(key))
            throw std::runtime_error(
                "ConfigManager::CheckCorrectness: missing JSON value: " +
                key.toStdString());
        if (config[key].type() != expected_type)
            throw std::runtime_error(
                "ConfigManager::CheckCorrectness: type mismatch for key " +
                key.toStdString());
    }
}

void ConfigLoader::CheckModListCorrectness(const QJsonArray &modList) {
    const std::vector<std::pair<QString, QJsonValue::Type>> required_keys = {
        {   "name", QJsonValue::String},
        {   "path", QJsonValue::String},
        {"enabled", QJsonValue::Bool},
        {   "type", QJsonValue::Double},
    };
    // if (modList.isEmpty()) {
    //     // throw std::runtime_error(
    //     //     "ConfigManager::CheckModListCorrectness: empty modList");
    // }
    for (auto iter = modList.begin(); iter != modList.end(); ++iter) {
        QJsonObject mod = iter->toObject();
        for (const auto &[key, expected_type] : required_keys) {
            if (!mod.contains(key))
                throw std::runtime_error(
                    "ConfigManager::CheckModListCorrectness: missing JSON "
                    "value: " +
                    key.toStdString());
            if (mod[key].type() != expected_type)
                throw std::runtime_error(
                    "ConfigManager::CheckModListCorrectness: type mismatch for "
                    "key:" +
                    key.toStdString());
        }
    }
}

std::vector<std::string> ConfigLoader::getSavedInstanceNames() {
    std::vector<std::string> result;
    fs::path configPath = getConfigPath();
    for (const auto &file : fs::directory_iterator(configPath)) {
        if (file.is_regular_file() && file.path().extension() == ".json") {
            result.push_back(file.path().filename());
        }
    }
    return result;
}
} // namespace LigmaCore
