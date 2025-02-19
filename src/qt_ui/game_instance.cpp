#include "game_instance.hpp"

#include "ui_game_instance.h"

#include <QFileDialog>
#include <QMessageBox>
#include <memory>
#include <qstandarditemmodel.h>

GameInstance::GameInstance(LigmaCore::InstanceFilesystem instance,
                           QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      m_instance(std::move(instance)),
      m_modTableModel(std::make_unique<QStandardItemModel>(this)) {
    setupUi();
}

GameInstance::GameInstance(const QJsonObject &config,
                           const std::filesystem::path &configPath,
                           QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      m_instance(config, configPath),
      m_modTableModel(std::make_unique<QStandardItemModel>(this)) {
    setupUi();
}

GameInstance::GameInstance(const QString &name, const QString &basePath,
                           const QString &gamePath, LigmaPlugin *gamePlugin,
                           QMainWindow *parent)
    : QMainWindow(parent), ui(new Ui::GameInstance),
      m_instance(name.toStdString(), basePath.toStdString(),
                 gamePath.toStdString(), gamePlugin),
      m_modTableModel(std::make_unique<QStandardItemModel>(this)) {
    setupUi();
}

GameInstance::~GameInstance() {
    delete ui;
}

void GameInstance::setupModTable() {
    QStringList headers;
    headers << "Mod Name" << "Mod Path" << "Enabled";
    m_modTableModel->setHorizontalHeaderLabels(headers);

    ui->modTableView->setModel(m_modTableModel.get());
    ui->modTableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    // to be expanded...
    // ui->modTableView->
}

void GameInstance::refreshModList() {
    // i hope removing rows frees the memory
    m_modTableModel->removeRows(0, m_modTableModel->rowCount());

    for (const auto &[modName, modPath] : m_instance.getModList()) {
        QList<QStandardItem *> row;
        // appendRow() asks for pointers so yeah
        auto name_item = new QStandardItem(QString::fromStdString(modName));
        auto path_item =
            new QStandardItem(QString::fromStdString(modPath.string()));
        auto enabled_item = new QStandardItem();
        enabled_item->setCheckable(true);
        enabled_item->setCheckState(Qt::Checked);
        row << name_item << path_item << enabled_item;

        m_modTableModel->appendRow(row);
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
    if (m_instance.isMounted()) {
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
    m_instance.addMod(ui->TEMP_modPathLineEdit->text().toStdString(),
                      ui->modNameLineEdit->text().toStdString());
    refreshUI();
}

void GameInstance::on_modPathBrowseButton_clicked() {
    QString mod_path = QFileDialog::getExistingDirectory(this, "Select mod",
                                                         QDir::home().path());
    if (!mod_path.isEmpty()) {
        ui->TEMP_modPathLineEdit->setText(mod_path);
    }
}

void GameInstance::on_runGameButton_clicked() {
    m_instance.openGame();
    refreshUI();
}

void GameInstance::on_removeModButton_clicked() {
    auto index = ui->modTableView->currentIndex();
    m_instance.removeMod(index.row());
    refreshUI();
}

void GameInstance::on_mountGameButton_clicked() {
    m_instance.mountGameFilesystem();
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
    ui->instanceNameLabel->setText(
        QString::fromStdString(m_instance.getInstanceName()));
}
