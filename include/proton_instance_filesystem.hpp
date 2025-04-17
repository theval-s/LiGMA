//
// Created by val on 3/10/25.
//

#pragma once
#include "i_instance_filesystem.hpp"

#include <QJsonObject>
#include <filesystem>
#include <memory>

namespace LigmaCore {

class ProtonInstanceFilesystem final : public BaseInstanceFilesystem {
  public:
    ProtonInstanceFilesystem() = delete;
    ProtonInstanceFilesystem(
        const QString &instanceName, const fs::path &basePath,
        const fs::path &gamePath,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin);
    ProtonInstanceFilesystem(
        const QJsonObject &config, const fs::path &pathToConfig,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin);
    ~ProtonInstanceFilesystem() override = default;

    void mountGameFilesystem() override;
    void unmountGameFilesystem() override;
    void addMod(const fs::path &modPath, const QString &modName,
                const QString &destPathString) override;
    void runGame() override;
    //[[nodiscard]] QJsonObject toJson() const override;
};

} // namespace LigmaCore
