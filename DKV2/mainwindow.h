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

    void fill_creditors_dropdown();
    void fill_rates_dropdown();
    void set_creditors_combo_by_id(int id);
    int getIdFromCreditorsList();
    int get_current_id_from_contracts_list();
    Contract get_contract_data_from_form();

private slots:

    void on_action_create_new_DB_triggered();
    void on_action_exit_program_triggered();
    void on_action_open_DB_triggered();

    void on_action_Liste_triggered();
    void on_action_create_sample_data_triggered();
    void on_action_create_new_creditor_triggered();
    void on_action_create_contract_for_creditor_triggered(int id=-1);

    void on_cancel_clicked();
    void on_action_edit_Creditor_triggered();

    void on_stackedWidget_currentChanged(int arg1);

    // Buttons on the "Vertrag anlegen" page
    void on_cancelCreateContract_clicked();

    void on_action_show_list_of_contracts_triggered();

    void on_CreditorsTableView_customContextMenuRequested(const QPoint &pos);

    void on_action_delete_creditor_triggered();

    void on_contractsTableView_customContextMenuRequested(const QPoint &pos);

    void on_action_activate_contract_triggered();

    void on_action_loeschePassivenVertrag_triggered();

    void on_action_log_anzeigen_triggered();

    void on_leFilter_editingFinished();

    void on_pbPersonFilterZuruecksetzen_clicked();

    void on_reset_contracts_filter_clicked();

    void on_leVertraegeFilter_editingFinished();

    void on_action_terminate_contract_triggered();

    void on_action_contracts_statistics_triggered();

    void on_action_show_contracts_triggered();

    void on_tblViewBookingsSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_actionShow_Bookings_triggered();

    void on_action_anual_interest_settlement_triggered();

    void on_leBetrag_editingFinished();

    void on_action_store_output_directory_triggered();

    void on_action_create_active_contracts_csv_triggered();

    void on_comboUebersicht_currentIndexChanged(int index);

    void on_pbPrint_clicked();

    void on_action_back_triggered();

    void on_cbKFrist_currentIndexChanged(int index);

    void on_action_create_copy_triggered();

    void on_action_create_anonymous_copy_triggered();

    void on_action_save_contact_go_contract_triggered();

    void on_action_save_contact_go_creditors_triggered();

    void on_action_save_contact_go_new_creditor_triggered();

    void on_action_save_contract_go_contracts_triggered();

    void on_action_save_contract_go_kreditors_triggered();

    void on_action_save_contract_new_contract_triggered();

    void on_deLaufzeitEnde_userDateChanged(const QDate &date);

    void on_action_ber_DKV2_triggered();

private:
    Ui::MainWindow *ui;
    QSplashScreen* splash;
    void prepareCreditorsTableView();
    void prepare_contracts_list_view();
    void prepareWelcomeMsg();
    bool useDb(const QString& dbfile="");
    void showDbInStatusbar();
    int save_creditor();
    bool save_new_contract();
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
    void empty_create_creditor_form();
    void init_creditor_form(int id);
    void empty_new_contract_form();
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
    QString prepare_overview_page(Uebersichten u);
    QMenu* menuSaveKreditorAnd;
    QMenu* menuSaveContractAnd;
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
