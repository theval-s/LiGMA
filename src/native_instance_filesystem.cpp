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
    //  TODO::
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
    fs::path destPath = m_basePath / LIGMA_MOD_FILES_DIR /
                        sanitizeForPath(modName).toStdString() /
                        destPathString.toStdString();
    copyMod(modPath, destPath);
    m_modList.emplace_back(modName, destPath, true, ModType::GameRoot);
    saveState();
    // TODO:: more error handling
}

void NativeInstanceFilesystem::mountGameFilesystem() {
    // TODO: error handling
    if (m_mounted) return;
    try {
        QString lower_dirs = getModsLowerDirsString(ModType::GameRoot);
        lower_dirs += m_gamePath.string();
        FuseOverlayFSMount::mount(
            m_basePath / LIGMA_GAME_MERGED_DIR, lower_dirs.toStdString(),
            m_basePath / LIGMA_GAME_UPPER_DIR, m_basePath / LIGMA_GAME_WORK_DIR);
        m_mounted = true;
        saveState();
    } catch (const std::exception &e) {
        std::cerr << e.what();
    }
}

// As of right now in UI I handle this with QProcess::start()
void NativeInstanceFilesystem::runGame() {
    // mountModFilesystem();
    if (!m_mounted) mountGameFilesystem();
    GameLauncher::openNative(m_basePath / LIGMA_GAME_MERGED_DIR /
                                 m_gamePlugin->executableName().toStdString(),
                             m_userConfig);
}

void NativeInstanceFilesystem::unmountGameFilesystem() {
    try {
        FuseOverlayFSMount::unmount(m_basePath / LIGMA_GAME_MERGED_DIR);
    } catch (const std::exception &e) {
        std::cerr << e.what();
    }
    m_mounted = false;
    saveState();
}

// void NativeInstanceFilesystem::saveState() const {
//     ConfigManager::saveInstance(m_configPath, this->toJson());
// }
} // namespace LigmaCore
