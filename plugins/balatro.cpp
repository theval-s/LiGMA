#include "ligma_plugin.hpp"

#include <filesystem>
#include <qtmetamacros.h>

class LigmaBalatro final : public QObject, public LigmaPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ligma.PluginInterface" FILE "ligmabalatro.json")
    Q_INTERFACES(LigmaPlugin)
  public:
    QString pluginUUID() const override {
        return "0cd4e6e6-b420-4ab1-a488-d41b68aeed0d";
    }
    QString gameName() const override { return "Balatro (Steam Proton)"; }
    int gameID() const override { return 2379780; }
    QString executableName() const override { return "Balatro.exe"; }
    bool usesProton() const override { return true; }
    std::vector<QString> environmentVariables() const override {
        return {"WINEDLLOVERRIDES=\"version=n,b\""};
    }
    std::vector<QString> modPaths() const override {
        return {
            "$PREFIX/drive_c/users/steamuser/AppData/Roaming/Balatro/Mods/"};
    }
    void initializeModState(std::filesystem::path modsDirPath) const override {
        return;
    }
    QString initialMessage() const override {
        return "Welcome to Balatro modding! Install lovely injector in game "
               "root "
               "and Steamodded into prefix and you're ready to install mods "
               "(into prefix)";
    }
};

#include "balatro.moc"