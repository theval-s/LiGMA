//
// Created by val on 4/10/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_instance_options.h" resolved

#include "instance_options.hpp"

#include "ui_instance_options.h"

#include <iostream>

InstanceOptions::InstanceOptions(LigmaCore::UserConfig &cfg,
                                 const bool usesProton, QWidget *parent)
    : QDialog(parent), ui(new Ui::InstanceOptions), cfg(cfg),
      usesProton(usesProton) {
    ui->setupUi(this);
    if (!usesProton) {
        ui->protonVersionComboBox->setDisabled(true);
        ui->useHomeIsolationCheckBox->setDisabled(true);
    } else {
        ui->useHomeIsolationCheckBox->setChecked(cfg.getUseHomeIsolation());
        fillProtonComboBox();
        //Connect after filling to not trigger on initial index change
        connect(ui->protonVersionComboBox, &QComboBox::currentIndexChanged,
                this, &InstanceOptions::protonVersionComboBoxIndexChanged);
    }
    fillEnvironmentVariablesTable();
    fillSteamRuntimeComboBox();
    connect(ui->steamRuntimeComboBox, &QComboBox::currentIndexChanged, this,
            &InstanceOptions::steamRuntimeComboBoxIndexChanged);
}

InstanceOptions::~InstanceOptions() {
    delete ui;
}
void InstanceOptions::fillEnvironmentVariablesTable() {
    auto vars = cfg.getEnvironmentVariables();
    //it's intended to only do initial setup
    if (ui->environmentVariablesTableWidget->rowCount() > 0) {
        return;
    }
    for (int i = 0; i < vars.size(); ++i) {
        ui->environmentVariablesTableWidget->insertRow(i);
        ui->environmentVariablesTableWidget->setItem(
            i, 0, new QTableWidgetItem(vars[i]));
    }
}
void InstanceOptions::fillProtonComboBox() {
    for (const auto &entry : LigmaCore::SteamFinder::protonDirName) {
        ui->protonVersionComboBox->addItem(
            QString::fromStdString(entry.second));
    }
    ui->protonVersionComboBox->setCurrentIndex(cfg.getProtonVersion());
}
void InstanceOptions::fillSteamRuntimeComboBox() {
    ui->steamRuntimeComboBox->addItem("Not using");
    ui->steamRuntimeComboBox->addItem("Scout (1.0)");
    ui->steamRuntimeComboBox->addItem("Sniper (2.0)");
    ui->steamRuntimeComboBox->addItem("Soldier (3.0)");
    ui->steamRuntimeComboBox->setCurrentIndex(cfg.getSteamRuntimeVersion());
}
void InstanceOptions::on_useHomeIsolationCheckBox_stateChanged(int state) {
    changed = true;
    if (Qt::Checked == state) {
        cfg.setUseHomeIsolation(true);
    } else {
        cfg.setUseHomeIsolation(false);
    }
}

void InstanceOptions::on_addVariableButton_clicked() {
    ui->environmentVariablesTableWidget->setRowCount(
        ui->environmentVariablesTableWidget->rowCount() + 1);
}

void InstanceOptions::
    on_environmentVariablesTableWidget_itemSelectionChanged() {
    if (ui->environmentVariablesTableWidget->selectionModel()->hasSelection())
        ui->removeVariableButton->setEnabled(true);
    else
        ui->removeVariableButton->setEnabled(false);
}

void InstanceOptions::on_removeVariableButton_clicked() {
    if (!ui->environmentVariablesTableWidget->selectionModel()->hasSelection())
        return;
    else {
        ui->environmentVariablesTableWidget->removeRow(
            ui->environmentVariablesTableWidget->currentIndex().row());
        //TODO: Remove from cfg
    }
}

void InstanceOptions::on_environmentVariablesTableWidget_itemChanged(
    QTableWidgetItem *item) {
    changed = true;
    cfg.modifyEnvironmentVariable(item->row(), item->text());
}

void InstanceOptions::protonVersionComboBoxIndexChanged(int index) {
    changed = true;
    cfg.setProtonVersion(static_cast<LigmaCore::ProtonVersion>(index));
}
void InstanceOptions::steamRuntimeComboBoxIndexChanged(int index) {
    changed = true;
    cfg.setSteamRuntimeVersion(
        static_cast<LigmaCore::SteamRuntimeVersion>(index));
}
