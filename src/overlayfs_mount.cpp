#include "overlayfs_mount.hpp"

#include <QProcess>
#include <QStringList>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <linux/magic.h>
#include <sys/statfs.h>

namespace LigmaCore {

void FuseOverlayFSMount::mount(const fs::path &mountPath,
                               const std::string &lowerDirs,
                               const std::string &upperDir,
                               const std::string &workDir) {
    mount(mountPath, QString::fromStdString(lowerDirs),
          QString::fromStdString(upperDir), QString::fromStdString(workDir));
}
void FuseOverlayFSMount::mount(const fs::path &mountPath,
                               const QString &lowerDirs,
                               const QString &upperDir,
                               const QString &workdir) {
    // TODO: check if dirs exist, error handling, etc etc
    //       maybe use absolute paths instead of setting working directory

    // there might be some issues if paths are absolute/are not absolute
    // since I assume mountPath&lowerDirs are absolute but upperDir & workDir
    // are not
    QProcess process;
    process.setWorkingDirectory(
        QString::fromStdString(mountPath.parent_path()));
    process.setProcessChannelMode(
        QProcess::ForwardedChannels);
    QStringList arguments;
    arguments << "-o" << QString("lowerdir=%1").arg(lowerDirs) << "-o"
              << QString("upperdir=%1").arg(upperDir) << "-o"
              << QString("workdir=%1").arg(workdir)
              << QString::fromStdString(mountPath);
    // process.setArguments(arguments);

    process.start("fuse-overlayfs", arguments);

    // this should be instant, so I lower default 30secs to 0.5
    if (!process.waitForFinished(500) || process.exitCode() != 0) {
        // fuse-overlayfs prints out errors, and we forwarded stdout, so we will
        // give what we executed but yeah that would be hard to parse the actual
        // error into UI if it timeouted there is something wrong so we will
        // still give what we executed

        //TODO: add stdout into the exception text?
        throw(std::runtime_error(std::format(
            "FuseOverlayFSMount::mount() process timeout, command: {}\n",
            (process.program() + " " + process.arguments().join(" "))
                .toStdString())));
    }
}

void FuseOverlayFSMount::unmount(const fs::path &mountPath) {

    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList arguments;
    arguments << "-u" << QString::fromStdString(mountPath);

    process.start("fusermount3", arguments);

    // this should be instant, so I lower default 30secs to .5
    if (!process.waitForFinished(500) || process.exitCode() != 0) {
        // if it timeouted there is something wrong so we will still give what
        // we executed
        throw(std::runtime_error(std::format(
            "FuseOverlayFSMount::unmount() process timeout, command: {}\n",
            (process.program() + " " + process.arguments().join(" "))
                .toStdString())));
    }
}

bool FuseOverlayFSMount::isMounted(const fs::path &dirPath) {
    struct statfs st;
    if (statfs((dirPath).c_str(), &st) == 0) {
        std::cout << std::format(
                         "Checking FS for {0}: {1} (hex: {1:x}) (fuse? - {2})",
                         (dirPath).string(), st.f_type,
                         st.f_type == FUSE_SUPER_MAGIC)
                  << std::endl;
        if (st.f_type != FUSE_SUPER_MAGIC) {
            return false;
        } else
            return true;
    } else {
        throw(std::runtime_error(std::format(
            "FuseOverlayFSMount::isMounted(): statfs() failed (errno: {}",
            std::strerror(errno))));
    }
    //TODO: Check for stale mounts?
}
} // namespace LigmaCore