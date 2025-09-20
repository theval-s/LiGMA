## LiGMA

Linux Game Modding Application  
**Work In Progress**

# Dependencies

CMake  
Qt6 Core, Qt6 Widgets  (qt6-base or qt6-base-devel?)  
fuse-overlayfs (and fuse3)  
Steam (proton binaries and prefixes are used from steam dir, only Steam games are supported)

# Building
```
mkdir build && cd build 
cmake .. && make
```

# How it works?

It creates a managed instance of mods for a specific game. Game support is based on plugins, that are supplied in ./plugins folder.
Stores instance info in .jsons in ./config folder.  
Currently Linux native games that rely on modifying something besides the game root directory are not compatible.

By default, instances are created in ./game_instances, a separate folder for each instance, but you can provide custom base path per instance.
Every instance has specific folders:

- game - fuse-overlayfs of lowerdir = original game folder, upperdir = mod_files/*/
- mod_files - folder that contains subfolders for each mod
- .work_game - overlayfs workdir for game folder  

In case your game also uses Proton - overlayed version of Steam's compat prefix is created, so following dirs are also created:

- prefix - fuse-overlayfs of lowerdir = compatdata for that game, upperdir = mod_files/prefix/*/
    
# Plan

- Basic mod managing +
- Plugins for different games +
- UI +-
- Steam Runtime, pressure_vessel home isolation +
- Tests, logs

# Current issues
- There might be some issues with Flatpak installed Steam (not tested)
- A lot of Qt and std:: mixing, I probably will rewrite some parts 
- Plugin specific issues:
  - Stardew: looks like XNA games are absolutely incompatible with overlayfs on Linux, and SMAPI is incompatible too.

# Contribution
Feel free to send a pull request
