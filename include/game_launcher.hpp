#pragma once
#include "steam_finder.hpp"

#include <QString>
#include <string>
#include <vector>

namespace LigmaCore {

class GameLauncher {
  public:
    static void openNative(const std::string &gamePath
                           //,const std::vector<QString> &args
    );
    static void
    openWithProton(const std::string &gamePath,
                   const std::vector<QString> &envVars,
                   const std::string &compatDataPath, const int &gameID,
                   const ProtonVersion &version = ProtonVersion::Hotfix);
};

} // namespace LigmaCore
