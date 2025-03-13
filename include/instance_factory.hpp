//
// Created by val on 3/13/25.
//
#pragma once

#include "i_instance_filesystem.hpp"
#include "native_instance_filesystem.hpp"
#include "proton_instance_filesystem.hpp"

#include <QString>
#include <iostream>

namespace LigmaCore {
inline std::unique_ptr<IInstanceFilesystem> createInstanceFilesystem(
    const QString &name, const QString &basePath, const QString &gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
        gamePlugin) {
    if (gamePlugin->usesProton()) {
        std::cerr << "i_instance_filesystem.cpp createInstanceFilesystem() "
                     "creating ProtonInstanceFilesystem...\n";
        return std::make_unique<ProtonInstanceFilesystem>(
            name, basePath.toStdString(), gamePath.toStdString(),
            std::move(gamePlugin));
    } else {
        std::cerr << "i_instance_filesystem.cpp createInstanceFilesystem() "
                     "creating NativeInstanceFilesystem...\n";
        return std::make_unique<NativeInstanceFilesystem>(
            name, basePath.toStdString(), gamePath.toStdString(),
            std::move(gamePlugin));
    }
}

inline std::unique_ptr<IInstanceFilesystem> createInstanceFilesystem(
    const QJsonObject &config, const std::filesystem::path &pathToConfig,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
        gamePlugin) {
    if (gamePlugin->usesProton()) {
        std::cerr << "i_instance_filesystem.cpp createInstanceFilesystem() "
                     "creating ProtonInstanceFilesystem...\n";
        return std::make_unique<ProtonInstanceFilesystem>(
            config, pathToConfig, std::move(gamePlugin));
    } else {
        std::cerr << "i_instance_filesystem.cpp createInstanceFilesystem() "
                     "creating NativeInstanceFilesystem...\n";
        return std::make_unique<NativeInstanceFilesystem>(
            config, pathToConfig, std::move(gamePlugin));
    }
}
} // namespace LigmaCore