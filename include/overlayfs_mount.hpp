#pragma once

#include <QString>
#include <filesystem>
#include <string>

namespace LigmaCore {
namespace fs = std::filesystem;

namespace FuseOverlayFSMount {
/// @brief Mounts fuse-overlayfs with given arguments (paths must be
/// absolute or in current dir)
/// @param mountPath - path to resulting merged directory
/// @param lowerDirs - string of lower directories in format
/// dir1:dir2:...:dirn
/// @param upperDir - upper directory which would have all the changes made
/// to merged dir (must be writable)
/// @param workDir - work directory for overlayfs
void mount(const fs::path &mountPath, const std::string &lowerDirs,
           const std::string &upperDir, const std::string &workDir);
void mount(const fs::path &mountPath, const QString &lowerDirs,
           const QString &upperDir, const QString &workDir);
/// @brief fusermount3 -u unmounts specified path
/// @param mountPath Directory to unmount
void unmount(const fs::path &mountPath);
bool isMounted(const fs::path &dirPath);
} // namespace FuseOverlayFSMount

} // namespace LigmaCore
