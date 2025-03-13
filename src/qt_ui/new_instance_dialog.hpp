#pragma once

#include "plugin_handler.hpp" //for PluginInfo, but I should probably move it to other scope

#include <QDialog>
#include <QString>

namespace Ui {
class NewInstanceDialog;
}

class NewInstanceDialog : public QDialog {
    Q_OBJECT

  public:
    explicit NewInstanceDialog(const LigmaCore::PluginInfo &,
                               QWidget *parent = nullptr);
    ~NewInstanceDialog();

    QString getInstanceName() const;
    QString getInstancePath() const;
    QString getGamePath() const;

  private slots:
    void validateInputs();
    void on_gamePathBrowseButton_clicked();

    void on_nameLineEdit_textEdited(const QString &arg1);

  private:
    Ui::NewInstanceDialog *ui;
    QString _instancePath;
    QString sanitizeForPath(const QString &);
};
