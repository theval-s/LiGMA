## LiGMA

Linux Game Modding Application
Work In Progress

# Dependencies

CMake  
Qt6 Core, Qt6 Widgets  (qt6-base or qt6-base-devel?)  
fuse-overlayfs (and fuse3)  
Steam (proton binaries and prefixes are used from steam dir)

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

- game - fuse-overlayfs of lowerdir = original game folder, upperdir = mod_files/gameroot/*/
- mod_files - folder that contains subfolders for each mod
- .work_game - overlayfs workdir for game folder  

In case your game also uses Proton - overlayed version of Steam's compat prefix is created, so following dirs are also created:

- prefix - fuse-overlayfs of lowerdir = compatdata for that game, upperdir = mod_files/prefix/*/
    
# Plan

- Basic mod managing 
    - Putting mods together/changing order +-
    - Creating overlayfs +
    - Launching +-
    - Removing mods +- (disabling specific mods not implemented yet)
    - Persistency between launches +- 
        -Qt Json object since we already use Qt for plugins and UI
        -Scanning directory for changes?
- Plugins for different games
    - Plugin API +-
    - Plugin integration +-
    - Making a couple of test plugins +-
- UI +-
- Properly documenting all this stuff
    - Doxygen
    - General comments
- Advanced mod managing
    - Dealing with specific mod paths (using plugins for getting needed file structure) 
    (making an algorithm for working with specific mod structure, probably that would require moving some
    of the functions to plugins)
      - current solution: prompt user where to put mod and check mod for it's existing file structure based on mod path list
      - though some games like Fallout 4 require listing used mods in .ini files, so that's where plugins should do it probably
    - Issues that might occur with specific games (proton/wine support)
    - Dearchiving
        - KDE KArchive?
        - libarchive?
- Tests?
- Additional security features
    - AppArmor profiles?
    - Using a different user? but it might disrupt the games
    - Bubblewrap?
- Logging, error handling, exceptions
    - Some functions have exception throws in critical points, but I don't catch them...
    - Checking if mount point already exists? maybe stat has something about it
- Additional features
    - Multithreading?
    - Downloading? 

# What else?

- Study performance overhead of userspace filesystems
- Using other build systems for speed? Ninja/meson?
- Using smart pointers?

# Current issues
- There might be some issues with Flatpak installed Steam (not tested)
- I was thinking of making it also CLI compilable with CLI not being dependent on Qt at all, using dlfcn for plugin handling
(that's why everything is on std::strings and std::filesystem) but QtPlugins is a cool thing and since I was planning on Qt UI 
I decided to use QtPlugins in core and also QtJson handling, so now there are a lot of conversions between std::string and QString
 and that's probably a big issue that would need some rewriting
- I probably need to study proper OOP practices and make interfaces for classes? solid and etc  
- Plugin specific issues:
  - Stardew: looks like XNA games are absolutely incompatible with overlayfs on Linux, and SMAPI is incompatible too.

# Contribution
Currently not accepting any contributions.