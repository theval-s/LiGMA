//
// Created by val on 4/17/25.
//

#pragma once

#include <QButtonGroup>
#include <QDialog>
#include <QRadioButton>
#include <filesystem>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class AddModDialog; }
QT_END_NAMESPACE

class AddModDialog : public QDialog {
Q_OBJECT

public:
    explicit AddModDialog(const std::vector<QString> &paths, QWidget *parent = nullptr);
    ~AddModDialog() override;

    QString getDestinationPath() const;
    QString getModPathQString() const;
    std::filesystem::path getModPath() const;
    QString getModName() const;

  private slots:
    void on_modPathBrowseButton_clicked();

  private:
    Ui::AddModDialog *ui;
    std::vector<QRadioButton *> buttons;
    std::unique_ptr<QButtonGroup> buttonGroup;
};

