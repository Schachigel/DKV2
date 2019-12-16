#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <dkdbhelper.h>
#include <QItemSelection>
#include <QSplashScreen>
#include <QMainWindow>
#include "vertrag.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setSplash(QSplashScreen* s);

    void FillKreditorDropdown();
    void FillRatesDropdown();
    void comboKreditorenAnzeigeNachKreditorenId(int id);
    int getPersonIdFromKreditorenList();
    int getContractIdFromContractsList();
    Vertrag VertragsdatenAusFormular();

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
    void on_actionDkGeberBearbeiten_triggered();

    void on_stackedWidget_currentChanged(int arg1);

    // Buttons on the "Vertrag anlegen" page
    void on_cancelCreateContract_clicked();

    void on_speichereVertragZurKreditorenListe_clicked();

    void on_saveContractGoContracts_clicked();

    void on_actionListe_der_Vertraege_anzeigen_triggered();

    void on_PersonsTableView_customContextMenuRequested(const QPoint &pos);

    void on_actionKreditgeber_loeschen_triggered();

    void on_contractsTableView_customContextMenuRequested(const QPoint &pos);

    void on_actionactivateContract_triggered();

    void on_actionVertrag_passiv_loeschen_triggered();

    void on_actionanzeigenLog_triggered();

    void on_leFilter_editingFinished();

    void on_pbPersonFilterZuruecksetzen_clicked();

    void on_FilterVertraegeZuruecksetzen_clicked();

    void on_leVertraegeFilter_editingFinished();

    void on_actionVertrag_Beenden_triggered();

    void on_action_Uebersicht_triggered();

    void on_actionVertraege_zeigen_triggered();

    void on_tblViewBookingsSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_actionShow_Bookings_triggered();

    void on_actionJahreszinsabrechnung_triggered();

    void on_leBetrag_editingFinished();

    void on_actionAusgabeverzeichnis_festlegen_triggered();

    void on_actionAktive_Vertraege_CSV_triggered();


private:
    Ui::MainWindow *ui;
    QSplashScreen* splash;
    int colIndexFieldActiveInContractList;
    void preparePersonTableView();
    void prepareContractListView();
    void DbInStatuszeileAnzeigen();
    bool KreditgeberSpeichern();
    bool saveNewContract();

    enum stackedWidgedsIndex
    {
        emptyPageIndex =0,
        PersonListIndex =1,
        newPersonIndex =2,
        newContractIndex =3,
        ContractsListIndex =4,
        OverviewIndex=5,
        bookingsListIndex=6
    };
    void KreditorFormulardatenLoeschen();
    void KreditorFormulardatenBelegen(int id);
    void clearNewContractFields();
protected:
    void timerEvent(QTimerEvent* te) override
    {
        if( splash)
        {
            splash->finish(this);
            delete splash;
        }
        killTimer(te->timerId());
    }
};

#endif // MAINWINDOW_H
