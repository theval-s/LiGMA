#pragma once
#include <unordered_map>
#include <QPluginLoader>
//#include <memory>
#include "ligma_plugin.hpp"

namespace LigmaCore{

//TODO:
//  think about handling images with plugins?
struct PluginInfo{
    //std::string is probably better than fs::path for that
    std::string name;
    std::string path;
    std::string uuid;
};
class PluginHandler{
private:
    std::vector<PluginInfo> m_plugInfoVec;
    std::unordered_map<std::string, std::string> m_plugPathByUUID;
    std::unordered_map<LigmaPlugin*, QPluginLoader> m_plugLoaderByInterface;

    void findPlugins(const std::filesystem::path &pluginsDir);

    PluginHandler();
    //I should probably use unique_ptr for that

public:
    //Singleton?
    PluginHandler(PluginHandler const&) = delete;
    void operator=(PluginHandler const&) = delete;

    static PluginHandler& getInstance(){
        static PluginHandler instance;
        return instance;
    }
    //use unique_ptr instead of raw pointer?
    LigmaPlugin* getPluginByUUID(const std::string&);
    std::vector<PluginInfo> getPluginInfo(){ return m_plugInfoVec; }
    void unloadPlugin(LigmaPlugin*);
};



}
