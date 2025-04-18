#include "game_instance.hpp"

#include "add_mod_dialog.hpp"
#include "instance_options.hpp"
#include "ui_game_instance.h"

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QTimer>
#include <instance_factory.hpp>
#include <memory>
#include <qstandarditemmodel.h>
#include <ui_add_mod_dialog.h>

bool ModTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column,
                                 const QModelIndex &parent) {
    // TODO: figure out how to pinpoint original row index?
    if (action != Qt::MoveAction) {
        return false;
    }

    if (row == -1) row = rowCount();
    bool result =
        QStandardItemModel::dropMimeData(data, action, row, 0, parent);
    if (result) {
        //qDebug() << "dest row: " << row;
        QByteArray encoded = data->data("application/x-qstandarditemmodeldatalist");
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        int sourceRow = -1;
        stream >> sourceRow;
        if (sourceRow < 0) throw std::runtime_error("ModTableModel::dropMimeData: Failed to read source row");
        //qDebug() << "sourceRow: " << sourceRow;

        //emit modOrderSwapped();
        QTimer::singleShot(0, this, [this, sourceRow, row]() { emit modOrderSwapped(sourceRow, row); });
    }
    return result;
}
GameInstance::GameInstance(
    std::unique_ptr<LigmaCore::IInstanceFilesystem> instance,
    QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      instance(std::move(instance)),
      modTableModel(std::make_unique<ModTableModel>(this)) {
    setupUi();
}

GameInstance::GameInstance(
    const QJsonObject &config, const std::filesystem::path &configPath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin,
    QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      instance(std::move(LigmaCore::createInstanceFilesystem(
          config, configPath, std::move(gamePlugin)))),
      modTableModel(std::make_unique<ModTableModel>(this)) {

    setupUi();
}

GameInstance::GameInstance(
    const QString &name, const QString &basePath, const QString &gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin,
    QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      instance(std::move(LigmaCore::createInstanceFilesystem(
          name, basePath, gamePath, std::move(gamePlugin)))),
      modTableModel(std::make_unique<ModTableModel>(this)) {
    setupUi();
}

GameInstance::~GameInstance() {
    delete ui;
}

void GameInstance::setupModTable() {
    QStringList headers;
    headers << "Mod Name" << "Enabled" << "Mod Path";
    modTableModel->setHorizontalHeaderLabels(headers);
    ui->modTableView->setModel(modTableModel.get());

    ui->modTableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Interactive);
    //ui->modTableView->horizontalHeader()->setStretchLastSection(true);
}

void GameInstance::refreshModList() {
    // i hope removing rows frees the memory
    modTableModel->removeRows(0, modTableModel->rowCount());

    for (const auto &[modName, modPath, state, type] : instance->getModList()) {
        QList<QStandardItem *> row;
        // appendRow() asks for pointers so yeah
        auto name_item = new QStandardItem(modName);
        auto enabled_item = new QStandardItem();
        auto path_item =
            new QStandardItem(QString::fromStdString(modPath.string()));
        enabled_item->setCheckable(true);
        enabled_item->setCheckState(state ? Qt::Checked : Qt::Unchecked);
        row << name_item << enabled_item << path_item;
        modTableModel->appendRow(row);
    }
}

void GameInstance::swapMods(int source, int dest) {
    if (dest == modTableModel->rowCount()) {
        instance->putToBack(source);
    } else try {
        instance->swapMods(source, dest);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void GameInstance::refreshUI() {
    refreshModList();
    if (instance->isMounted()) {
        ui->mountGameButton->setEnabled(false);
        ui->unmountGameButton->setEnabled(true);
    } else {
        ui->mountGameButton->setEnabled(true);
        ui->unmountGameButton->setEnabled(false);
    }
}

void GameInstance::selectionCheck(const QItemSelection &selected,
                                  const QItemSelection &deselected) {
    // std::cout << "In selection check!\n";
    if (selected.isEmpty())
        ui->removeModButton->setDisabled(true);
    else
        ui->removeModButton->setEnabled(true);
}
void GameInstance::checkedItemChanged(QStandardItem *item) {
    if (!item->isCheckable()) return;
    instance->setEnabled(item->row(), item->checkState() == Qt::Checked);
}

void GameInstance::on_addModButton_clicked() {
    QString destPath;
    AddModDialog dialog = AddModDialog(instance->getModPaths(), this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            instance->addMod(dialog.getModPath(),
                             dialog.getModName(), dialog.getDestinationPath());
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", e.what());
        }
        refreshUI();
    }
}

void GameInstance::on_runGameButton_clicked() {
    try {
        instance->runGame();
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error!", e.what());
    }
    refreshUI();
}

void GameInstance::on_removeModButton_clicked() {
    auto index = ui->modTableView->currentIndex();
    instance->removeMod(index.row());
    refreshUI();
}

void GameInstance::on_mountGameButton_clicked() {
    try {
        instance->mountGameFilesystem();
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error!", e.what());
    }
}
void GameInstance::on_settingsButton_clicked() {
    InstanceOptions settings(instance->getUserConfigRef(),
                             instance->isUsingProton(), this);
    settings.setModal(true);
    settings.exec();

    //Runs after dialog has finished;
    if (settings.changed) {
        instance->saveState();
    }
}

void GameInstance::setupUi() {

    ui->setupUi(this);
    setupModTable();
    refreshUI();

    // ui->modTableView->dropEvent(ui->modTableView->moveEvent());
    connect(ui->refreshUiButton, &QPushButton::clicked, this,
            &GameInstance::refreshUI);
    connect(ui->modTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &GameInstance::selectionCheck);
    connect(modTableModel.get(), &ModTableModel::modOrderSwapped, this,
            &GameInstance::swapMods);
    connect(modTableModel.get(), &QStandardItemModel::itemChanged, this,
        &GameInstance::checkedItemChanged);
    ui->removeModButton->setDisabled(true);
    this->setWindowTitle(instance->getInstanceName() + ": LiGMA Instance");
}

void GameInstance::on_unmountGameButton_clicked() {
    instance->unmountGameFilesystem();
    refreshUI();
}

