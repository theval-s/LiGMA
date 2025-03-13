#include "plugin_handler.hpp"

#include <QJsonObject>
#include <QObject>
#include <QPluginLoader>
#include <filesystem>
#include <iostream>
#include <memory>

namespace LigmaCore {
void PluginHandler::findPlugins(const std::filesystem::path &pluginsDir) {
    try {
        for (const auto &file :
             std::filesystem::directory_iterator(pluginsDir)) {
            if (file.is_regular_file() && file.path().extension() == ".so") {
                std::cerr << std::format("current file: {}\n",
                                         file.path().string());
                auto loader = QPluginLoader(QString(file.path().c_str()));
                QJsonObject plug_metadata = loader.metaData();
                if (!plug_metadata["MetaData"].toObject().contains("name") ||
                    !plug_metadata["MetaData"].toObject().contains("uuid")) {
                    std::cerr << std::format(
                        "Error with plugin at path: {}\nMetadata "
                        "does not contain UUID and/or name\n",
                        file.path().string());
                } else {
                    PluginInfo plug = {
                        plug_metadata["MetaData"].toObject()["name"].toString(),
                        file.path().string(),
                        plug_metadata["MetaData"]
                            .toObject()["uuid"]
                            .toString()};
                    if (plug.uuid.isEmpty() || plug.name.isEmpty()) {
                        std::cerr
                            << std::format("PluginHandler::findPlugins: "
                                           "error with plugin {}\n",
                                           file.path().filename().string());
                        continue;
                    }
                    plugInfoVec.emplace_back(plug);
                    plugPathByUUID.emplace(plug.uuid, plug.path);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "plugin_handler.cpp: directory iterator exception: "
                  << e.what() << std::endl;
    }
}

PluginHandler::PluginHandler() {
    // TODO: more error handling and etc
    std::filesystem::path plugins_dir =
        std::filesystem::canonical("/proc/self/exe").parent_path() / "plugins";
    findPlugins(plugins_dir);
}

std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
PluginHandler::getPluginByUUID(const std::string &uuid) {
    return std::move(getPluginByUUID(QString::fromStdString(uuid)));
}
std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
PluginHandler::getPluginByUUID(const QString &uuid) {
    if (plugPathByUUID.contains(uuid)) {
        const std::string path = plugPathByUUID[uuid];
        auto loader =
            std::make_unique<QPluginLoader>(QString::fromStdString(path));
        if (QObject *plug = loader->instance()) {
            std::cerr << loader->isLoaded() << " "
                      << plug->metaObject()->className() << std::endl;
            auto plugin_interface = qobject_cast<LigmaPlugin *>(plug);
            std::cerr << std::format(
                "trying to invoke plugin functions:\ngameName(): {}\n gameID():"
                "{}\n",
                plugin_interface->gameName().toStdString(),
                plugin_interface->gameID());

            auto pluginUnloader = [this](LigmaPlugin *plugin) {
                unloadPlugin(plugin);
            };
            plugLoaderByInterface[plugin_interface] = std::move(loader);
            return std::unique_ptr<LigmaPlugin, decltype(pluginUnloader)>(
                plugin_interface, pluginUnloader);
            // not properly tested
        } else
            throw std::runtime_error(
                std::format("error in plugin loading, error {}\n",
                            loader->errorString().toStdString()));
    } else {
        throw std::runtime_error(std::format(
            "Failed to find plugin by uuid ({})\n", uuid.toStdString()));
    }
}

void PluginHandler::unloadPlugin(LigmaPlugin *plugInterface) {
    if (plugLoaderByInterface.contains(plugInterface)) {
        plugLoaderByInterface[plugInterface]->unload();
        plugLoaderByInterface.erase(plugInterface);
    } else {
        // throw std::runtime_error(std::format("Failed to find stored plugin
        // interface (plugin uuid:{}\n",
        //                                      plugInterface->pluginUUID()));
        std::cerr << std::format(
            "Failed to find stored plugin interface (plugin uuid:{}\n",
            plugInterface->pluginUUID().toStdString());
    }
}
} // namespace LigmaCore
