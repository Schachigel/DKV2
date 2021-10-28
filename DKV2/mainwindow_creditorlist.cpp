#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pch.h"

#include "contracttablemodel.h"
#include "transaktionen.h"


// Creditor list view page
void MainWindow::on_action_menu_creditors_listview_triggered()
{   LOG_CALL;
    busycursor b;
    prepare_CreditorsListPage();
    if( not ui->CreditorsTableView->currentIndex().isValid())
        ui->CreditorsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(creditorsListPageIndex);
}
void MainWindow::prepare_CreditorsListPage()
{   LOG_CALL;
    busycursor b;
    QSqlTableModel* model = new QSqlTableModel(ui->CreditorsTableView);
    model->setTable(qsl("Kreditoren"));
    model->setFilter(qsl("Vorname LIKE '%") + ui->le_CreditorsFilter->text() + qsl("%' OR Nachname LIKE '%") + ui->le_CreditorsFilter->text() + qsl("%'"));
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->CreditorsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->CreditorsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CreditorsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->CreditorsTableView->setAlternatingRowColors(true);
    ui->CreditorsTableView->setSortingEnabled(true);
    ui->CreditorsTableView->setModel(model);
    ui->CreditorsTableView->hideColumn(0);
    ui->CreditorsTableView->hideColumn(11); // Zeitstempel
    ui->CreditorsTableView->resizeColumnsToContents();
}
void MainWindow::on_le_CreditorsFilter_editingFinished()
{   LOG_CALL;
    busycursor b;
    prepare_CreditorsListPage();
}
int  MainWindow::id_SelectedCreditor()
{   LOG_CALL;
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->CreditorsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid()) {
        QVariant data(ui->CreditorsTableView->model()->data(mi));
        return data.toInt();
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return -1;
}
void MainWindow::on_btn_reset_filter_creditors_clicked()
{   LOG_CALL;
    busycursor b;
    ui->le_CreditorsFilter->setText(qsl(""));
    prepare_CreditorsListPage();
}
void MainWindow::on_action_menu_creditors_delete_triggered()
{
    on_action_cmenu_delete_creditor_triggered();
}

// Context Menue in Creditor Table
void MainWindow::on_CreditorsTableView_customContextMenuRequested(QPoint pos)
{   LOG_CALL;
    QModelIndex index = ui->CreditorsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid()) {
        int nbrOfContracts =executeSingleValueSql(qsl("SELECT count(*) FROM Vertraege WHERE KreditorId=%1").arg(id_SelectedCreditor())).toInt();

        QMenu menu( qsl("PersonContextMenu"), this);
        menu.addAction(ui->action_cmenu_edit_creditor);
        if( nbrOfContracts > 0)
            menu.addAction(ui->action_cmenu_go_contracts);
        else
            menu.addAction(ui->action_cmenu_delete_creditor);
        menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
        return;
    }
}
void MainWindow::on_action_cmenu_edit_creditor_triggered()
{   LOG_CALL;
    editCreditor(id_SelectedCreditor());
    updateViews();
}
void MainWindow::on_action_cmenu_delete_creditor_triggered()
{   LOG_CALL;
//    const QTableView * const tv = ui->CreditorsTableView;
//    QModelIndex mi(tv->currentIndex());
//    qlonglong index = tv->model()->data(mi.siblingAtColumn(0)).toLongLong();
    int creditorId =id_SelectedCreditor();
    if( creditorId <=0 ) {
        QMessageBox::critical(this, qsl("Fehler"), qsl("Beim Löschen ist ein Fehler aufgetreten. Details findest Du im der LOG Datei"));
        return;
    }

    creditor c (id_SelectedCreditor());
    QString msg( qsl("Soll der Kreditgeber %1 %2 (id %3) gelöscht werden?"));
    msg =msg.arg(c.getValue(qsl("Vorname")).toString(), c.getValue(qsl("Nachname")).toString(), QString::number(id_SelectedCreditor()));

    if( QMessageBox::Yes not_eq QMessageBox::question(this, qsl("Kreditgeber löschen?"), msg))
        return;
    busycursor b;

    if( c.remove())
        prepare_CreditorsListPage();
    else
        QMessageBox::information(this, qsl("Löschen unmöglich"), qsl("Ein Kreditor mit aktiven oder beendeten Verträgen kann nicht gelöscht werden"));
}
void MainWindow::on_action_cmenu_go_contracts_triggered()
{   LOG_CALL;
    busycursor b;
    ui->le_ContractsFilter->setText(qsl("kreditor:") +QString::number(id_SelectedCreditor()));
    on_action_menu_contracts_listview_triggered();
}

// new creditor and contract Wiz
void MainWindow::on_actionNeu_triggered()
{
    newCreditorAndContract();
    updateViews();
}

