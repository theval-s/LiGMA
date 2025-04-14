#pragma once

#include "config.hpp"
#include "i_instance_filesystem.hpp"
#include "ligma_plugin.hpp"
#include "plugin_handler.hpp"

#include <filesystem>

namespace LigmaCore {

class NativeInstanceFilesystem : public BaseInstanceFilesystem {
  public:
    NativeInstanceFilesystem() = delete;
    NativeInstanceFilesystem(
        const QString &instanceName, const fs::path &basePath,
        const fs::path &gamePath,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin);
    NativeInstanceFilesystem(
        const QJsonObject &config, const fs::path &pathToConfig,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin);
    ~NativeInstanceFilesystem() override {
        // std::cerr << "InstanceFilesystem deconstructed\n";
        // if (m_mounted) NativeInstanceFilesystem::unmountGameFilesystem();
    }

    void mountGameFilesystem() override;
    void unmountGameFilesystem() override;
    void addMod(const fs::path &modPath, const QString &modName,
                const QString &destPathString) override;
    void runGame() override;
};
} // namespace LigmaCore
