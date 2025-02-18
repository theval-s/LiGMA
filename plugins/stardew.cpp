#include "ligma_plugin.hpp"
#include <qtmetamacros.h>
#include <iostream>

class LigmaStardew : public QObject, public LigmaPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ligma.PluginInterface" FILE "ligmastardew.json")
    Q_INTERFACES(LigmaPlugin)
public:
    std::string pluginUUID() const override{
        return "7734463d-9fea-4d46-a8cc-e8265cbe46b2";
    }
    std::string gameName() const override { return "Stardew Valley (Linux Native)"; }
    int gameID() const override {
        return 413150;
        // Stardew Valley ID in Steam
    };
    std::string executeCommand() const override{ return "StardewValley"; }
    std::string modPath() const override{return "Mods";};
    void initializeModState(std::filesystem::path modsDirPath) const override;

};

void LigmaStardew::initializeModState(std::filesystem::path modsDirPath) const{
    // Download SMAPI -> Install -> ?
    // include QDir or std::filesystem?
    std::cout << "PLUGIN CURRENTLY NOT FULLY IMPLEMENTED TO INSTALL SMAPI AUTOMATICALLY. Install SMAPI to " << modsDirPath << std::endl;
    //if my expectations are too much I will just say that plugins are not supposed to instal basic modding api and things
    //thats such a cool thing for me to do yeah??
    std::filesystem::create_directory(modsDirPath/"Mods");
  return;
}
//You either split plugin in header and source file and then auto moc works, or you make it in
//one source file and add this in the end so moc knows
#include "stardew.moc"