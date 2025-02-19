#include "qt_ui/mainwindow.hpp"

#include <QApplication>
#include <filesystem>
#include <iostream>

// i need to clean this up don't i?...

int main(int argc, char **argv) {
    // system - possible security concerns with dirname command injection?
    // std::vector<std::string> lower_dirs = {"/home/thevals/test/lower_dir"};
    // std::string upper_dir = "/home/thevals/test/upper_dir";
    // std::string merged_dir = "/home/thevals/test/merged";
    // std::string workdir = "/home/thevals/test/workdir";

    // Changing working folder to executable folder
    std::cerr << "Trying to change current path to exe folder\n";
    std::filesystem::current_path(
        std::filesystem::canonical("/proc/self/exe").parent_path());
    if (!std::filesystem::exists("game_instances")) {
        std::cerr << "Creating game_instances directory\n";
        std::filesystem::create_directory("game_instances",
                                          std::filesystem::current_path());
    }
    if (!std::filesystem::exists("config")) {
        std::cerr << "Creating config directory\n";
        std::filesystem::create_directory("config",
                                          std::filesystem::current_path());
    }
    std::cerr << "Launching QApplication\n";
    QApplication a(argc, argv);

    // metadata has UUIDs of plugins
    // auto plug_uuid =
    // plug_metadata[0]["MetaData"].toObject()["uuid"].toString();
    //  std::cout << .toStdString() << std::endl;

    /*
    QJsonObject test_json;
    test_json.insert("instanceName", "instance1");
    test_json.insert("pluginUUID", "plug_uuid");
    test_json.insert("basePath", QString::fromStdString((fs::current_path() /
    ".game_states" / "test_game").string())); test_json.insert("gamePath",
    "/home/val/Desktop/testExe"); test_json.insert("mounted", true);

    ConfigManager::saveInstance(test_json,
    (fs::current_path()/"config"/"test.json"));

    QJsonObject test_load_json =
    ConfigManager::loadInstance(fs::current_path()/"config"/"test.json");
    for(auto iter = test_load_json.begin(); iter != test_load_json.end();
    iter++){ auto value = (iter.value().isBool())?
    (iter.value().toBool()?"true":"false"):
    iter.value().toString().toStdString(); std::cout << iter.key().toStdString()
    << ":" << value << "\n";
    }
    */

    MainWindow w;
    w.setWindowTitle(QString("Ligma App"));
    w.show();

    // ligma_fs::fs::create_directory(".game_states");
    // ligma_fs::fs::create_directory(fs::current_path()/".game_states"/"test_game");
    // ligma_fs::InstanceFilesystem
    // manager(fs::current_path()/".game_states"/"test_game",
    // fs::path("/home/thevals/thisIsAGame"),"program");
    // manager.addMod("/home/thevals/diplom/test_files");
    // manager.mountModFilesystem();
    // manager.openGame();
    // manager.unmount();
    // manager.cleanState();

    return a.exec();
}
