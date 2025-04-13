//
// Created by val on 3/10/25.
//

#include "proton_instance_filesystem.hpp"

#include "game_launcher.hpp"
#include "overlayfs_mount.hpp"

#include <iostream>

namespace LigmaCore {

ProtonInstanceFilesystem::ProtonInstanceFilesystem(
    const QString &instanceName, const fs::path &basePath,
    const fs::path &gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin)
    : BaseInstanceFilesystem(instanceName, basePath, gamePath,
                             std::move(gamePlugin)) {

    //gamePlugin->initializeModState((basePath / UPPER_DIR).string());
    fs::create_directory(basePath / LIGMA_MOD_FILES_DIR / LIGMA_PREFIX_MODS_DIR,
                         basePath);
    fs::create_directory(basePath / LIGMA_PREFIX_WORK_DIR, basePath);
    fs::create_directory(basePath / LIGMA_PREFIX_MERGED_DIR, basePath);
    fs::create_directory(basePath / LIGMA_PREFIX_UPPER_DIR, basePath);
}

ProtonInstanceFilesystem::ProtonInstanceFilesystem(
    const QJsonObject &config, const fs::path &pathToConfig,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin)
    : BaseInstanceFilesystem(config, pathToConfig, std::move(gamePlugin)) {}

void ProtonInstanceFilesystem::mountGameFilesystem() {
    if (mounted) return;
    try {
        QString lower_dirs = getModsLowerDirsString(ModType::GameRoot);
        lower_dirs += gamePath.string();
        FuseOverlayFSMount::mount(
            basePath / LIGMA_GAME_MERGED_DIR, lower_dirs.toStdString(),
            basePath / LIGMA_GAME_UPPER_DIR, basePath / LIGMA_GAME_WORK_DIR);
        lower_dirs = getModsLowerDirsString(ModType::Prefix);
        lower_dirs +=
            SteamFinder::findCompatDataDir(gamePlugin->gameID()).string();
        FuseOverlayFSMount::mount(basePath / LIGMA_PREFIX_MERGED_DIR,
                                  lower_dirs.toStdString(),
                                  (basePath / LIGMA_PREFIX_UPPER_DIR).string(),
                                  basePath / LIGMA_PREFIX_WORK_DIR);
        mounted = true;
        saveState();

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void ProtonInstanceFilesystem::unmountGameFilesystem() {
    if (!mounted) return;
    try {
        FuseOverlayFSMount::unmount(basePath / LIGMA_GAME_MERGED_DIR);
        FuseOverlayFSMount::unmount(basePath / LIGMA_PREFIX_MERGED_DIR);
        mounted = false;
        saveState();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
void ProtonInstanceFilesystem::addMod(const fs::path &modPath,
                                      const QString &modName,
                                      const QString &destPathString) {
    if (!destPathString.isEmpty() &&
        destPathString[0] ==
            "/") //no need to write destPathString because in my plan user has to choose it right before this function
        throw std::invalid_argument(
            "Destination path contains '/' as a first symbol, but paths "
            "somewhere beside game folder or prefix are not supported");
    const ModType type = (destPathString.contains("$PREFIX") ||
                          destPathString.contains("drive_c"))
                             ? ModType::Prefix
                             : ModType::GameRoot;
    fs::path destPathResolved = resolveMacros(destPathString).toStdString();
    //we get string like: /home/user/ or pfx
    fs::path modBasePath = basePath / LIGMA_MOD_FILES_DIR;
    fs::path destPath;
    if (type == ModType::Prefix) {
        modBasePath = modBasePath / LIGMA_PREFIX_MODS_DIR /
                      sanitizeForPath(modName).toStdString();
        destPath = modBasePath / destPathResolved /
                   sanitizeForPath(modName).toStdString();
    } else if (type == ModType::GameRoot) {
        modBasePath = modBasePath / sanitizeForPath(modName).toStdString();
        destPath = modBasePath / destPathResolved;
    }
    copyMod(modPath, destPath);
    modList.emplace_back(modName, modBasePath, true, type);
    saveState();
    //copyMod throws exceptions, so if we're here then it succeded
    //TODO: add checking of modPath structure
}
void ProtonInstanceFilesystem::runGame() {
    if (!mounted) mountGameFilesystem();
    GameLauncher::openWithProton(basePath / LIGMA_GAME_MERGED_DIR /
                                     gamePlugin->executableName().toStdString(),
                                 basePath / LIGMA_PREFIX_MERGED_DIR,
                                 gamePlugin->gameID(), userConfig);
}

//QJsonObject ProtonInstanceFilesystem::toJson() const {}

} // namespace LigmaCore