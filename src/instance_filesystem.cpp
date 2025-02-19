#include "instance_filesystem.hpp"

#include "plugin_handler.hpp"

#include <QJsonArray>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <overlayfs_mount.hpp>
#include <stdexcept>
#include <utility>

namespace LigmaCore {
InstanceFilesystem::InstanceFilesystem(const std::string &instanceName,
                                       const fs::path &basePath,
                                       const fs::path &gamePath,
                                       LigmaPlugin *gamePlugin)
    : m_basePath(basePath), m_gamePath(gamePath), m_instanceName(instanceName),
      m_configPath(fs::canonical("/proc/self/exe").parent_path() / "config" /
                   (sanitizeForPath(instanceName) + ".json")) {
    m_gamePlugin = gamePlugin;
    // TODO: check if gamePath has gameExeName
    //       error handling
    //       check if plugin is appropriate?

    fs::create_directory(basePath,
                         basePath.parent_path()); // to copy permissions
    fs::create_directory(basePath / GAME_WORK_DIR, basePath);
    fs::create_directory(basePath / UPPER_DIR, basePath);
    gamePlugin->initializeModState((basePath / UPPER_DIR).string());
    fs::create_directory(basePath / MOD_FILES_DIR, basePath);
    fs::create_directory(basePath / MERGED_DIR, basePath);

    this->m_pluginUUID = gamePlugin->pluginUUID();
    if (m_pluginUUID.empty())
        throw std::runtime_error(
            "InstanceFilesystem(): Failed to get UUID from plugin");
    std::cerr << std::format("InstanceFilesystem initialized with basePath = "
                             "{}\ngamePath = {}\nInstance Name = {}\n",
                             basePath.string(), gamePath.string(),
                             instanceName);

    saveState();
}

InstanceFilesystem::InstanceFilesystem(const QJsonObject &config,
                                       const fs::path &pathToConfig)
    : m_configPath(pathToConfig) {
    const std::vector<QString> requiredKeys = {"instanceName", "pluginUUID",
                                               "basePath",     "gamePath",
                                               "mounted",      "modList"};
    for (const auto &k : requiredKeys)
        if (!config.contains(k))
            throw std::runtime_error("missing JSON value: " + k.toStdString());

    m_instanceName = config["instanceName"].toString().toStdString();
    m_basePath = config["basePath"].toString().toStdString();
    m_gamePath = config["gamePath"].toString().toStdString();
    m_pluginUUID = config["pluginUUID"].toString().toStdString();
    gameMounted = config["mounted"].toBool();
    QJsonArray mod_list_array = config["modList"].toArray();
    if (!mod_list_array.isEmpty()) {
        // rewrite this loop
        for (auto iter = mod_list_array.begin(); iter != mod_list_array.end();
             ++iter) {
            // for (const auto &item: mod_list_array) {
            // QJsonObject mod_obj = item.toObject();
            QJsonObject mod_obj = iter->toObject();
            if (mod_obj.contains("name") && mod_obj.contains("path")) {
                m_modList.emplace_back(
                    mod_obj["name"].toString().toStdString(),
                    fs::path(mod_obj["path"].toString().toStdString()));
            } else
                throw(std::runtime_error(
                    "Errors in JSON parsing, modList array entry does not have "
                    "name or path keys"));
        }
    }
    m_gamePlugin = PluginHandler::getInstance().getPluginByUUID(m_pluginUUID);
    // I probably need to have plugins loaded even after initial creation
    // But that differs from game to game. Right now I only tested games with
    // simple modding where you just put mods into mods folder and that's it

    if (FuseOverlayFSMount::isMounted(m_basePath / MERGED_DIR) &&
        !gameMounted) {
        std::cerr << "InstanceFilesystem(QJsonObject, fs::path):"
                     "gameMounted is false but merged dir has fuse fs type. "
                     "Treating it as mounted...\n";
        gameMounted = true;
        saveState();
    }
    /*
     */
}

std::string InstanceFilesystem::sanitizeForPath(const std::string &input) {
    std::string result = input;
    // maybe regex?
    const std::string forbiddenChars = R"( ()<>":;\|/?*[].^&$%#@)";
    std::erase_if(result, [&forbiddenChars](const char &c) {
        return forbiddenChars.find(c) != std::string::npos;
    });
    std::replace(result.begin(), result.end(), ' ', '_');
    return result;
}

QJsonObject InstanceFilesystem::toJson() const {
    QJsonArray mods_array;
    for (const auto &[mod_name, mod_path] : m_modList) {
        QJsonObject mod_object;
        mod_object["name"] = QString::fromStdString(mod_name);
        mod_object["path"] = QString::fromStdString(mod_path.string());
        // in case something else gets added I've structured it like that
        mods_array.append(mod_object);
    }
    return {
        {"instanceName",      QString::fromStdString(m_instanceName)},
        {  "pluginUUID",        QString::fromStdString(m_pluginUUID)},
        {    "basePath", QString::fromStdString(m_basePath.string())},
        {    "gamePath", QString::fromStdString(m_gamePath.string())},
        {     "mounted",                     QJsonValue(gameMounted)},
        {     "modList",                                  mods_array},
    };
}

void InstanceFilesystem::addMod(const fs::path &modPath,
                                const std::string &modName) {
    // not yet truly implemented - just testing right now
    // expected structure of mod handling is:
    //    unzip, manage mods in their folder (by name)
    //    add all mods in a single folder into _modsDir

    // As of right now it's expected that mod is in the same structure that it
    // should be in _mods folder, Proper handling is WIP
    //  TODO:: archive extraction integration
    //         error handling (incl. check for repeated mods)
    if (gameMounted) unmount();

    fs::path dest_folder =
        m_basePath / MOD_FILES_DIR / sanitizeForPath(modName);
    fs::create_directory(dest_folder, m_basePath);
    try {
        for (const auto &file : fs::recursive_directory_iterator(modPath)) {
            const fs::path &source_file = file.path();
            fs::path dest_file =
                dest_folder / fs::relative(source_file, modPath);
            try {
                fs::create_directories(dest_file.parent_path());
                fs::copy(
                    source_file, dest_file,
                    fs::copy_options::overwrite_existing | // probably should
                                                           // remove overwrite
                                                           // and handle errors
                        fs::copy_options::recursive);
                std::cerr << "filesystem.cpp: copied " << source_file
                          << std::endl;
            } catch (const fs::filesystem_error &e) {
                std::cerr << "filesystem.cpp: copy exception: " << e.what()
                          << std::endl;
                return;
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "filesystem.cpp: directory iterator exception: "
                  << e.what() << std::endl;
        return;
    }
    m_modList.emplace_back(modName, dest_folder);
    saveState();
    // TODO:: more error handling
}

std::string InstanceFilesystem::getLowerDirsString() {
    std::string mod_folders;
    // mod_folders += '"'; //QProcess automatically escapes with quotes if
    // needed
    for (int i = m_modList.size() - 1; i >= 0; i--) {
        mod_folders += m_modList[i].second.string() + ":";
    }
    // mod_folders += m_gamePath += '"';
    return mod_folders;
    // rightmost dir is the lowers layer, leftmost is the highest, so order is
    // reversed and game is added on top
}

void InstanceFilesystem::removeMod(const size_t &id) {
    if (gameMounted) {
        unmount();
    }
    fs::remove_all(m_modList[id].second);
    m_modList.erase(m_modList.begin() + id);
    saveState();
}

void InstanceFilesystem::mountGameFilesystem() {
    // TODO: error handling
    // test for specific stuff
    /*
    const fs::path old_work_dir = fs::current_path();
    current_path(m_basePath);
    std::string command = std::format(
        "fuse-overlayfs -o lowerdir={} -o upperdir={} -o workdir={} {}",
        getLowerDirsString(), UPPER_DIR, GAME_WORK_DIR, MERGED_DIR);
    int return_code = system(command.c_str());
    std::cerr << format("system({}) executed with retcode={}\n", command,
                        return_code);
    if (return_code != 0) {
        // fuse-overlayfs and fusermount3 print the errors in stdout
        //  if already mounted its 255 i think? 256 = not permitted?
        throw(std::runtime_error("fuse-overlayfs failed"));
    }
    current_path(old_work_dir);*/
    try {
        FuseOverlayFSMount::mount(m_basePath / MERGED_DIR, getLowerDirsString(),
                                  UPPER_DIR, GAME_WORK_DIR);
    } catch (const std::exception &e) {
        std::cerr << e.what();
        return;
    }
    gameMounted = true;
    saveState();
}

// As of right now in UI I handle this with QProcess::start()
void InstanceFilesystem::openGame() {
    // mountModFilesystem();
    if (!gameMounted) try {
            mountGameFilesystem();
        } catch (std::exception &e) {
            std::cerr << e.what();
            return;
        }
    current_path(m_basePath / MERGED_DIR);
    // maybe using fork to make it non blocking?
    // I also should sanitize this stuff i guess?
    const std::string command =
        std::format("./{}", m_gamePlugin->executeCommand());
    system(command.c_str());
}

void InstanceFilesystem::unmount() {
    try {
        FuseOverlayFSMount::unmount(m_basePath / MERGED_DIR);
    } catch (const std::expression &e) {
        std::cerr << e.what();
    }
    gameMounted = false;
    saveState();
}

void InstanceFilesystem::cleanState() {
    if (gameMounted) unmount();
    std::cerr << std::format("InstanceFilesystem::cleanState() with base path "
                             "<{}> removed {} files\n",
                             m_basePath.string(), fs::remove_all(m_basePath));
}

void InstanceFilesystem::saveState() const {
    ConfigManager::saveInstance(m_configPath, this->toJson());
}
} // namespace LigmaCore
