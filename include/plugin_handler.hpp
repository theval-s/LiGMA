#pragma once
#include "ligma_plugin.hpp"

#include <QPluginLoader>
#include <memory>
#include <unordered_map>

namespace LigmaCore {

// TODO:
//   think about handling images with plugins?
struct PluginInfo {
    // std::string is probably better than fs::path for that
    QString name;
    std::string path;
    QString uuid;
};
class PluginHandler {
  private:
    std::vector<PluginInfo> plugInfoVec;
    std::unordered_map<QString, std::string> plugPathByUUID;
    std::unordered_map<LigmaPlugin *, std::unique_ptr<QPluginLoader>>
        plugLoaderByInterface;

    void findPlugins(const std::filesystem::path &pluginsDir);

    PluginHandler();

  public:
    // Singleton?
    PluginHandler(PluginHandler const &) = delete;
    void operator=(PluginHandler const &) = delete;

    static PluginHandler &getInstance() {
        static PluginHandler instance;
        return instance;
    }

    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
    getPluginByUUID(const std::string &);
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
    getPluginByUUID(const QString &);
    std::vector<PluginInfo> getPluginInfo() { return plugInfoVec; }
    void unloadPlugin(LigmaPlugin *);
};

} // namespace LigmaCore
