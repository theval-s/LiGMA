#include "config.hpp"
#include <QFile>
#include <QJsonDocument>
#include <format>
#include <iostream>

namespace LigmaCore {
    void ConfigManager::saveInstance(const fs::path &configPath, const QJsonObject &config) {
        // TODO: proper error handling (check for .json etc etc
        // probably should rewrite this to std?
        QFile file(configPath);
        // Truncates by default
        if (!file.open(QIODevice::WriteOnly)) {
            std::cerr << std::format("Error creating/modifying file at {}\n",
                                     configPath.string());
            throw(std::ios_base::failure(std::format(
                //most likely dir not existing
                "Error creating/modifying file at {}\n", configPath.string())));
        }
        file.write(QJsonDocument(config).toJson());
    }

    QJsonObject ConfigManager::loadInstance(const fs::path &jsonPath) {
        // TODO: proper error handling
        QFile file(jsonPath);
        if (!file.open(QIODevice::ReadOnly)) {
            std::cerr << std::format("Error opening file at {}\n", jsonPath.string());
            throw(std::ios_base::failure(
                std::format("Error opening file at {}\n", jsonPath.string())));
        }
        QByteArray fileData = file.readAll();
        QJsonDocument loadedFile(QJsonDocument::fromJson(fileData));
        return loadedFile.object();
    }

    std::filesystem::path ConfigManager::getConfigPath() {
        return fs::canonical("/proc/self/exe").parent_path() / "config";
    }

    std::vector<std::string> ConfigManager::getSavedInstanceNames() {
        std::vector<std::string> result;
        //maybe using some env variable to store path to config?
        fs::path configPath = getConfigPath();
        for (const auto &file: fs::directory_iterator(configPath)) {
            if (file.is_regular_file() && file.path().extension() == ".json") {
                result.push_back(file.path().filename());
            }
        }
        return result;
    }
} // namespace LigmaCore
