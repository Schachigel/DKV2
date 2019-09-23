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

    void FillDKGeberCombo();

    void FillRatesCombo();

    void SelectcbDKGeberComboByPersonId(int id);

    int getPersonIdFromDKGeberList();

private slots:

    void on_action_Neue_DB_anlegen_triggered();

    void on_actionProgramm_beenden_triggered();

    void on_actionDBoeffnen_triggered();

    void on_action_Liste_triggered();

    void on_actioncreateSampleData_triggered();

    void on_actionNeuer_DK_Geber_triggered();

    void on_actionVertrag_anlegen_triggered();

    void on_saveExit_clicked();

    void on_saveNew_clicked();

    void on_saveList_clicked();

    void on_cancel_clicked();

    void on_stackedWidget_currentChanged(int arg1);

    // Buttons on the "Vertrag anlegen" page
     void on_cancelCreateContract_clicked();

    void on_saveContractGoToDKGeberList_clicked();

    void on_saveContractGoContracts_clicked();

    void on_actionListe_der_Vertr_ge_anzeigen_triggered();

private:
    Ui::MainWindow *ui;

    void preparePersonTableView();
    void prepareContractListView();
    void setCurrentDbInStatusBar();
    bool savePerson();
    bool saveNewContract();

    enum stackedWidgedsIndex
    {
        emptyPageIndex =0,
        PersonListIndex =1,
        newPersonIndex =2,
        newContractIndex =3,
        ContractsListIndex =4
    };
    void clearEditPersonFields();
    void clearNewContractFields();
};

#endif // MAINWINDOW_H
