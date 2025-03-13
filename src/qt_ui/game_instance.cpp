#include "game_instance.hpp"

#include "ui_game_instance.h"

#include <QButtonGroup>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>
#include <instance_factory.hpp>
#include <memory>
#include <qstandarditemmodel.h>

GameInstance::GameInstance(
    std::unique_ptr<LigmaCore::IInstanceFilesystem> instance,
    QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      instance(std::move(instance)),
      modTableModel(std::make_unique<QStandardItemModel>(this)) {
    setupUi();
}

GameInstance::GameInstance(
    const QJsonObject &config, const std::filesystem::path &configPath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin,
    QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      instance(std::move(LigmaCore::createInstanceFilesystem(
          config, configPath, std::move(gamePlugin)))),
      modTableModel(std::make_unique<QStandardItemModel>(this)) {

    setupUi();
}

GameInstance::GameInstance(
    const QString &name, const QString &basePath, const QString &gamePath,
    std::unique_ptr<LigmaPlugin, std::function<void(LigmaPlugin *)>> gamePlugin,
    QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      instance(std::move(LigmaCore::createInstanceFilesystem(
          name, basePath, gamePath, std::move(gamePlugin)))),
      modTableModel(std::make_unique<QStandardItemModel>(this)) {
    setupUi();
}

GameInstance::~GameInstance() {
    delete ui;
}

void GameInstance::setupModTable() {
    QStringList headers;
    headers << "Mod Name" << "Mod Path" << "Enabled";
    modTableModel->setHorizontalHeaderLabels(headers);

    ui->modTableView->setModel(modTableModel.get());
    ui->modTableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    // to be expanded...
    // ui->modTableView->
}

void GameInstance::refreshModList() {
    // i hope removing rows frees the memory
    modTableModel->removeRows(0, modTableModel->rowCount());

    for (const auto &[modName, modPath, state, type] : instance->getModList()) {
        QList<QStandardItem *> row;
        // appendRow() asks for pointers so yeah
        auto name_item = new QStandardItem(modName);
        auto path_item =
            new QStandardItem(QString::fromStdString(modPath.string()));
        auto enabled_item = new QStandardItem();
        enabled_item->setCheckable(true);
        enabled_item->setCheckState(state ? Qt::Checked : Qt::Unchecked);
        row << name_item << path_item << enabled_item;

        modTableModel->appendRow(row);
    }
}

void GameInstance::validateInputs() {
    if (!ui->TEMP_modPathLineEdit->text().isEmpty() &&
        !ui->modNameLineEdit->text().isEmpty()) {
        ui->addModButton->setEnabled(true);
    }
    // TODO: add proper validation and highlighting what's wrong
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

void GameInstance::on_addModButton_clicked() {
    QString destPath;
    QDialog dialog(this);
    dialog.setWindowTitle("Select Mod Destination");
    auto layout = new QVBoxLayout(&dialog);
    auto group = new QButtonGroup(&dialog);
    std::vector<QString> paths = instance->getModPaths();
    int id = 0;
    for (const auto &pathString : paths) {
        QRadioButton *button = new QRadioButton(pathString);
        group->addButton(button, id++);
        layout->addWidget(button);
    }
    group->buttons()[0]->setChecked(true);
    auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        std::cerr << "QDialog accepted with choice: "
                  << paths[group->checkedId()].toStdString() << std::endl;
        if (group->checkedId() == 0)
            destPath = "";
        else
            destPath = paths[group->checkedId()];
        //testing
        try {
            instance->addMod(ui->TEMP_modPathLineEdit->text().toStdString(),
                             ui->modNameLineEdit->text(), destPath);
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", e.what());
        }
        refreshUI();
    }
}

void GameInstance::on_modPathBrowseButton_clicked() {
    QString mod_path = QFileDialog::getExistingDirectory(this, "Select mod",
                                                         QDir::home().path());
    if (!mod_path.isEmpty()) {
        ui->TEMP_modPathLineEdit->setText(mod_path);
    }
}

void GameInstance::on_runGameButton_clicked() {
    instance->runGame();
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

void GameInstance::setupUi() {

    ui->setupUi(this);
    setupModTable();
    refreshUI();

    connect(ui->refreshUiButton, &QPushButton::clicked, this,
            &GameInstance::refreshUI);
    connect(ui->TEMP_modPathLineEdit, &QLineEdit::textChanged, this,
            &GameInstance::validateInputs);
    connect(ui->modNameLineEdit, &QLineEdit::textEdited, this,
            &GameInstance::validateInputs);
    connect(ui->modTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &GameInstance::selectionCheck);
    ui->addModButton->setDisabled(true);
    ui->removeModButton->setDisabled(true);
    ui->instanceNameLabel->setText(instance->getInstanceName());
}
