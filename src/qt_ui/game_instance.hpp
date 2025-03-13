#pragma once

#include "native_instance_filesystem.hpp"

#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>
#include <memory>

namespace Ui {
class GameInstance;
}

class GameInstance : public QMainWindow {
    Q_OBJECT

  public:
    // 2 constructors - one from json and one from data
    // or leave 1 from InstanceManager?
    explicit GameInstance(
        std::unique_ptr<LigmaCore::IInstanceFilesystem> instance,
        QMainWindow *parent = nullptr);

    explicit GameInstance(
        const QJsonObject &config, const std::filesystem::path &configPath,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin,
        QMainWindow *parent = nullptr);

    explicit GameInstance(
        const QString &name, const QString &basePath, const QString &gamePath,
        std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>>
            gamePlugin,
        QMainWindow *parent = nullptr);

    ~GameInstance() override;

  private slots:
    // connectable slots
    void validateInputs();
    void refreshUI();
    void selectionCheck(const QItemSelection &selected,
                        const QItemSelection &deselected);

    // Auto MOC slots
    void on_addModButton_clicked();
    void on_modPathBrowseButton_clicked();
    void on_removeModButton_clicked();
    void on_mountGameButton_clicked();
    // void on_unmountGameButton_clicked();
    void on_runGameButton_clicked();

  private:
    Ui::GameInstance *ui;
    //LigmaCore::NativeInstanceFilesystem instance;
    std::unique_ptr<LigmaCore::IInstanceFilesystem> instance;
    std::unique_ptr<QStandardItemModel> modTableModel;

    void setupUi();

    void setupModTable();

    void refreshModList();
};
