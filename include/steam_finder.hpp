//
// Created by val on 3/12/25.
//

#pragma once
#include <filesystem>
#include <map>
#include <string>

namespace LigmaCore {
enum ProtonVersion {
    Hotfix = 0,
    Experimental = 1,
    Next = 2,
    Seven = 3,
    Eight = 4,
    NineBeta = 5,
};
enum SteamRuntimeVersion { None = 0, Scout = 1, Soldier = 2, Sniper = 3 };
namespace SteamFinder {
//TODO: Detect existing Proton and SteamRuntime installs
//this should be probably refactored to make it a bit more optimized
inline std::map<ProtonVersion, std::string> protonDirName = {
    {        ProtonVersion::Next,           "Proton Next"},
    {       ProtonVersion::Seven,            "Proton 7.0"},
    {       ProtonVersion::Eight,            "Proton 8.0"},
    {    ProtonVersion::NineBeta,     "Proton 9.0 (Beta)"},
    {      ProtonVersion::Hotfix,         "Proton Hotfix"},
    {ProtonVersion::Experimental, "Proton - Experimental"},
};

inline std::map<SteamRuntimeVersion, std::string> steamRuntimeDirName = {
    {   None,                          ""},
    {  Scout,         "SteamLinuxRuntime"},
    {Soldier, "SteamLinuxRuntime_soldier"},
    { Sniper,  "SteamLinuxRuntime_sniper"}
};
std::filesystem::path findSteamPath();
std::filesystem::path
findProtonPath(const ProtonVersion &version = ProtonVersion::Hotfix);
std::filesystem::path findCompatDataDir(int gameID);
std::filesystem::path findSteamRuntimePath(SteamRuntimeVersion version);
} //namespace SteamFinder
} // namespace LigmaCore
