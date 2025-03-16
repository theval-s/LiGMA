//
// Created by val on 3/12/25.
//

#include "steam_finder.hpp"

#include <QDir>
#include <array>
#include <filesystem>
#include <format>

std::filesystem::path LigmaCore::SteamFinder::findSteamPath() {
    static std::string steam_path;
    if (!steam_path.empty()) {
        return steam_path;
    }
    std::array<std::string, 5> steamPaths = {
        QDir::home().filesystemAbsolutePath() / ".local/share/Steam/",
        QDir::home().filesystemAbsolutePath() / ".steam/steam/",
        QDir::home().filesystemAbsolutePath() / ".steam/root/",
        QDir::home().filesystemAbsolutePath() /
            "snap/steam/common/.local/share/Steam/",
        QDir::home().filesystemAbsolutePath() /
            ".var/app/com.valvesoftware.Steam/data/Steam/"};
    for (auto dir : steamPaths) {
        if (std::filesystem::exists(std::filesystem::path(dir))) {
            steam_path = dir;
            return dir;
        }
    }
    throw std::runtime_error(
        "SteamFinder::findSteamPath(): could not find Steam path");
}
std::filesystem::path
LigmaCore::SteamFinder::findProtonPath(const ProtonVersion &version) {
    std::filesystem::path searchPath = findSteamPath();
    searchPath = searchPath/ "steamapps"/"common";
    if (std::filesystem::exists(searchPath / protonDirName[version])) {
        return searchPath / protonDirName[version] / "proton";
    } else
        throw std::runtime_error(
            std::format("SteamFinder::findProtonPath: Failed to find Proton "
                        "Executable of {} version",
                        protonDirName[version]));
}

std::filesystem::path LigmaCore::SteamFinder::findCompatDataDir(int gameID) {
    std::filesystem::path searchPath =
        findSteamPath() / "steamapps" / "compatdata" / std::to_string(gameID);
    if (std::filesystem::exists(searchPath)) return searchPath;
    //can happen if game is installed in another library folder
    throw std::runtime_error("SteamFinder::findCompatDataDir(): could not find "
                             "compatdata for game in Steam folder");
}