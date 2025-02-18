#include "plugin_handler.hpp"
#include <QJsonObject>
#include <QPluginLoader>
#include <QObject>
#include <filesystem>
#include <iostream>

namespace LigmaCore {
    void PluginHandler::findPlugins(const std::filesystem::path &pluginsDir) {
        try {
            for (const auto &file: std::filesystem::directory_iterator(pluginsDir)) {
                if (file.is_regular_file() && file.path().extension() == ".so") {
                    std::cerr << std::format("current file: {}\n", file.path().string());
                    QPluginLoader loader = QPluginLoader(QString(file.path().c_str()));
                    QJsonObject plug_metadata = loader.metaData();
                    if (!plug_metadata["MetaData"].toObject().contains("name") ||
                        !plug_metadata["MetaData"].toObject().contains("uuid")) {
                        std::cerr << std::format("Error with plugin at path: {}\nMetadata "
                                                 "does not contain UUID and/or name\n",
                                                 file.path().string());
                    } else {
                        PluginInfo plug = {
                            plug_metadata["MetaData"].toObject()["name"].toString().toStdString(),
                            file.path().string(),
                            plug_metadata["MetaData"].toObject()["uuid"].toString().toStdString()
                        };
                        m_plugInfoVec.emplace_back(plug);
                        //!!! check for null in value of name or uuid
                        m_plugPathByUUID.emplace(plug.uuid, plug.path);
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "plugin_handler.cpp: directory iterator exception: " << e.what()
                    << std::endl;
        }
    }

    LigmaCore::PluginHandler::PluginHandler() {
        // TODO: more error handling and etc
        std::filesystem::path plugins_dir =
                std::filesystem::canonical("/proc/self/exe").parent_path() / "plugins";
        findPlugins(plugins_dir);
    }

    LigmaPlugin *PluginHandler::getPluginByUUID(const std::string &uuid) {
        auto iter = m_plugPathByUUID.find(uuid);
        if (iter != m_plugPathByUUID.end()) {
            QPluginLoader loader = QPluginLoader(QString::fromStdString(iter->second));
            QObject *plug = loader.instance();
            if (plug) {
                std::cerr << loader.isLoaded() << " "
                        << plug->metaObject()->className() << std::endl;
                LigmaPlugin *plugin_interface = qobject_cast<LigmaPlugin *>(plug);
                std::cerr << std::format(
                    "trying to invoke plugin functions:\ngameName(): {}\n gameID():"
                    "{}\n",
                    plugin_interface->gameName(),
                    plugin_interface->gameID());
                return plugin_interface;
                //currently needs unloading
            } else
                throw std::runtime_error(std::format("error in plugin loading, error {}\n",
                                                     loader.errorString().toStdString()));
        } else {
            throw std::runtime_error(std::format("Failed to find plugin by uuid ({})\n", uuid));
        }
    }

    void PluginHandler::unloadPlugin(LigmaPlugin *plugInterface) {
        auto iter = m_plugLoaderByInterface.find(plugInterface);
        if (iter != m_plugLoaderByInterface.end()) {
            iter->second.unload();
            m_plugLoaderByInterface.erase(iter);
        } else {
            //throw std::runtime_error(std::format("Failed to find stored plugin interface (plugin uuid:{}\n",
            //                                     plugInterface->pluginUUID()));
            std::cerr << std::format("Failed to find stored plugin interface (plugin uuid:{}\n",
                                                 plugInterface->pluginUUID());
        }
    }
} //namespace LigmaCore


