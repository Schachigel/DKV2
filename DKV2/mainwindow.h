#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStringLiteral>
#include <QGuiApplication>
#include <QItemSelection>
#include <QSplashScreen>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QMainWindow>
#include <QPrinter>

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

class InvestmentsTableModel : public QSqlTableModel
{
public:
    InvestmentsTableModel(QWidget* w) : QSqlTableModel(w){};
private:
    QVariant data(const QModelIndex& i, int role) const;
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

    bool dbLoadedSuccessfully =false;

    int id_SelectedCreditor();
    int get_current_id_from_contracts_list();
    void updateListViews();
#ifndef QT_DEBUG
protected:
    void closeEvent(QCloseEvent *event) override;
#endif
private slots:

    void currentChange_ctv(const QModelIndex & , const QModelIndex & );
    void on_action_menu_database_new_triggered();
    void on_action_menu_database_program_exit_triggered();

    void on_action_menu_creditors_listview_triggered();
    void on_action_menu_debug_create_sample_data_triggered();
    void on_action_menu_creditors_delete_triggered();

    void on_action_cmenu_edit_creditor_triggered();

    void on_stackedWidget_currentChanged(int arg1);

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

    void on_action_menu_contracts_annual_interest_settlement_triggered();

    void on_action_menu_database_configure_outdir_triggered();

    void on_action_menu_contracts_print_lists_triggered();

    void on_comboUebersicht_currentIndexChanged(int index);

    void on_pbPrint_clicked();

    void on_action_menu_database_start_triggered();

    void on_action_menu_database_copy_triggered();

    void on_action_menu_database_anonymous_copy_triggered();

    void on_action_about_DKV2_triggered();

    void on_action_cmenu_change_contract_triggered();

    void on_actionDatenbank_Views_schreiben_triggered();

    void on_actionBeendete_Vertr_ge_anzeigen_triggered();

    void on_actionProjektkonfiguration_ndern_triggered();

    void on_actionNeu_triggered();

    void on_action_Neu_triggered();

    void on_actionAktuelle_Auswahl_triggered();

    void on_actionTEST_triggered();

    void on_btnNextBooking_clicked();
    void on_btnPrevBooking_clicked();
    void on_btnUpdatePreview_clicked();
    void doPaint(QPrinter*);

    void on_btnCreateFromContracts_clicked();

    void on_btnNewInvestment_clicked();

    void on_actionAnlagen_verwalten_triggered();

    void on_InvestmentsTableView_customContextMenuRequested(const QPoint &pos);

    void on_actionInvestmentLoeschen_triggered();

    void on_actionInvestmentSchliessen_triggered();

    void on_actionTyp_Bezeichnung_aendern_triggered();

private:
    Ui::MainWindow *ui;
    void prepare_CreditorsListPage();
    void prepare_deleted_contracts_list_view();
    void prepare_valid_contraccts_list_view();
    void prepare_contracts_list_view();
    void prepare_investmentsListView();
    void prepare_startPage();

    void prepare_printPreview();
    QVector<booking> toBePrinted;
    QVector<booking>::const_iterator currentBooking;

    QString askUserForNextDb();
    QString findValidDatabaseToUse();
    bool useDb(const QString& dbfile);
    void showDbInStatusbar(QString filename = "");

    enum stackedWidgedsPageIndex
    {
        startPageIndex
        ,creditorsListPageIndex
        ,contractsListPageIndex
        ,overviewsPageIndex
        ,investmentsPageIndex
        ,printPreviewPageIndex
    };
    QVector<QString> Statistics_Filenames{
        qsl("Kurzinfo"),
        qsl("Uebersicht"),
        qsl("Zinsen"),
        qsl("Vertragsenden"),
        qsl("Zinsverteilungen"),
        qsl("Laufzeiten"),
        qsl("Gesamtübersicht")
    };

private:
    bool showDeletedContracts =false;
};

#endif // MAINWINDOW_H
