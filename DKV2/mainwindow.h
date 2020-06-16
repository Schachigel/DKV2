#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGuiApplication>
#include <QItemSelection>
#include <QSplashScreen>
#include <QSqlQueryModel>
#include <QMainWindow>
#include <dkdbhelper.h>
#include "contract.h"

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
    int id_SelectedCreditor();
    int get_current_id_from_contracts_list();
    contract get_contract_data_from_form();


private slots:

    void currentChange_ctv(const QModelIndex & , const QModelIndex & );
    void on_action_menu_database_new_triggered();
    void on_action_menu_database_program_exit_triggered();
    void on_action_menu_database_open_triggered();

    void on_action_menu_creditors_listview_triggered();
    void on_action_menu_debug_create_sample_data_triggered();
    void on_action_menu_creditors_create_triggered();
    void on_action_menu_creditors_delete_triggered();
    void on_action_menu_contracts_create_triggered();

    void on_cancelCreateCreditor_clicked();
    void on_action_cmenu_edit_creditor_triggered();

    void on_stackedWidget_currentChanged(int arg1);

    // Buttons on the "Vertrag anlegen" page
    void on_cancelCreateContract_clicked();

    void on_action_menu_contracts_listview_triggered();

    void on_CreditorsTableView_customContextMenuRequested(const QPoint &pos);

    void on_action_cmenu_delete_creaditor_triggered();

    void on_contractsTableView_customContextMenuRequested(const QPoint &pos);

    void on_action_cmenu_activate_contract_triggered();

    void on_action_cmenu_delete_inactive_contract_triggered();

    void on_action_menu_debug_show_log_triggered();

    void on_le_CreditorsFilter_editingFinished();

    void on_btn_reset_filter_creditors_clicked();

    void on_reset_contracts_filter_clicked();

    void on_le_ContractsFilter_editingFinished();

    void on_action_cmenu_terminate_contract_triggered();

    void on_action_menu_contracts_statistics_view_triggered();

    void on_action_cmenu_go_contracts_triggered();

    void on_tblViewBookingsSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_actionShow_Bookings_triggered();

    void on_action_menu_contracts_anual_interest_settlement_triggered();

    void on_leBetrag_editingFinished();

    void on_action_menu_database_configure_outdir_triggered();

    void on_action_menu_contracts_print_lists_triggered();

    void on_comboUebersicht_currentIndexChanged(int index);

    void on_pbPrint_clicked();

    void on_action_menu_database_start_triggered();

    void on_cbKFrist_currentIndexChanged(int index);

    void on_action_menu_database_copy_triggered();

    void on_action_menu_database_anonymous_copy_triggered();

    void on_action_saveCreditor_go_contract_triggered();

    void on_action_saveCreditor_go_creditors_triggered();

    void on_action_saveCreditor_go_new_creditor_triggered();

    void on_action_save_contract_go_contracts_triggered();

    void on_action_save_contract_go_kreditors_triggered();

    void on_action_save_contract_new_contract_triggered();

    void on_deLaufzeitEnde_userDateChanged(const QDate &date);

    void on_action_about_DKV2_triggered();

    void on_action_cmenu_change_contract_triggered();

private:
    Ui::MainWindow *ui;
    QSplashScreen* splash;
    void fillCombo_NoticePeriods();
    void createButtonMenu_saveCreditorAnd();
    void createBtnMenu_saveContractAnd();
    void prepare_CreditorsListPage();
    void prepare_contracts_list_view();
    void prepare_startPage();
    bool useDb(const QString& dbfile="");
    void showDbInStatusbar(QString filename = "");
    int save_creditor();
    qlonglong passNewCreditorIdToNewContract =-1;
    bool save_new_contract();
    enum stackedWidgedsPageIndex
    {
        startPageIndex =0,
        creditorsListPageIndex =1,
        newCreditorPageIndex =2,
        newContractPageIndex =3,
        contractsListPageIndex =4,
        overviewsPageIndex=5,
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
