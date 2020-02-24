#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGuiApplication>
#include <QItemSelection>
#include <QSplashScreen>
#include <QSqlQueryModel>
#include <QMainWindow>
#include <dkdbhelper.h>
#include "vertrag.h"

struct busycursor
{
    busycursor()
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    };
    ~busycursor()
    {
        QGuiApplication::restoreOverrideCursor();
    }
};

namespace Ui {


class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void setSplash(QSplashScreen* s);

    void FillKreditorDropdown();
    void FillRatesDropdown();
    void comboKreditorenAnzeigeNachKreditorenId(int id);
    int getPersonIdFromKreditorenList();
    int getContractIdFromContractsList();
    Vertrag VertragsdatenAusFormular();

private slots:

    void on_action_Neue_DB_anlegen_triggered();
    void on_action_Programm_beenden_triggered();
    void on_action_DBoeffnen_triggered();

    void on_action_Liste_triggered();
    void on_actioncreateSampleData_triggered();
    void on_action_Neuer_DK_Geber_triggered();
    void on_action_Vertrag_anlegen_triggered(int id=-1);
    void on_saveExit_clicked();
    void on_saveNew_clicked();

    void on_saveList_clicked();
    void on_cancel_clicked();
    void on_action_DkGeberBearbeiten_triggered();

    void on_stackedWidget_currentChanged(int arg1);

    // Buttons on the "Vertrag anlegen" page
    void on_cancelCreateContract_clicked();

    void on_speichereVertragZurKreditorenListe_clicked();

    void on_saveContractGoContracts_clicked();

    void on_action_Liste_der_Vertraege_anzeigen_triggered();

    void on_PersonsTableView_customContextMenuRequested(const QPoint &pos);

    void on_action_Kreditgeber_loeschen_triggered();

    void on_contractsTableView_customContextMenuRequested(const QPoint &pos);

    void on_action_activateContract_triggered();

    void on_action_loeschePassivenVertrag_triggered();

    void on_actionanzeigenLog_triggered();

    void on_leFilter_editingFinished();

    void on_pbPersonFilterZuruecksetzen_clicked();

    void on_FilterVertraegeZuruecksetzen_clicked();

    void on_leVertraegeFilter_editingFinished();

    void on_actionVertrag_Beenden_triggered();

    void on_action_Uebersicht_triggered();

    void on_action_Vertraege_zeigen_triggered();

    void on_tblViewBookingsSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_actionShow_Bookings_triggered();

    void on_action_Jahreszinsabrechnung_triggered();

    void on_leBetrag_editingFinished();

    void on_action_Ausgabeverzeichnis_festlegen_triggered();

    void on_action_Aktive_Vertraege_CSV_triggered();

    void on_comboUebersicht_currentIndexChanged(int index);

    void on_pbPrint_clicked();

    void on_action_zurueck_triggered();

    void on_cbKFrist_currentIndexChanged(int index);

    void on_action_Kopie_anlegen_triggered();

    void on_action_Anonymisierte_Kopie_triggered();

private:
    Ui::MainWindow *ui;
    QSplashScreen* splash;
    void preparePersonTableView();
    void prepareContractListView();
    void prepareWelcomeMsg();
    void DbInStatuszeileAnzeigen();
    int KreditgeberSpeichern();
    bool saveNewContract();
    QSqlQueryModel* tmp_ContractsModel;
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
    enum Uebersichten
    {
        UEBERSICHT = 0,
        VERTRAGSENDE,
        ZINSVERTEILUNG,
        LAUFZEITEN
    };
    QVector<QString> Uebersichten_kurz{
        QString("Uebersicht"),
        QString("Vertragsenden"),
        QString("Zinsverteilungen"),
        QString("Laufzeiten")
    };
    QString prepareOverviewPage(Uebersichten u);
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
