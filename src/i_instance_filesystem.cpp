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
#include <utility>

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
    fs::path gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin)
    : m_basePath(basePath), m_gamePath(std::move(gamePath)), m_instanceName(instanceName),
      m_configPath(ConfigLoader::getConfigPathQString() +
                 (sanitizeForPath(instanceName) + ".json")),
      m_gamePlugin(std::move(gamePlugin)) {
    // TODO: check if gamePath has gameExeName
    //       error handling
    //       check if plugin is appropriate?

    if (m_gamePlugin->pluginUUID().isEmpty()) {
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

    for (const auto &var : m_gamePlugin->environmentVariables()) {
        m_userConfig.addEnvironmentVariable(var);
    }
}
BaseInstanceFilesystem::BaseInstanceFilesystem(
    const QJsonObject &config, const fs::path &pathToConfig,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> plugin)
    : m_gamePlugin(std::move(plugin)) {
    ConfigLoader::CheckCorrectness(config);

    m_instanceName = config["instanceName"].toString();
    m_basePath = config["basePath"].toString().toStdString();
    m_gamePath = config["gamePath"].toString().toStdString();
    QString pluginUUID = config["pluginUUID"].toString();
    m_mounted = config["mounted"].toBool();
    QJsonArray mod_list_array = config["modList"].toArray();
    if (pluginUUID != m_gamePlugin->pluginUUID()) {
        throw std::runtime_error(
            "BaseInstanceFilesystem(): UUID in supplied plugin and supplied "
            "config file are different");
    }

    ConfigLoader::CheckModListCorrectness(mod_list_array);

    for (auto iter = mod_list_array.begin(); iter != mod_list_array.end();
         ++iter) {
        QJsonObject mod_obj = iter->toObject();
        m_modList.emplace_back(mod_obj["name"].toString(),
                             fs::path(mod_obj["path"].toString().toStdString()),
                             mod_obj["enabled"].toBool(),
                             static_cast<ModType>(mod_obj["type"].toInt()));
    }
    m_userConfig.updateFromJson(config);
    m_configPath = QString::fromStdString(pathToConfig);
    if (FuseOverlayFSMount::isMounted(m_basePath / LIGMA_GAME_MERGED_DIR)) {
        if (!m_mounted) {
            std::cerr << "BaseInstanceFilesystem(): config says unmounted, but "
                         "folder is m_mounted. Treating it as m_mounted...\n";
            m_mounted = true;
            BaseInstanceFilesystem::saveState();
        }
    } else if (m_mounted) {
        std::cerr << "BaseInstanceFilesystem(): config says m_mounted, but "
                     "folder is unmounted. Treating it as unmounted...\n\n";
        m_mounted = false;
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
    if (m_mounted) unmountGameFilesystem();

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
    if (m_mounted) unmountGameFilesystem();
    std::cerr << std::format("InstanceFilesystem::cleanState() with base path "
                             "<{}> removed {} files\n",
                             m_basePath.string(), fs::remove_all(m_basePath));
    fs::remove(m_configPath.toStdString());
}

void BaseInstanceFilesystem::removeMod(const int id) {
    if (m_mounted) unmountGameFilesystem();

    fs::remove_all(m_modList[id].path);
    m_modList.erase(m_modList.begin() + id);
    saveState();
}

QJsonObject BaseInstanceFilesystem::toJson() const {
    QJsonArray mods_array;
    for (const auto &[mod_name, mod_path, mod_enabled, mod_type] : m_modList) {
        QJsonObject mod_object;
        mod_object["name"] = mod_name;
        mod_object["path"] = QString::fromStdString(mod_path);
        mod_object["enabled"] = mod_enabled;
        mod_object["type"] = static_cast<int>(mod_type);
        // in case something else gets added I've structured it like that
        mods_array.append(mod_object);
    }
    QJsonObject obj = {
        {"instanceName",                     m_instanceName},
        {  "pluginUUID",                       m_gamePlugin->pluginUUID()},
        {    "basePath", QString::fromStdString(m_basePath)},
        {    "gamePath", QString::fromStdString(m_gamePath)},
        {     "mounted",              QJsonValue(m_mounted)},
        {     "modList",                       mods_array},
    };
    QJsonObject user_json = m_userConfig.toJson();
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
    for (const auto &mod : m_modList) {
        if (mod.type == type) mod_folders += mod.path.string() + ":";
    }
    //mod_folders += m_gamePath.path();
    return mod_folders;
}

} // namespace LigmaCore