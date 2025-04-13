#include "mainwindow.hpp"

#include "game_instance.hpp"
#include "i_instance_filesystem.hpp"
#include "new_instance_dialog.hpp"
#include "plugin_handler.hpp"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QThread>
#include <iostream>
#include <qdebug.h>

void MainWindow::RefreshLists() {
    std::vector<LigmaCore::PluginInfo> info =
        LigmaCore::PluginHandler::getInstance().getPluginInfo();
    for (const auto &p : info) {
        ui->pluginsList->addItem(p.name);
    }
    for (const auto &s : LigmaCore::ConfigLoader::getSavedInstanceNames()) {
        ui->instanceList->addItem(QString::fromStdString(s));
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->createInstanceButton->setDisabled(true);
    ui->openInstanceButton->setDisabled(true);
    RefreshLists();

    connect(ui->pluginsList->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &MainWindow::pluginSelectionCheck);
    connect(ui->instanceList->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &MainWindow::instanceSelectionCheck);
    ui->statusbar->addWidget(new QLabel("Ligma alpha 0.001"));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::pluginSelectionCheck(const QItemSelection &selected,
                                      const QItemSelection &deselected) {
    if (selected.isEmpty())
        ui->createInstanceButton->setDisabled(true);
    else
        ui->createInstanceButton->setDisabled(false);
}

void MainWindow::instanceSelectionCheck(const QItemSelection &selected,
                                        const QItemSelection &deselected) {
    if (selected.isEmpty())
        ui->openInstanceButton->setDisabled(true);
    else
        ui->openInstanceButton->setDisabled(false);
}

void MainWindow::on_createInstanceButton_clicked() {
    try {
        if (const auto current_selected = ui->pluginsList->currentItem()) {
            int plugin_index = ui->pluginsList->row(current_selected);
            std::cerr << "I am in selected plugin with index " << plugin_index
                      << std::endl;
            std::vector<LigmaCore::PluginInfo> info =
                LigmaCore::PluginHandler::getInstance().getPluginInfo();
            // maybe i should store it?
            NewInstanceDialog dialog(info[plugin_index], this);
            dialog.setWindowTitle(QString::fromStdString(
                std::format("Creating instance for: {}",
                            info[plugin_index].name.toStdString())));
            if (dialog.exec() == QDialog::Accepted) {
                std::cerr << "QDialog Accepted!\n";
                qDebug() << "Instance name: " << dialog.getInstanceName()
                         << "\nGame Path: " << dialog.getGamePath()
                         << "\nInstance path: " << dialog.getInstancePath()
                         << "\n";
                // create game instance;

                auto plug =
                    LigmaCore::PluginHandler::getInstance().getPluginByUUID(
                        info[plugin_index].uuid);
                QMessageBox::information(this, "Information from plugin",
                                         plug->initialMessage());
                GameInstance *instance = new GameInstance(
                    dialog.getInstanceName(), dialog.getInstancePath(),
                    dialog.getGamePath(), std::move(plug));
                instance->setAttribute(Qt::WA_DeleteOnClose);
                instance->setWindowTitle("Ligma Game Instance");
                instance->show();

                // QObject().thread()->usleep(1000*1000*5);
                this->close();
            }
        }
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void MainWindow::on_openInstanceButton_clicked() {
    // error handling?
    try {
        if (const auto current_selected = ui->instanceList->currentItem()) {
            //int plugin_index = ui->instanceList->row(current_selected);
            std::filesystem::path conf_path =
                LigmaCore::ConfigLoader::getConfigPath() /
                current_selected->text().toStdString();
            const QJsonObject &conf =
                (LigmaCore::ConfigLoader::loadInstance(conf_path));
            QString plug_uuid = conf["pluginUUID"].toString();
            if (plug_uuid.isEmpty()) {
                QMessageBox::warning(
                    this, "Error",
                    "This configuration file does not have plugin uuid! Can't "
                    "open instance with it!");
                return;
            }
            auto plug = LigmaCore::PluginHandler::getInstance().getPluginByUUID(
                plug_uuid);
            auto *instance = new GameInstance(conf, conf_path, std::move(plug));
            instance->show();
            this->close();
        }
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}
