#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStringLiteral>
#define qsl(x) QStringLiteral(x)
#include <QGuiApplication>
#include <QItemSelection>
#include <QSplashScreen>
#include <QSqlQueryModel>
#include <QMainWindow>
#include "reporthtml.h"
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

    int id_SelectedCreditor();
    int get_current_id_from_contracts_list();

private slots:

    void currentChange_ctv(const QModelIndex & , const QModelIndex & );
    void on_action_menu_database_new_triggered();
    void on_action_menu_database_program_exit_triggered();
    void on_action_menu_database_open_triggered();

    void on_action_menu_creditors_listview_triggered();
    void on_action_menu_debug_create_sample_data_triggered();
    void on_action_menu_creditors_create_triggered();
    void on_action_menu_creditors_delete_triggered();

    void on_cancelCreateCreditor_clicked();
    void on_action_cmenu_edit_creditor_triggered();

    void on_stackedWidget_currentChanged(int arg1);

    // Buttons on the "Vertrag anlegen" page

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

    void on_action_menu_contracts_anual_interest_settlement_triggered();

    void on_action_menu_database_configure_outdir_triggered();

    void on_action_menu_contracts_print_lists_triggered();

    void on_comboUebersicht_currentIndexChanged(int index);

    void on_pbPrint_clicked();

    void on_action_menu_database_start_triggered();

    void on_action_menu_database_copy_triggered();

    void on_action_menu_database_anonymous_copy_triggered();

    void on_action_saveCreditor_go_contract_triggered();

    void on_action_saveCreditor_go_creditors_triggered();

    void on_action_saveCreditor_go_new_creditor_triggered();

    void on_action_about_DKV2_triggered();

    void on_action_cmenu_change_contract_triggered();

    void on_actionDatenbank_Views_schreiben_triggered();

    void on_actionBeendete_Vertr_ge_anzeigen_triggered();

    void on_actionProjektkonfiguration_ndern_triggered();

    void on_actionNeu_triggered();

    void on_action_Neu_triggered();

private:
    Ui::MainWindow *ui;
    void createButtonMenu_saveCreditorAnd();
    void prepare_CreditorsListPage();
    void prepare_contracts_list_view();
    void prepare_startPage();
    bool useDb(const QString& dbfile);
    void showDbInStatusbar(QString filename = "");
    int save_creditor();
    qlonglong passNewCreditorIdToNewContract =-1;
    enum stackedWidgedsPageIndex
    {
        startPageIndex =0,
        creditorsListPageIndex =1,
        newCreditorPageIndex =2,
        contractsListPageIndex =3,
        overviewsPageIndex=4,
        bookingsListIndex=5
    };
    void empty_create_creditor_form();
    void init_creditor_form(int id);
    QVector<QString> Uebersichten_kurz{
        qsl("Uebersicht"),
        qsl("Vertragsenden"),
        qsl("Zinsverteilungen"),
        qsl("Laufzeiten"),
        qsl("Gesamtübersicht")
    };
    QString prepare_overview_page(Uebersichten u);
    QMenu* menuSaveKreditorAnd;
    QMenu* menuSaveContractAnd;
private:
    bool showDeletedContracts =false;
};

#endif // MAINWINDOW_H
