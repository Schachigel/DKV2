#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "helper.h"
#include "dlgdisplaycolumns.h"
#include "busyCursor.h"
#include "transaktionen.h"
#include "creditor.h"
#include "helpersql.h"


// Creditor list view page
void MainWindow::on_action_menu_creditors_listview_triggered()
{   LOG_CALL;
    busyCursor b;
    prepare_CreditorsListPage();
    if( not ui->CreditorsTableView->currentIndex().isValid())
        ui->CreditorsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(creditorsListPageIndex);
}

enum colPosCreditors {
    col_id =0,
    col_Vorname, col_Nachname,
    col_Strasse, col_Plz, col_Stadt, col_Land,
    col_Tel, col_Email,
    col_Anmerkung, col_APartner, col_BKonto,
    col_IBAN, col_BIC,
    col_Zeitstempel,
    col_count
};

const QVector<tableViewColTexts> columnTextsCreditors {
    /*col_id         */ {"",         ""},
    /*col_Vorname    */ {qsl("Vorname"),  ""},
    /*col_Nachname,  */ {qsl("Nachname"), ""},
    /*col_Strasse,   */ {qsl("Strasse"),  ""},
    /*col_Plz,       */ {qsl("PLZ"),      qsl("Postleitzahl")},
    /*col_Stadt,     */ {qsl("Stadt"),    ""},
    /*col_Land,      */ {qsl("Land"),     ""},
    /*col_Tel,       */ {qsl("Telefon Nr."), qsl("Telefonnummer für schnellen Kontakt")},
    /*col_Email      */ {qsl("E-Mail"),   qsl("E-Mail Adresse zum Zusenden von Vertragsinformation")},
    /*col_Anmerkung, */ {qsl("Anmerkung"), qsl("Allgemeine Anmerkung")},
    /*col_APartner,  */ {qsl("Ansprechp."), qsl("Ansprechpartner im Projekt")},
    /*col_BKonto,    */ {qsl("Buch.Konto"), qsl("Buchungskonto oder ähnliche Info.")},
    /*col_IBAN,      */ {qsl("IBAN"),       qsl("Bank Information für Überweisungen")},
    /*col_BIC,       */ {qsl("BIC"),        qsl("zusätzliche BI für Auslandsüberw.")},
    /*col_Zeitstempel*/ {"", ""}
};

const QString creditorTableColumnVisibilityStatus {qsl("KreditorenSpalten")};
const QString creditorTableColumnVisibilityDefault{qsl("011111111111110")};

void MainWindow::prepare_CreditorsListPage()
{   LOG_CALL;
    busyCursor b;
    QSqlTableModel* model = new QSqlTableModel(ui->CreditorsTableView);
    model->setTable(qsl("Kreditoren"));
    model->setFilter(qsl("Vorname LIKE '%") + ui->le_CreditorsFilter->text() + qsl("%' OR Nachname LIKE '%") + ui->le_CreditorsFilter->text() + qsl("%'"));
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    ui->CreditorsTableView->setModel(model);

    QBitArray ba =toQBitArray (getMetaInfo (creditorTableColumnVisibilityStatus, creditorTableColumnVisibilityDefault));
    /* Force appropriate length of QBitArray */
    qsizetype oldSize = ba.size();
    ba.resize(col_count);
    if( oldSize < col_count)
        ba.fill(true, oldSize, col_count);
    /* Hide unwanted columns */
    ba[col_id] = ba[col_Zeitstempel] = 0;

    for (int i = 0; i < int(colPosCreditors::col_count); i++)
    {
        if( ba[i]) ui->CreditorsTableView->showColumn (i);
        else       ui->CreditorsTableView->hideColumn (i);
    }

    ui->CreditorsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->CreditorsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CreditorsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->CreditorsTableView->setAlternatingRowColors(true);
    ui->CreditorsTableView->setSortingEnabled(true);
    ui->CreditorsTableView->resizeColumnsToContents();
}
void MainWindow::on_le_CreditorsFilter_editingFinished()
{   LOG_CALL;
    busyCursor b;
    prepare_CreditorsListPage();
}
int  MainWindow::id_SelectedCreditor()
{   LOG_CALL;
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->CreditorsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid()) {
        QVariant id(ui->CreditorsTableView->model()->data(mi));
        return id.toInt();
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return SQLITE_invalidRowId;
}
void MainWindow::on_btn_reset_filter_creditors_clicked()
{   LOG_CALL;
    busyCursor b;
    ui->le_CreditorsFilter->setText("");
    prepare_CreditorsListPage();
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
    msg =msg.arg(c.getValue(creditor::fnVorname).toString(), c.getValue(creditor::fnNachname).toString(), i2s(id_SelectedCreditor()));

    if( QMessageBox::Yes not_eq QMessageBox::question(this, qsl("Kreditgeber löschen?"), msg))
        return;
    busyCursor b;

    if( c.remove())
        prepare_CreditorsListPage();
    else
        QMessageBox::information(this, qsl("Löschen unmöglich"), qsl("Ein Kreditor mit aktiven oder beendeten Verträgen kann nicht gelöscht werden"));
}
void MainWindow::on_action_cmenu_go_contracts_triggered()
{   LOG_CALL;
    busyCursor b;
    ui->le_ContractsFilter->setText(qsl("kreditor:") +i2s(id_SelectedCreditor()));
    on_action_menu_contracts_listview_triggered();
}

// new creditor and contract Wiz
void MainWindow::on_actionNeu_triggered()
{
    newCreditorAndContract();
    updateViews();
}

void MainWindow::on_pbCreditorsColumnsOnOff_clicked()
{
    QBitArray ba =toQBitArray (getMetaInfo(creditorTableColumnVisibilityStatus, creditorTableColumnVisibilityDefault));
    qsizetype oldSize = ba.size();
    ba.resize(col_count);
    ba.fill(true, oldSize, col_count);

    QVector <QPair<int, QString>> colInfo;
    for(int i=0; i<int(colPosCreditors::col_count); i++) {
        colInfo.push_back (QPair<int, QString>(i, columnTextsCreditors[i].header));
    }
    dlgDisplayColumns dlg(colInfo, ba, getMainWindow ());
    if( dlg.exec () == QDialog::Accepted ) {
        setMetaInfo (creditorTableColumnVisibilityStatus, toString(dlg.getNewStatus()));
        prepare_CreditorsListPage ();
    }

}
