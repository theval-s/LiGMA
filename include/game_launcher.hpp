#pragma once
#include "config.hpp"
#include "steam_finder.hpp"

#include <QString>
#include <string>
#include <vector>

namespace LigmaCore {

class GameLauncher {
  public:
    static void openNative(const std::string &gamePath
                           ,const UserConfig &cfg
    );
    static void
    openWithProton(const std::string &gamePath,
                   const std::string &compatDataPath, const int &gameID,
                   const UserConfig &cfg);
};

} // namespace LigmaCore
