
#include "helper.h"

#include "busycursor.h"
#include "csvwriter.h"
#include "contracttablemodel.h"
#include "uiitemformatter.h"
#include "dlgdisplaycolumns.h"

#include "dkdbviews.h"
#include "investment.h"
#include "transaktionen.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"


/////////////////////////////////////////////////
// Contracts List & deleted contracts list
/////////////////////////////////////////////////
void MainWindow::on_action_menu_contracts_listview_triggered()
{   LOG_CALL;
    showDeletedContracts =false;
    prepare_contracts_list_view();
    ui->actionAktuelle_Auswahl->setEnabled (true);
}
void MainWindow::on_action_menu_Beendete_Vertr_ge_anzeigen_triggered()
{
    busycursor bc;
    showDeletedContracts =true;
    prepare_contracts_list_view();
}
void MainWindow::prepare_contracts_list_view()
{   LOG_CALL;
    busycursor bc;

    if( showDeletedContracts)
        prepare_deleted_contracts_list_view();
    else
        prepare_valid_contracts_list_view();

    ui->contractsTableView->selectRow(0);
    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}

namespace {
qlonglong intFromRight(const QString& input, const QString& flag)
{
    auto convOk =false;
    QStringRef argument =input.rightRef(input.length()-flag.length());
    qlonglong nbr =argument.toInt(&convOk);
    if( convOk)
        return nbr;
    else
        return 0;
}
QString nbrFromRight(const QString& input, const QString& flag)
{
    auto nbr =intFromRight(input, flag);
    if( 0)
        return QString();
    else
        return i2s(nbr);
}
QString filterFromFilterphrase(const QString &fph)
{
    const QString filterKreditor {qsl("kreditor:")};
    if( fph.startsWith(filterKreditor, Qt::CaseSensitivity::CaseInsensitive)) {
        return qsl("KreditorId=%1").arg(nbrFromRight(fph, filterKreditor));
    }
    const QString filterValueGt {qsl("vwert:>")};
    if( fph.startsWith(filterValueGt, Qt::CaseSensitivity::CaseInsensitive)) {
        return qsl("Nominalwert>=%1").arg(nbrFromRight(fph, filterValueGt));
    }
    const QString filterValueLt {qsl("vwert:<")};
    if( fph.startsWith(filterValueLt, Qt::CaseSensitivity::CaseInsensitive)) {
        return qsl("Nominalwert<%1").arg(nbrFromRight(fph, filterValueLt));
    }
    const QString filterInvestment {qsl("Anlage:")};
    if( fph.startsWith(filterInvestment, Qt::CaseSensitivity::CaseInsensitive)) {
        return qsl("AnlagenId=%1").arg(nbrFromRight(fph, filterValueLt));
    }
    return fph.isEmpty() ? QString() :
                           (qsl("Kreditorin LIKE '%") + fph
                            + qsl("%' OR Vertragskennung LIKE '%") + fph
                            + qsl("%' OR Zinssatz LIKE '%") +fph +"%'");
}
void applyFilterToModel( QSqlTableModel* model, const QString filter)
{   LOG_CALL_W (filter);
    model->setFilter( filterFromFilterphrase(filter));
}
}
const QVector<tableViewColTexts> columnTextsContracts {
    /*cp_vid,             */ {"", ""},
    /*cp_Creditor_id,     */ {"", ""},
    /*cp_Investment_id,   */ {"", ""},
    /*cp_Creditor,        */ {qsl("KreditorIn"), qsl("Nachname, Vorname der Vertragspartnerin / des Vertragsparnters")},
    /*cp_ContractLabel,   */ {qsl("Vertragskennung"), qsl("Die Vertragskennung identifiziert den Vertrag eindeutig")},
    /*cp_Comment,         */ {qsl("Anmerkung"), qsl("Freitext zum Vertrag")},
    /*cp_ContractDate,    */ {qsl("Vertragsdatum"), qsl("Datum der letzten Unterschrift auf dem Vertrag")},
    /*cp_InitialBooking,  */ {qsl("Geldeingang"), qsl("'Wertstellung' der ersten Überweisung")},
    /*cp_InterestActive,  */ {qsl("Zins Status"), qsl("Ist die Zinsanrechnung aktiv?")},
    /*cp_ContractValue,   */ {qsl("Nominalwert"), qsl("Bei aktiven Verträgen: Höhe der Einlage, sonst der im Vertrag vereinbarte Kreditbetrag")},
    /*cp_InterestRate,    */ {qsl("Zinssatz"), qsl("Zinssatz in %")},
    /*cp_InterestMode,    */ {qsl("Zinsmodus"), qsl("Verträge können Auszahlend, Thesaurierend oder mit festem Zins vereinbart sein")},
    /*cp_InterestBearing, */ {qsl("Verzinsliches\nGuthaben"), qsl("Bei thesaurierenden Verträgen: Einlage und angesparte Zinsen")},
    /*cp_Interest,        */ {qsl("Angesparter\nZins"), qsl("Nicht ausgezahlte Zinsen bei Verträgen mit fester Verzinsung und thesaurierenden Verträgen")},
    /*cp_ContractEnd      */ {qsl("Kündigungsfrist/ \nVertragsende"), qsl("Modalität und Datum des Vertrag Endes")},
    /*cp_CancelDate       */ {qsl("Kündigungsdatum"), qsl("Datum, zu dem die Kündigung ausgesprochen wurde")}
};
const QString defaultVisibilityPattern_contracts {qsl("0001111111111111")};
const QString visibilityPatternMetaInfoName {qsl("VertraegeSpalten")};
const QVector<tableViewColTexts> columnTexts_d_Contracts {
    /*cp_d_vid,                */ {"", ""},
    /*cp_d_Creditor_id,        */ {"", ""},
    /*cp_d_Creditor,           */ {qsl("KreditorIn"), qsl("Nachname, Vorname der Vertragspartnerin / des Vertragsparnters")},
    /*cp_d_ContractLabel,      */ {qsl("Vertragskennung"), qsl("Die Vertragskennung identifiziert den Vertrag eindeutig")},
    /*cp_d_ContractActivation, */ {qsl("Geldeingang"), qsl("'Wertstellung' der ersten Überweisung")},
    /*cp_d_ContractTermination,*/ {qsl("Vertragsende"), qsl("Datum, zu dem der Vertrag beendet wurde")},
    /*cp_d_InitialValue,       */ {qsl("Nominalwert"), qsl("Ursprünglich vereinbarter Kreditbetrag")},
    /*cp_d_InterestRate,       */ {qsl("Zinssatz"), qsl("Vereinbarter Zinssatz")},
    /*cp_d_InterestMode,       */ {qsl("Zinsmodus"), qsl("Verträge können Auszahlend, Thesaurierend oder mit festem Zins vereinbart sein")},
    /*cp_d_Interest,           */ {qsl("Zins"), ""},
    /*cp_d_TotalDeposit,       */ {qsl("Einzahlungen"), qsl("Summe aller Einzahlungen")},
    /*cp_d_FinalPayout,        */ {qsl("Abschl. Auszahlung"), ""},
    /*cp_CancelDate       */      {qsl("Kündigungsdatum"), qsl("Datum, zu dem die Kündigung ausgesprochen wurde")}
    /*cp_d_colCount            */
};
const QString defaultVisibilityPattern_deletedContracts {qsl("0011111111111")};
const QString visibilityPattern_d_MetaInfoName {qsl("geloeschteVertraegeSpalten")};
///////////////////////////////////////
/// preparation of the list view
///////////////////////////////////////
void MainWindow::prepare_valid_contracts_list_view()
{ LOG_CALL;
    busycursor bc;

    QSqlTableModel* model = new ContractTableModel(this);
    QSortFilterProxyModel *proxy = new ContractProxyModel(this);
    model->setTable(vnContractView);

    for(int i =0; i< int(colmn_Pos::cp_colCount); i++) {
        model->setHeaderData (i, Qt::Horizontal, columnTextsContracts[i].header);
        model->setHeaderData (i, Qt::Horizontal, columnTextsContracts[i].toolTip, Qt::ToolTipRole);
    }

    QTableView *&tv = ui->contractsTableView;
    proxy->setSourceModel(model);
    tv->setModel(proxy);
    if ( not model->select()) {
        qCritical() << "Model selection failed" << model->lastError();
        return;
    }

    qobject_cast<ContractTableModel*>(model)->setCol13ExtraData();

    applyFilterToModel(model, ui->le_ContractsFilter->text());

    tv->setEditTriggers(QTableView::NoEditTriggers);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setAlternatingRowColors(true);
    tv->setSortingEnabled(true);
    tv->setItemDelegateForColumn(cp_InitialBooking, new DateItemFormatter);
    tv->setItemDelegateForColumn(cp_InterestActive, new DateItemFormatter);
    tv->setItemDelegateForColumn(cp_ContractDate, new DateItemFormatter);
    tv->setItemDelegateForColumn(cp_ContractValue, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_InterestBearing, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_Interest, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_InterestMode, new interestModeFormatter(tv));
    tv->setItemDelegateForColumn(cp_CancelDate, new DateItemFormatter);

    Q_ASSERT(cp_colCount == columnTextsContracts.count ());
    QBitArray ba =toQBitArray(getMetaInfo (visibilityPatternMetaInfoName, defaultVisibilityPattern_contracts));
    /* make sure that array is long enough to hold all columns */
    int oldSize = ba.size();
    ba.resize(cp_colCount);
    ba.fill(true, oldSize, cp_colCount);
    for(int i=0; i<int(colmn_Pos::cp_colCount); i++) {
        if( columnTextsContracts[i].header.isEmpty ()){
            tv->hideColumn (i);
            ba[i] =false;
            setMetaInfo (visibilityPatternMetaInfoName, toString(ba));
        } else {
            if( ba[i])
                tv->showColumn (i);
            else
                tv->hideColumn (i);
        }
    }

    tv->resizeColumnsToContents();
    tv->resizeRowsToContents();

    connect(ui->contractsTableView->selectionModel(),
            &QItemSelectionModel::currentChanged, this ,&MainWindow::currentChange_ctv);

    if( not proxy->rowCount()) {
        ui->bookingsTableView->setModel(new QSqlTableModel(this));
    } else
        tv->setCurrentIndex(proxy->index(0, 1));
}
void MainWindow::prepare_deleted_contracts_list_view()
{ LOG_CALL;
    busycursor bc;
    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable(vnExContractView);

    for(int i =0; i< int(column_pos_del::cp_d_colCount); i++) {
        model->setHeaderData(i, Qt::Horizontal, columnTexts_d_Contracts[i].header);
        model->setHeaderData(i, Qt::Horizontal, columnTexts_d_Contracts[i].toolTip, Qt::ToolTipRole);
    }

    QTableView*& contractsTV = ui->contractsTableView;
    contractsTV->setModel(model);
    if ( not model->select()) {
        qCritical() << "Model selection failed" << model->lastError();
        return;
    }

    applyFilterToModel(model, ui->le_ContractsFilter->text());

    contractsTV->setEditTriggers(QTableView::NoEditTriggers);
    contractsTV->setSelectionMode(QAbstractItemView::SingleSelection);
    contractsTV->setSelectionBehavior(QAbstractItemView::SelectRows);
    contractsTV->setAlternatingRowColors(true);
    contractsTV->setSortingEnabled(true);
    contractsTV->setItemDelegateForColumn(cp_d_InitialValue, new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_d_Interest,     new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_d_TotalDeposit, new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_d_FinalPayout,  new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_d_CancelDate,   new DateItemFormatter);


    Q_ASSERT(cp_d_colCount == columnTexts_d_Contracts.count());
    QBitArray ba =toQBitArray (getMetaInfo (visibilityPattern_d_MetaInfoName, defaultVisibilityPattern_deletedContracts));
    /* make sure that array is long enough to hold all columns */
    int oldSize = ba.size();
    ba.resize(cp_d_colCount);
    ba.fill(true, oldSize, cp_d_colCount);
    for (int i = 0; i < int(cp_d_colCount); i++) {
        if( columnTexts_d_Contracts[i].header.isEmpty ()) {
            contractsTV->hideColumn (i);
            ba[i] =false;
            setMetaInfo (visibilityPattern_d_MetaInfoName, toString(ba));
        } else {
            if( ba[i])
                contractsTV->showColumn (i);
            else
                contractsTV->hideColumn (i);
        }
    }

    contractsTV->resizeColumnsToContents();
    contractsTV->resizeRowsToContents();

    connect(ui->contractsTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::currentChange_ctv);

    if( model->rowCount() == 0) {
        ui->bookingsTableView->setModel(new QSqlTableModel(this));
    } else
        contractsTV->setCurrentIndex(model->index(0, 1));
}

void MainWindow::on_le_ContractsFilter_editingFinished()
{   LOG_CALL;
    static QString lastFilter {qsl("init")};
    if( lastFilter == ui->le_ContractsFilter->text())
        return;

    lastFilter =ui->le_ContractsFilter->text();

    prepare_contracts_list_view ();
}
void MainWindow::on_reset_contracts_filter_clicked()
{   LOG_CALL;
    ui->le_ContractsFilter->setText(QString());
    on_le_ContractsFilter_editingFinished ();
}
void MainWindow::on_btnVertragsSpalten_clicked()
{
    QString storageName;
    int colCount =0;
    const QVector<tableViewColTexts>* colTexts =nullptr;
    if( showDeletedContracts) {
        storageName =visibilityPattern_d_MetaInfoName;
        colCount =cp_d_colCount;
        colTexts =&columnTexts_d_Contracts;
    } else {
       storageName =visibilityPatternMetaInfoName;
       colCount =cp_colCount;
       colTexts =&columnTextsContracts;
    }

    QVector<QPair<int, QString>> colInfo;
    for( int i=0; i < colCount; i++){
        colInfo.push_back (QPair<int, QString>(i, (*colTexts)[i].header));
    }
    QBitArray ba =toQBitArray(getMetaInfo (storageName, (showDeletedContracts ? defaultVisibilityPattern_deletedContracts : defaultVisibilityPattern_contracts)));
    if( ba.size() not_eq colCount) {
        ba.resize(colCount); // force right length of QBitArray
        ba.fill(true, ba.size()-1, colCount); // switch new elements to visible
    }

    dlgDisplayColumns dlg(colInfo, ba, getMainWindow ());
    if( dlg.exec () == QDialog::Accepted ) {
        setMetaInfo (storageName, toString(dlg.getNewStatus()));
        showDeletedContracts ? prepare_deleted_contracts_list_view ()
                             : prepare_valid_contracts_list_view ();
    }
}

void MainWindow::currentChange_ctv(const QModelIndex & newI, const QModelIndex & )
{
//// every time the selection in the contract tree view changes
//// the bookings table below needs to be updated
    busycursor bc;
    // todo: do all init only once, this function should only do the
    // setFilter and the select()
    QModelIndex indexIndex = newI.siblingAtColumn(0);
    int index =ui->contractsTableView->model()->data(indexIndex).toInt();
    QSqlTableModel* model = new QSqlTableModel(this);
    if( showDeletedContracts) {
        model->setTable(tn_ExBuchungen);
        model->setFilter(qsl("%1.%2=%3").arg(tn_ExBuchungen, fn_bVertragsId, i2s(index)));
    } else {
        model->setTable(tn_Buchungen);
        model->setFilter(qsl("%1.%2=%3").arg(tn_Buchungen, fn_bVertragsId, i2s(index)));
    }
    model->setSort(0, Qt::SortOrder::DescendingOrder);

    ui->bookingsTableView->setModel(model);
    model->select();
    ui->bookingsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->bookingsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bookingsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bookingsTableView->setAlternatingRowColors(true);
    ui->bookingsTableView->setSortingEnabled(false);
    ui->bookingsTableView->hideColumn(0);
    ui->bookingsTableView->hideColumn(1);
    ui->bookingsTableView->setItemDelegateForColumn(2, new DateItemFormatter);
    ui->bookingsTableView->setItemDelegateForColumn(3, new bookingTypeFormatter);
    ui->bookingsTableView->setItemDelegateForColumn(4, new BookingAmountItemFormatter);
    ui->bookingsTableView->hideColumn(5);
    ui->bookingsTableView->resizeColumnsToContents();
}

/////////////////////////////////////////////////
// contract list context menu
/////////////////////////////////////////////////

// this ugly global makes the contract, on which the context menu was called,
// available in the handler functions
namespace {
contract* contractUnderMouseMenu =nullptr;
}

void MainWindow::on_contractsTableView_customContextMenuRequested(QPoint pos)
{   LOG_CALL;
    busycursor bc;
    QTableView*& tv = ui->contractsTableView;
    QModelIndex index = tv->indexAt(pos).siblingAtColumn(0);
    if( not index.isValid ())
        return;

    int contractId = index.data().toInt();
    contract c;
    contractUnderMouseMenu = &c;
    QMenu menu( qsl("ContractContextMenu"), this);
    if (showDeletedContracts) {
        c.loadExFromDb (contractId);
        menu.addAction(ui->action_cmenu_contracts_EndLetter);
        menu.addAction (ui->action_cmenu_contracts_delete_deleted);
        bc.finish ();
        menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
    } else {
        c.loadFromDb (contractId);
        if(c.initialPaymentReceived())
        {
            menu.addAction (ui->action_cmenu_change_label);
            menu.addAction (ui->action_cmenu_change_conclusion_date);
            if( c.noBookingButInitial ())
                menu.addAction (ui->action_cmenu_change_initialPayment_date);
            if( c.hasEndDate ())
                ui->action_cmenu_terminate_contract->setText(qsl("Vertrag beenden"));
            else
                ui->action_cmenu_terminate_contract->setText(qsl("Vertrag kündigen"));
            menu.addAction(ui->action_cmenu_terminate_contract);
            menu.addAction (ui->action_cmenu_undo_last_booking);
            menu.addAction(ui->action_cmenu_change_contract);
            // interest activation only after init.payment (not enforced by contract::)
            if( not c.interestActive())
                menu.addAction(ui->action_cmenu_activate_interest_payment);
        }
        else
        {
            menu.addAction (ui->action_cmenu_change_conclusion_date);
            menu.addAction(ui->action_cmenu_activate_contract);
            menu.addAction(ui->action_cmenu_delete_inactive_contract); // passive Verträge können gelöscht werden
        }
        menu.addAction(ui->action_cmenu_changeContractTermination);
        menu.addAction(ui->action_cmenu_Anmerkung_aendern);
        menu.addAction(ui->action_cmenu_assoc_investment);

        bc.finish ();
        menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
    }
    contractUnderMouseMenu =nullptr;
    return;
//////////////////////////////////////

}

void MainWindow::on_action_cmenu_change_conclusion_date_triggered()
{   LOG_CALL;
    changeContractDate(contractUnderMouseMenu);
    updateViews();
}

void MainWindow::on_action_cmenu_change_label_triggered()
{   LOG_CALL;
    changeContractLabel(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_change_initialPayment_date_triggered()
{   LOG_CALL;
    changeInitialPaymentDate(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_activate_contract_triggered()
{   LOG_CALL;
    receiveInitialBooking(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_terminate_contract_triggered()
{   LOG_CALL;
    terminateContract(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_delete_inactive_contract_triggered()
{   LOG_CALL;
    deleteInactiveContract (contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_change_contract_triggered()
{   LOG_CALL;
    changeContractValue(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_undo_last_booking_triggered()
{   LOG_CALL;
    undoLastBooking(contractUnderMouseMenu);
    updateViews ();
}
void MainWindow::on_action_cmenu_activate_interest_payment_triggered()
{   LOG_CALL;
    activateInterest(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_Anmerkung_aendern_triggered()
{   LOG_CALL;
    changeContractComment(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_changeContractTermination_triggered()
{   LOG_CALL;
    changeContractTermination(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_contracts_EndLetter_triggered()
{
    LOG_CALL;
    finalizeContractLetter(contractUnderMouseMenu);
    updateViews();
}
void MainWindow::on_action_cmenu_contracts_delete_deleted_triggered()
{   LOG_CALL;
    deleteFinalizedContract(contractUnderMouseMenu);
    updateViews();
}

void MainWindow::on_action_cmenu_assoc_investment_triggered()
{   LOG_CALL;
    QVector<investment> invests =openInvestments(contractUnderMouseMenu->dbInterest (), contractUnderMouseMenu->conclusionDate());
    switch (invests.size())
    {
    case 0:
    {
        if( QMessageBox::Yes not_eq
            QMessageBox::question(this, qsl("Fehler"), qsl("Keine Geldanlage passt zu Zins und Vertragsdatum dieses Vertrags. Möchtest Du eine Anlage anlegen?")))
            return;
        int interest{contractUnderMouseMenu->dbInterest ()};
        QDate from (contractUnderMouseMenu->conclusionDate().addDays(-1));
        QDate to;
        qlonglong newIId =createInvestment_matchingContract(interest, from, to);
        if(newIId <= 0) break;
        if( investment(SQLITE_invalidRowId, interest, from, to).matchesContract(*contractUnderMouseMenu)) {
            if( not contractUnderMouseMenu->updateInvestment(newIId))
                QMessageBox::information(this, qsl("Fehler"), qsl("Die Geldanlage konnte dem Vertrag nicht zugewiesen werden! \n Weiterführende Info können in der LoG Datei gefunden werden."));
        } else
            QMessageBox::information(this, qsl("Fehler"), qsl("Die angelegte Geldanlage passt nicht zu dem ausgewählten Vertrag \n Weiterführende Info können in der LoG Datei gefunden werden."));
        break;
    }
    case 1:
    {
        if(isValidRowId (contractUnderMouseMenu->investmentId ()))
            QMessageBox::information(this, qsl("Erfolg"), qsl("Die Geldanlage mit dem passenden Zins und Vertragsdatum ist bereits zugewiesen"));
        else if( contractUnderMouseMenu->updateInvestment(invests[0].rowid))
            QMessageBox::information(this, qsl("Erfolg"), qsl("Die Geldanlage mit dem passenden Zins und Vertragsdatum wurde zugewiesen"));
        else
            QMessageBox::information(this, qsl("Fehler"), qsl("Die einzige Geldanlage konnte nicht zugewiesen werden. Schau bitte in die Log Datei."));
        break;
    }
    default:
    {
        QInputDialog id(this);
        id.setLabelText (qsl("Wähle die gewünschte Geldanlage aus der Liste aus"));
        id.setComboBoxEditable(false);
        QStringList iList;
        for( const auto& inv: qAsConst(invests)) {
            iList.push_back(inv.toString());
        }
        id.setComboBoxItems(iList);
        QComboBox* cb =id.findChild<QComboBox*>();
        int i =0;
        for( const auto& inv: qAsConst(invests)) {
            cb->setItemData(i++, inv.rowid);
        }
        qInfo() << cb;
        if( id.exec() not_eq QDialog::Accepted) {
            qInfo() << "selection of investment aborted";
            return;
        }
        int rowId =cb->itemData(cb->currentIndex()).toInt();
        if( not contractUnderMouseMenu->updateInvestment(rowId)) {
            qCritical() << "failed to save investment to contract" << contractUnderMouseMenu->toString() << "\n" << rowId;
        }
        break;
    }
    }; // Eo switch
    updateViews();
}

void MainWindow::on_btnSave2Csv_clicked()
{
    csvwriter csv;
    QSqlTableModel* model = (showDeletedContracts ?
           qobject_cast<QSqlTableModel*>( ui->contractsTableView->model())
            :
           qobject_cast<QSqlTableModel*>(
               qobject_cast<QAbstractProxyModel *>(ui->contractsTableView->model())->sourceModel()));
    if (model == nullptr)
        return;
    QSqlRecord rec =model->record();

    QBitArray ba =(showDeletedContracts ?
                       toQBitArray(getMetaInfo (visibilityPattern_d_MetaInfoName, defaultVisibilityPattern_deletedContracts))
                      : toQBitArray(getMetaInfo (visibilityPatternMetaInfoName, defaultVisibilityPattern_contracts)));
    // header
    for( int i=0; i<rec.count(); i++) {
        if( ba.at(i))
            csv.addColumn(rec.fieldName(i));
    }
    // data
    for( int i=0; i<model->rowCount(); i++) {
        QSqlRecord recSingleRow =model->record(i);
        for( int j=0; j<recSingleRow.count(); j++) {
            if( ba.at(j)) {
                QVariant v =recSingleRow.value (j);
                QVariant tmp(v);
                if( tmp.canConvert (QVariant::Double) && tmp.convert (QVariant::Double))
                    csv.appendToRow( QLocale().toString(tmp.toDouble (), 'f', 2));
                else
                    csv.appendToRow(recSingleRow.value(j).toString());
            }
        }
    }
    csv.saveAndShowInExplorer((showDeletedContracts
                               ? QDate::currentDate().toString(qsl("yyyy-MM-dd") +qsl("_beendeteVertraege.csv"))
                               : QDate::currentDate().toString(qsl("yyyy-MM-dd") +qsl("_laufendeVertraege.csv"))));
}

/////////////////////////////////////////////////
// new creditor or contract from contract menu
/////////////////////////////////////////////////
void MainWindow::on_action_Neu_triggered()
{
    on_actionNeu_triggered();
    updateViews();
}
