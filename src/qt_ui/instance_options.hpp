//
// Created by val on 4/10/25.
//

#pragma once
#include "config.hpp"

#include <QDialog>
#include <qtablewidget.h>

namespace Ui {
class InstanceOptions;
}

class InstanceOptions : public QDialog {
    Q_OBJECT

  public:
    bool changed = false;

    InstanceOptions() = delete;
    InstanceOptions(LigmaCore::UserConfig &cfg, bool usesProton,
                    QWidget *parent = nullptr);
    ~InstanceOptions() override;

  private:
    Ui::InstanceOptions *ui;
    LigmaCore::UserConfig &cfg;
    bool usesProton;

    void fillEnvironmentVariablesTable();
    void fillProtonComboBox();
  private slots:
    void protonVersionComboBoxIndexChanged(int index);

    void on_useHomeIsolationCheckBox_stateChanged(int state);
    void on_addVariableButton_clicked();
    void on_environmentVariablesTableWidget_itemSelectionChanged();
    void on_removeVariableButton_clicked();
    void on_environmentVariablesTableWidget_itemChanged(
        QTableWidgetItem *item);
};
