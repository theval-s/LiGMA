#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QList>

namespace Ui {
class MainWindow;
}

//namespace LigmaUI{} ???
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void RefreshLists();

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void pluginSelectionCheck(const QItemSelection &selected, const QItemSelection &deselected);
    void instanceSelectionCheck(const QItemSelection &selected, const QItemSelection &deselected);
    void on_createInstanceButton_clicked();
    void on_openInstanceButton_clicked();

private:
    Ui::MainWindow *ui;
};

