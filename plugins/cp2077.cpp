#include "ligma_plugin.hpp"

#include <iostream>
#include <qtmetamacros.h>

class LigmaStardew final : public QObject, public LigmaPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ligma.PluginInterface" FILE "ligmacp2077.json")
    Q_INTERFACES(LigmaPlugin)
  public:
    QString pluginUUID() const override {
        return "9efda10d-36c5-4381-a8e1-7f9c34ccd0f1";
    }
    QString gameName() const override {
        return "Cyberpunk 2077 (Proton)";
    }
    int gameID() const override {
        return 1091500;
        // Stardew Valley ID in Steam
    };
    QString executableName() const override { return "bin/x64/Cyberpunk2077.exe"; }
    bool usesProton() const override { return true; }
    std::vector<QString> environmentVariables() const override { return {}; }

    //All of the mods are in subdirectories of game folder.
    std::vector<QString> modPaths() const override { return {}; }
    void initializeModState(std::filesystem::path modsDirPath) const override;
    QString initialMessage() const override {
        return "Welcome to Cyberpunk 2077 modding through LiGMA!";
    }
};

void LigmaStardew::initializeModState(std::filesystem::path modsDirPath) const {
    return;
}

// You either split plugin in header and source file and then auto moc works, or
// you make it in one source file and add this in the end so moc knows
#include "cp2077.moc"