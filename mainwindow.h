#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_action_Neue_DB_anlegen_triggered();

    void on_actionProgramm_beenden_triggered();

    void on_actionDBoeffnen_triggered();

    void on_action_Liste_triggered();

    void on_actioncreateSampleData_triggered();

private:
    void openAppDefaultDb(QString f=QString(""));
    void preparePersonTableView();

    Ui::MainWindow *ui;

    enum stackedWidgedsIndex
    {
        emptyPageIndex = 0,
        PersonListIndex = 1
    };
};

#endif // MAINWINDOW_H
