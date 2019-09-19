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

    void on_actionNeuer_DK_Geber_triggered();

    void on_saveExit_clicked();

    void on_saveNew_clicked();

    void on_saveList_clicked();

    void on_cancel_clicked();

private:
    Ui::MainWindow *ui;

    void preparePersonTableView();
    void setCurrentDbInStatusBar();
    bool savePerson();

    enum stackedWidgedsIndex
    {
        emptyPageIndex = 0,
        PersonListIndex = 1,
        newPersonIndex = 2
    };
    void emptyEditPersonFields();
};

#endif // MAINWINDOW_H
