#include "new_instance_dialog.hpp"

#include "ui_new_instance_dialog.h"

#include <QFileDialog>
#include <QLineEdit>
#include <filesystem>

NewInstanceDialog::NewInstanceDialog(const LigmaCore::PluginInfo &info,
                                     QWidget *parent)
    : QDialog(parent), ui(new Ui::NewInstanceDialog) {
    ui->setupUi(this);
    connect(ui->gamePathLineEdit, &QLineEdit::textChanged, this,
            &NewInstanceDialog::validateInputs);
    connect(ui->nameLineEdit, &QLineEdit::textEdited, this,
            &NewInstanceDialog::validateInputs);
    // connect(ui->instancePathLineEdit, &QLineEdit::textEdited, this,
    // &NewInstanceDialog::validateInputs); move lower stuff to instance name
    // line edit change signal
    _instancePath = QString::fromStdString(
        (std::filesystem::canonical("/proc/self/exe").parent_path() /
         "game_instances")
            .string());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->instancePathLineEdit->setText("/diplom/build/game_instances/test/");
}

NewInstanceDialog::~NewInstanceDialog() {
    delete ui;
}

QString NewInstanceDialog::getInstanceName() const {
    return ui->nameLineEdit->text();
}

QString NewInstanceDialog::getInstancePath() const {
    return ui->instancePathLineEdit->text();
}

QString NewInstanceDialog::getGamePath() const {
    return ui->gamePathLineEdit->text();
}

void NewInstanceDialog::validateInputs() {
    if (!ui->gamePathLineEdit->text().isEmpty() &&
        !ui->nameLineEdit->text().isEmpty() &&
        !ui->instancePathLineEdit->text().isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    // TODO: add proper validation and highlighting what's wrong
}

void NewInstanceDialog::on_gamePathBrowseButton_clicked() {
    QString game_dir =
        QFileDialog::getExistingDirectory(this, "Choose game directory");
    if (!game_dir.isEmpty()) {
        ui->gamePathLineEdit->setText(game_dir);
    }
}

void NewInstanceDialog::on_nameLineEdit_textEdited(const QString &arg1) {
    ui->instancePathLineEdit->setText(_instancePath + "/" +
                                      sanitizeForPath(arg1));
}

// Works with QString unlike InstanceFilesystem::
QString NewInstanceDialog::sanitizeForPath(const QString &input) {
    QString forbiddenChars(R"(()<>";:\|/?*[].^&$%#@)");
    QString result = input;
    for (const QChar &c : forbiddenChars) result.remove(c);
    result.replace(' ', '_');
    return result;
}
