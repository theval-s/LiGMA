//
// Created by val on 3/10/25.
//

#include "i_instance_filesystem.hpp"

#include "overlayfs_mount.hpp"
#include "plugin_handler.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QString>
#include <filesystem>
#include <iostream>

namespace LigmaCore {

QString sanitizeForPath(const QString &input) {
    QString forbiddenChars(R"(()<>";:\|/?*[].^&$%#@)");
    QString result = input;
    for (const QChar &c : forbiddenChars) result.remove(c);
    result.replace(' ', '_');
    return result;
}

QString BaseInstanceFilesystem::resolveMacros(const QString &input) {
    //TODO:
    //  restructure this? like this way non-proton Instance can still resolve $PREFIX
    const std::map<QString, QString> macros{
        {"$PREFIX", "pfx"}
    };
    QString result = input;
    for (const auto &macro : macros) {
        result = result.replace(macro.first, macro.second);
    }
    return result;
}

//TODO: rewrite/add constructors from QDir, QString
BaseInstanceFilesystem::BaseInstanceFilesystem(
    const QString &instanceName, const fs::path &basePath,
    const fs::path &gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin)
    : basePath(basePath), gamePath(gamePath), instanceName(instanceName),
      configPath(ConfigLoader::getConfigPathQString() +
                 (sanitizeForPath(instanceName) + ".json")),
      gamePlugin(std::move(gamePlugin)) {
    // TODO: check if gamePath has gameExeName
    //       error handling
    //       check if plugin is appropriate?
    this->pluginUUID = this->gamePlugin->pluginUUID();
    if (this->pluginUUID.isEmpty()) {
        throw std::runtime_error(
            "BaseInstanceFilesystem(): Failed to get UUID from plugin");
    }

    fs::create_directory(basePath,
                         basePath.parent_path()); // to copy permissions
    fs::create_directory(basePath / LIGMA_GAME_MERGED_DIR, basePath);
    fs::create_directory(basePath / LIGMA_GAME_UPPER_DIR, basePath);
    fs::create_directory(basePath / LIGMA_GAME_WORK_DIR, basePath);
    fs::create_directory(basePath / LIGMA_MOD_FILES_DIR);
    //TODO: add logging

    for (const auto &var : this->gamePlugin->environmentVariables()) {
        userConfig.addEnvironmentVariable(var);
    }

}
BaseInstanceFilesystem::BaseInstanceFilesystem(
    const QJsonObject &config, const fs::path &pathToConfig,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
        plugin)
            : gamePlugin(std::move(plugin)){
    ConfigLoader::CheckCorrectness(config);

    instanceName = config["instanceName"].toString();
    basePath = config["basePath"].toString().toStdString();
    gamePath = config["gamePath"].toString().toStdString();
    pluginUUID = config["pluginUUID"].toString();
    mounted = config["mounted"].toBool();
    QJsonArray mod_list_array = config["modList"].toArray();
    if (pluginUUID != gamePlugin->pluginUUID()) {
        throw std::runtime_error(
            "BaseInstanceFilesystem(): UUID in supplied plugin and supplied "
            "config file are different");
    }
    //TODO: Make UserConfig from json

    ConfigLoader::CheckModListCorrectness(mod_list_array);

    for (auto iter = mod_list_array.begin(); iter != mod_list_array.end();
         ++iter) {
        QJsonObject mod_obj = iter->toObject();
        modList.emplace_back(mod_obj["name"].toString(),
                             fs::path(mod_obj["path"].toString().toStdString()),
                             mod_obj["enabled"].toBool(),
                             static_cast<ModType>(mod_obj["type"].toInt()));
    }
    userConfig.updateFromJson(config);
    configPath = QString::fromStdString(pathToConfig);
    if (FuseOverlayFSMount::isMounted(basePath / LIGMA_GAME_MERGED_DIR)) {
        if (!mounted) {
            std::cerr << "BaseInstanceFilesystem(): config says unmounted, but folder is mounted. Treating it as mounted...\n";
            mounted = true;
            BaseInstanceFilesystem::saveState();
        }
    } else if (mounted) {
        std::cerr << "BaseInstanceFilesystem(): config says mounted, but folder is unmounted. Treating it as unmounted...\n\n";
        mounted = false;
        BaseInstanceFilesystem::saveState();
    }
}

void BaseInstanceFilesystem::copyMod(const QDir &modPath,
                                     const QDir &destPath) {
    //throw std::logic_error("Not implemented");
    copyMod(modPath.filesystemPath(), destPath.filesystemPath());
}
void BaseInstanceFilesystem::copyMod(const fs::path &modPath,
                                     const fs::path &destPath) {
    if (mounted) unmountGameFilesystem();

    //TODO: probably rewrite to QDir
    fs::create_directories(destPath);
    //try {
    for (const auto &file : fs::recursive_directory_iterator(modPath)) {
        const fs::path &source_file = file.path();
        fs::path dest_file = destPath / fs::relative(source_file, modPath);
        //try {
        fs::create_directories(dest_file.parent_path());
        fs::copy(source_file, dest_file,
                 fs::copy_options::overwrite_existing | // probably should
                                                        // remove overwrite
                                                        // and handle errors
                     fs::copy_options::recursive);
        std::cerr << "filesystem.cpp: copied " << source_file << std::endl;
        // } catch (const fs::filesystem_error &e) {
        //     std::cerr << "filesystem.cpp: copy exception: " << e.what()
        //               << std::endl;
        //     return;
        // }
    }
    // } catch (const fs::filesystem_error &e) {
    //     std::cerr << "filesystem.cpp: directory iterator exception: "
    //               << e.what() << std::endl;
    //     return;
    // }
    //modList.emplace_back(modName, dest_folder, true);
    //saveState();
}

void BaseInstanceFilesystem::cleanState() {
    //Maybe list all files to be deleted before actually deleting?
    if (mounted) unmountGameFilesystem();
    std::cerr << std::format("InstanceFilesystem::cleanState() with base path "
                             "<{}> removed {} files\n",
                             basePath.string(), fs::remove_all(basePath));
    fs::remove(configPath.toStdString());
}

void BaseInstanceFilesystem::removeMod(const size_t &id) {
    if (mounted) unmountGameFilesystem();

    fs::remove_all(modList[id].path);
    modList.erase(modList.begin() + id);
    saveState();
}

QJsonObject BaseInstanceFilesystem::toJson() const {
    QJsonArray mods_array;
    for (const auto &[mod_name, mod_path, mod_enabled, mod_type] : modList) {
        QJsonObject mod_object;
        mod_object["name"] = mod_name;
        mod_object["path"] = QString::fromStdString(mod_path);
        mod_object["enabled"] = mod_enabled;
        mod_object["type"] = static_cast<int>(mod_type);
        // in case something else gets added I've structured it like that
        mods_array.append(mod_object);
    }
    QJsonObject obj = {
        {"instanceName",                     instanceName},
        {  "pluginUUID",                       pluginUUID},
        {    "basePath", QString::fromStdString(basePath)},
        {    "gamePath", QString::fromStdString(gamePath)},
        {     "mounted",              QJsonValue(mounted)},
        {     "modList",                       mods_array},
    };
    QJsonObject user_json = userConfig.toJson();
    for (auto it = user_json.begin(); it != user_json.end(); ++it) {
        obj.insert(it.key(), it.value());
    }
    return obj;
}

QString
BaseInstanceFilesystem::getModsLowerDirsString(const ModType &type) const {
    QString mod_folders;
    // mod_folders += '"';
    // QProcess automatically escapes with quotes if needed
    for (const auto &mod : modList) {
        if (mod.type == type) mod_folders += mod.path.string() + ":";
    }
    //mod_folders += gamePath.path();
    return mod_folders;
}

} // namespace LigmaCore