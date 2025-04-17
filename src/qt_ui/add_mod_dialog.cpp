//
// Created by val on 4/17/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_add_mod_dialog.h" resolved

#include "add_mod_dialog.hpp"

#include "QFileDialog"
#include "ui_add_mod_dialog.h"



AddModDialog::AddModDialog(const std::vector<QString> &paths, QWidget *parent) :
    QDialog(parent), ui(new Ui::AddModDialog) {
    ui->setupUi(this);
    this->setWindowTitle("Add mod...");

    buttonGroup = std::make_unique<QButtonGroup>(this);
    for (const auto &pathString : paths) {
        auto radioButton = new QRadioButton(pathString, this);
        buttons.push_back(radioButton);
        buttonGroup->addButton(radioButton);
        ui->verticalLayout->insertWidget(ui->verticalLayout->count() - 1, radioButton);
    }
    buttonGroup->buttons()[0]->setChecked(true);
}

AddModDialog::~AddModDialog() {
    delete ui;
    for (auto button : buttons) {
        delete button;
    }
}
QString AddModDialog::getDestinationPath() const {
    if (buttonGroup->checkedButton()->text() == "Game root") return "";
    else return buttonGroup->checkedButton()->text();
}
QString AddModDialog::getModPathQString() const {
    return ui->modPathLineEdit->text();
}
std::filesystem::path AddModDialog::getModPath() const {
    return ui->modPathLineEdit->text().toStdString();
}
QString AddModDialog::getModName() const {
    return ui->modNameLineEdit->text();
}

void AddModDialog::on_modPathBrowseButton_clicked()
{
    QString mod_path = QFileDialog::getExistingDirectory(this, "Select mod",
                                                         QDir::home().path());
    if (!mod_path.isEmpty()) {
        ui->modPathLineEdit->setText(mod_path);
    }
}

