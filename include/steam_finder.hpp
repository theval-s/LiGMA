//
// Created by val on 3/12/25.
//

#pragma once
#include <filesystem>
#include <map>
#include <string>

namespace LigmaCore {
enum ProtonVersion { Hotfix, Experimental, Eight };

namespace SteamFinder {

//this should be probably refactored to make it a bit more optimized
inline std::map<ProtonVersion, std::string> protonDirName = {
    {      ProtonVersion::Hotfix,         "Proton Hotfix"},
    {ProtonVersion::Experimental, "Proton - Experimental"},
    {       ProtonVersion::Eight,            "Proton 8.0"}
};
std::filesystem::path findSteamPath();
std::filesystem::path
findProtonPath(const ProtonVersion &version = ProtonVersion::Hotfix);
std::filesystem::path findCompatDataDir(int gameID);
} //namespace SteamFinder
} // namespace LigmaCore
