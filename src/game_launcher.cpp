//
// Created by val on 3/3/25.
//

#include "game_launcher.hpp"

#include <QProcess>
#include <filesystem>
#include <iostream>

namespace LigmaCore {
void GameLauncher::openNative(const std::string &gamePath
                              ,const UserConfig &cfg
) {
    //TODO: add steam runtime and UserConfig int
    try {
        QProcess process;
        process.setProgram(QString::fromStdString(gamePath));
        QStringList args_list;
        //for (const auto &arg : args) {
        //    args_list << arg;
        //}
        //process.setArguments(args_list);
        process.startDetached();
    } catch (const std::exception &e) {
        std::cerr << e.what();
    }
}
void GameLauncher::openWithProton(const std::string &gamePath,
                                  const std::string &compatDataPath,
                                  const int &gameID,
                                  const UserConfig &cfg) {
    QProcess protonProcess;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // my tested pipeline without any other tools (Balatro for example):
    //   export STEAM_COMPAT_DATA_PATH=/path/to/overlayed/prefix
    //   export STEAM_COMPAT_CLIENT_INSTALL_PATH=$HOME/.steam/root/
    //   export SteamGameId=2379780
    //   export SteamAppId=2379780
    //   export WINEDLLOVERRIDES="version=n,b" #(required by lovely injector for balatro)
    //   /path/to/proton/proton run /path/to/overlayed/game/Balatro.exe
    //
    //   issue: steam overlay is not working (and no screenshots)
    //   winedevice might be left hanging and would not allow to unmount?
    //   i did not find a way to make steam launch game through proton with custom executable and custom prefix
    //   except for setting steam launch arguments and launching from steam:
    //   WINEDLLOVERRIDES="version=n,b" STEAM_COMPAT_DATA_PATH=/tmp/test/pfx/ /home/val/.steam/root/steamapps/common/Proton\ Hotfix/proton run
    //   /tmp/test/game/merged/Balatro.exe # %command%

    //  Possible way:
    // Steam launches games like this:
    //    /home/val/.local/share/Steam/ubuntu12_32/steam-launch-wrapper --
    //    /home/val/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=2379780 --
    //    '/home/val/.local/share/Steam/steamapps/common/SteamLinuxRuntime_sniper'/_v2-entry-point --verb=waitforexitandrun --
    //    '/home/val/.local/share/Steam/steamapps/common/Proton Hotfix'/proton waitforexitandrun  '/home/val/.local/share/Steam/steamapps/common/Balatro/Balatro.exe'
    //    (1 command)
    // I got this by setting "echo '%command%' > filename" in steam game launch options
    // So maybe launching with steam-launch-wrapper would correctly launch with SteamOverlay?

    //TODO: add steam runtime
    env.insert("STEAM_COMPAT_DATA_PATH",
               QString::fromStdString(compatDataPath));
    env.insert("STEAM_COMPAT_CLIENT_INSTALL_PATH",
               QString::fromStdString(SteamFinder::findSteamPath()));

    if (gameID != 0) {
        env.insert("SteamGameId", QString::number(gameID));
        env.insert("SteamAppId", QString::number(gameID));
    }

    for (const auto &v : cfg.getEnvironmentVariables()) {
        if (int pos = v.indexOf('='); pos > 0) {
            QString key = v.left(pos);
            QString value = v.mid(pos+1);
            env.insert(key, value);
        }
    }
    protonProcess.setProcessEnvironment(env);

    // protonProcess.setProgram("env");
    // protonProcess.startDetached();
    try {
        protonProcess.setProgram(
            QString::fromStdString(SteamFinder::findProtonPath(cfg.getProtonVersion())));
        QStringList args;
        args << "run" << QString::fromStdString(gamePath);
        protonProcess.setArguments(args);
        protonProcess.startDetached();
    } catch (const std::exception &e) {
        std::cerr << e.what();
    }

    //TODO: detect preferred proton runtime from manifests
    //      allow using Proton not from steam by supplying proton path (in app or through env)
}

} // namespace LigmaCore