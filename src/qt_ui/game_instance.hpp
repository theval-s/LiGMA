#pragma once

#include "native_instance_filesystem.hpp"

#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>
#include <memory>

namespace Ui {
class GameInstance;
}

class ModTableModel : public QStandardItemModel {
    Q_OBJECT
  public:
    explicit ModTableModel(QObject *parent = nullptr)
        : QStandardItemModel(parent) {}

    Qt::DropActions supportedDropActions() const override {
        return Qt::MoveAction;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        if (!index.isValid()) return Qt::ItemIsDropEnabled;
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable;
    }
    //drag&drop
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                      int column, const QModelIndex &parent) override;

  signals:
    void modOrderSwapped(int source, int dest);
};

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
    void refreshUI();
    void selectionCheck(const QItemSelection &selected,
                        const QItemSelection &deselected);
    void checkedItemChanged(QStandardItem *item);
    void swapMods(int source, int dest);

    // Auto MOC slots
    void on_addModButton_clicked();
    void on_removeModButton_clicked();
    void on_mountGameButton_clicked();
    void on_settingsButton_clicked();
    // void on_unmountGameButton_clicked();
    void on_runGameButton_clicked();

    void on_unmountGameButton_clicked();

  private:
    Ui::GameInstance *ui;
    //LigmaCore::NativeInstanceFilesystem instance;
    std::unique_ptr<LigmaCore::IInstanceFilesystem> instance;
    std::unique_ptr<ModTableModel> modTableModel;
    //ModTableModel modTableModel;

    void setupUi();

    void setupModTable();

    void refreshModList();
};
