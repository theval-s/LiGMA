#include "ligma_plugin.hpp"
#include <QObject>
#include <QtPlugin>
#include <qt6/QtCore/qtmetamacros.h>

class LigmaTest : public QObject, public LigmaPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ligma.PluginInterface" FILE "testplug.json")
    Q_INTERFACES(LigmaPlugin)
public:
    std::string pluginUUID() const override;
    std::string gameName() const override;
    int gameID() const override;
    std::string executeCommand() const override;
    std::string modPath() const override;
    void initializeModState(std::filesystem::path modsDirPath) const override;

};
