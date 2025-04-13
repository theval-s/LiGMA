//
// Created by val on 3/3/25.
//

#include "game_launcher.hpp"

#include <QProcess>
#include <filesystem>
#include <iostream>

namespace LigmaCore {
void GameLauncher::openNative(const std::string &gamePath,
                              const UserConfig &cfg) {
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList args_list;

    if (cfg.getSteamRuntimeVersion() != None) {
        if (cfg.getSteamRuntimeVersion() != Scout &&
            cfg.getUseHomeIsolation()) {
            //SteamRT's home isolation is not available on 1.0 because it uses
            //LD_PRELOAD instead of pressure_vessel
            env.insert("PRESSURE_VESSEL_SHARE_HOME", "0");
        }
        process.setProgram(QString::fromStdString(
            SteamFinder::findSteamRuntimePath(cfg.getSteamRuntimeVersion())));
        args_list << QString::fromStdString(gamePath);
        //testing
        args_list << "echo" << "1234";
    } else {
        process.setProgram(QString::fromStdString(gamePath));
    }
    process.setArguments(args_list);
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::MergedChannels);

    //Check if process runs successfully for first .25 seconds (in case executable is not existing or smth)
    //Should be fine for games
    process.start();
    process.waitForFinished(250);
    if (process.exitCode() != 0) {
        throw std::runtime_error(std::format(
            "Test process \"{}\" failed: {}", process.program().toStdString(),
            process.errorString().toStdString()));
    }
    process.kill();
    //Giving process a miniscule amount to get killed, or it will try to startDetached immediately
    //Maybe not the best way but I don't know how to implement it without changing to non-detached processes
    //And I want processes to be detached so you could close app
    process.waitForFinished(5);
    process.startDetached();
}
void GameLauncher::openWithProton(const std::string &gamePath,
                                  const std::string &compatDataPath,
                                  const int &gameID, const UserConfig &cfg) {
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
            QString value = v.mid(pos + 1);
            env.insert(key, value);
        }
    }
    if (cfg.getUseHomeIsolation()) {
        env.insert("PRESSURE_VESSEL_SHARE_HOME", "0");
    }
    protonProcess.setProcessEnvironment(env);
    protonProcess.setProcessChannelMode(QProcess::MergedChannels);

    //Proton always uses SteamRuntime 2.0 or 3.0 so we don't have to modify anything
    protonProcess.setProgram(QString::fromStdString(
        SteamFinder::findProtonPath(cfg.getProtonVersion())));
    QStringList args;
    args << "run" << QString::fromStdString(gamePath);
    protonProcess.setArguments(args);
    protonProcess.startDetached();

    //TODO:
    //      allow using Proton not from steam by supplying proton path (in app or through env)
}

} // namespace LigmaCore