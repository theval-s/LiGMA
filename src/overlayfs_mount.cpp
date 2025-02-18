#include "overlayfs_mount.hpp"
#include <QProcess>
#include <cstdlib>
#include <iostream>

namespace LigmaCore{

void FuseOverlayFSMount::mount(const fs::path &mountPath, const std::string &lowerDirs,
const std::string &upperDir, const std::string workdir) {
    const fs::path old_work_dir = fs::current_path();
    fs::current_path;
}

 void FuseOverlayFSMount::unmount(const fs::path &mountPath) {
    std::string command =
               std::format("fusermount3 -u {}", mountPath.string());
    int return_code = system(command.c_str());
    std::cerr << format("system({}) executed with retcode={}\n", command,
                     return_code);

}

}//namespace LigmaCore