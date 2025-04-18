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

    MainWindow w;
    w.setWindowTitle(QString("Ligma App"));

    try {
        w.show();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
    }

    return a.exec();
}
