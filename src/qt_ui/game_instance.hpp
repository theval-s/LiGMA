#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include <memory>
#include <QItemSelection>
#include "instance_filesystem.hpp"

namespace Ui {
    class GameInstance;
}

class GameInstance : public QMainWindow {
    Q_OBJECT

public:
    //2 constructors - one from json and one from data
    //or leave 1 from InstanceManager?
    explicit GameInstance(LigmaCore::InstanceFilesystem instance, QMainWindow *parent = nullptr);

    explicit GameInstance(const QJsonObject &config, const std::filesystem::path &configPath,
                          QMainWindow *parent = nullptr);

    explicit GameInstance(const QString &name, const QString &basePath,
                          const QString &gamePath, LigmaPlugin *gamePlugin,
                          QMainWindow *parent = nullptr);

    ~GameInstance();

private slots:
    void validateInputs();

    void selectionCheck(const QItemSelection &selected, const QItemSelection &deselected);

    void on_addModButton_clicked();

    void on_modPathBrowseButton_clicked();

    void on_runGameButton_clicked();

    void on_removeModButton_clicked();

private:
    Ui::GameInstance *ui;
    LigmaCore::InstanceFilesystem m_instance;
    std::unique_ptr<QStandardItemModel> m_modTableModel;

    void setupUi();

    void setupModTable();

    void refreshModList();
};

