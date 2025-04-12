#include "native_instance_filesystem.hpp"

#include "game_launcher.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <overlayfs_mount.hpp>
#include <stdexcept>
#include <utility>

namespace LigmaCore {
namespace fs = std::filesystem;

NativeInstanceFilesystem::NativeInstanceFilesystem(
    const QString &instanceName, const fs::path &basePath,
    const fs::path &gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin)
    : BaseInstanceFilesystem(instanceName, basePath, gamePath,
                             std::move(gamePlugin)) {
    //BaseInstanceFilesystem::saveState();
}
NativeInstanceFilesystem::NativeInstanceFilesystem(
    const QJsonObject &config, const fs::path &pathToConfig,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin)
    : BaseInstanceFilesystem(config, pathToConfig, std::move(gamePlugin)) {}

void NativeInstanceFilesystem::addMod(const fs::path &modPath,
                                      const QString &modName,
                                      const QString &destPathString) {

    // As of right now it's expected that mod is in the same structure that it
    // should be in _mods folder,  is WIP
    //  TODO:: archive extraction integration
    //         error handling (incl. check for repeated mods)
    //         proper mod structure detection
    if (destPathString[0] ==
        "/") //no need to write destPathString because in my plan user has to choose it right before this function
        throw std::invalid_argument(
            "Destination path contains '/' as a first symbol, but paths "
            "somewhere beside game folder or prefix are not currently "
            "supported");
    if (destPathString.contains("$PREFIX") ||
        destPathString.contains("drive_c")) {
        throw std::invalid_argument(
            "NativeInstanceFilesystem::addMod(): Destination path contained "
            "$PREFIX or 'drive_c', suggesting it's for a proton prefix, but "
            "this instance is native");
    }
    fs::path destPath = basePath / LIGMA_MOD_FILES_DIR /
                        sanitizeForPath(modName).toStdString() /
                        destPathString.toStdString();
    copyMod(modPath, destPath);
    modList.emplace_back(modName, destPath, true, ModType::GameRoot);
    saveState();
    // TODO:: more error handling
}

void NativeInstanceFilesystem::mountGameFilesystem() {
    // TODO: error handling
    if (mounted) return;
    try {
        QString lower_dirs = getModsLowerDirsString(ModType::GameRoot);
        lower_dirs += gamePath.string();
        FuseOverlayFSMount::mount(
            basePath / LIGMA_GAME_MERGED_DIR, lower_dirs.toStdString(),
            basePath / LIGMA_GAME_UPPER_DIR, basePath / LIGMA_GAME_WORK_DIR);
        mounted = true;
        saveState();
    } catch (const std::exception &e) {
        std::cerr << e.what();
    }
}

// As of right now in UI I handle this with QProcess::start()
void NativeInstanceFilesystem::runGame() {
    // mountModFilesystem();
    if (!mounted) mountGameFilesystem();
    GameLauncher::openNative(basePath / LIGMA_GAME_MERGED_DIR /
                             gamePlugin->executableName().toStdString(), userConfig);
}

void NativeInstanceFilesystem::unmountGameFilesystem() {
    try {
        FuseOverlayFSMount::unmount(basePath / LIGMA_GAME_MERGED_DIR);
    } catch (const std::exception &e) {
        std::cerr << e.what();
    } //probably should not catch exceptions here and handle them in ui
    mounted = false;
    saveState();
}

// void NativeInstanceFilesystem::saveState() const {
//     ConfigManager::saveInstance(m_configPath, this->toJson());
// }
} // namespace LigmaCore
