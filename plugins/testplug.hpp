#include "ligma_plugin.hpp"

#include <QObject>
#include <QtPlugin>
#include <qtmetamacros.h>

class LigmaTest : public QObject, public LigmaPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ligma.PluginInterface" FILE "testplug.json")
    Q_INTERFACES(LigmaPlugin)
  public:
    QString pluginUUID() const override;
    QString gameName() const override;
    int gameID() const override;
    QString executableName() const override;
    bool usesProton() const override;
    std::vector<QString> environmentVariables() const override;
    std::vector<QString> modPaths() const override;
    void initializeModState(std::filesystem::path modsDirPath) const override;
    QString initialMessage() const override;
};
