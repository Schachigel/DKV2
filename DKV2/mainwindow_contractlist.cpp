#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "contracttablemodel.h"
#include "uiitemformatter.h"
#include "dkdbviews.h"
#include "investment.h"
#include "transaktionen.h"
#include "helper.h"

// Contract List
void MainWindow::on_action_menu_contracts_listview_triggered()
{   LOG_CALL;
    showDeletedContracts =false;
    prepare_contracts_list_view();
    if( not ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
QString filterFromFilterphrase(const QString &fph)
{
    if( fph.startsWith(qsl("kreditor:")))
    {
        bool conversionOK = true;
        qlonglong contractId = fph.rightRef(fph.length()-9).toInt(&conversionOK);
        if( not conversionOK)
            return QString();
        else
            return qsl("KreditorId=") + QString::number(contractId);
    }
    return fph.isEmpty() ? QString() :
                           (qsl("Kreditorin LIKE '%") + fph
                            + qsl("%' OR Vertragskennung LIKE '%") + fph
                            + qsl("%' OR Zinssatz LIKE '%") +fph +"%'");
}
void MainWindow::prepare_valid_contracts_list_view()
{ LOG_CALL;
    QSqlTableModel* model = new ContractTableModel(this);
    model->setTable(vnContractView);
    model->setFilter( filterFromFilterphrase(ui->le_ContractsFilter->text()));
    model->setHeaderData(cp_Creditor, Qt::Horizontal, qsl("KreditorIn"));
    model->setHeaderData(cp_Creditor, Qt::Horizontal, qsl("Nachname, Vorname der Vertragspartnerin / des Vertragsparnters"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractLabel, Qt::Horizontal, qsl("Vertragskennung"));
    model->setHeaderData(cp_ContractLabel, Qt::Horizontal, qsl("Die Vertragskennung identifiziert den Vertrag eindeutig"), Qt::ToolTipRole);
    model->setHeaderData(cp_Comment, Qt::Horizontal, qsl("Anmerkung"));
    model->setHeaderData(cp_Comment, Qt::Horizontal, qsl("Freitext zum Vertrag"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractDate, Qt::Horizontal, qsl(""), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractValue, Qt::Horizontal, qsl("Bei aktiven Verträgen: Höhe der Ersteinlage, sonst der im Vertrag vereinbarte Kreditbetrag"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestRate, Qt::Horizontal, qsl(""), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestMode, Qt::Horizontal, qsl("Verträge können Auszahlend, Thesaurierend oder mit festem Zins vereinbart sein"), Qt::ToolTipRole);
    model->setHeaderData(cp_ActivationDate, Qt::Horizontal, qsl("Datum des ersten Geldeingangs und Beginn der Zinsberechnung"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestBearing, Qt::Horizontal, qsl("Verzinsliches\nGuthaben"));
    model->setHeaderData(cp_InterestBearing, Qt::Horizontal, qsl("Bei thesaurierenden Verträgen: Einlage und angesparte Zinsen"), Qt::ToolTipRole);
    model->setHeaderData(cp_Interest, Qt::Horizontal, qsl("Angesparter\nZins"));
    model->setHeaderData(cp_Interest, Qt::Horizontal, qsl("Nicht ausgezahlte Zinsen bei Verträgen mit fester Verzinsung und thesaurierenden Verträgen"), Qt::ToolTipRole);
    model->setHeaderData(cp_LastBooking, Qt::Horizontal, qsl("Letztes\nBuchungsdatum"));
    model->setHeaderData(cp_ContractEnd, Qt::Horizontal, qsl("Kündigungsfrist/ \nVertragsende"));

    qDebug() << "contract list model filter: " << model->filter();

    QTableView*& tv = ui->contractsTableView;
    tv->setModel(model);
    if ( not model->select()) {
        qCritical() << "Model selection failed" << model->lastError();
        return;
    } else {
        qobject_cast<ContractTableModel*>(model)->setCol13ExtraData();
    }

    tv->setEditTriggers(QTableView::NoEditTriggers);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setAlternatingRowColors(true);
    tv->setSortingEnabled(true);
    tv->setItemDelegateForColumn(cp_ActivationDate, new DateItemFormatter);
    tv->setItemDelegateForColumn(cp_ContractDate, new DateItemFormatter);
    tv->setItemDelegateForColumn(cp_LastBooking, new DateItemFormatter);
    tv->setItemDelegateForColumn(cp_ContractValue, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_InterestBearing, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_Interest, new CurrencyFormatter(tv));
    tv->setItemDelegateForColumn(cp_InterestMode, new interestModeFormatter(tv));
    tv->hideColumn(cp_vid);
    tv->hideColumn(cp_Creditor_id);


    tv->resizeColumnsToContents();
    tv->resizeRowsToContents();

    connect(ui->contractsTableView->selectionModel(), &QItemSelectionModel::currentChanged, this ,&MainWindow::currentChange_ctv);

    if( not model->rowCount()) {
        ui->bookingsTableView->setModel(new QSqlTableModel(this));
    } else
        tv->setCurrentIndex(model->index(0, 1));
}
void MainWindow::prepare_deleted_contracts_list_view()
{ LOG_CALL;
    enum column_pos_del {
        cp_vid,
        cp_Creditor_id,
        cp_Creditor,
        cp_ContractLabel,
        cp_ContractActivation,
        cp_ContractTermination,
        cp_InitialValue,
        cp_InterestRate,
        cp_InterestMode,
        cp_Interest,
        cp_TotalDeposit,
        cp_FinalPayout
    };

    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable(qsl("vVertraege_geloescht"));
    model->setFilter( filterFromFilterphrase(ui->le_ContractsFilter->text()));
    qDebug() << "contract list model filter: " << model->filter();

    model->setHeaderData(cp_Creditor, Qt::Horizontal, qsl("Nachname, Vorname der Vertragspartnerin / des Vertragsparnters"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractLabel, Qt::Horizontal, qsl("Eindeutige Identifizierung des Vertrags"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractActivation, Qt::Horizontal, qsl("Datum der Vertragsaktivierung / Beginn der Zinsberechnung"), Qt::ToolTipRole);
    model->setHeaderData(cp_ContractTermination, Qt::Horizontal, qsl("Datum des Vertragsende"), Qt::ToolTipRole);
    model->setHeaderData(cp_InitialValue, Qt::Horizontal, qsl("Höhe der Ersteinlage"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestRate, Qt::Horizontal, qsl("Zinsfuss"), Qt::ToolTipRole);
    model->setHeaderData(cp_InterestMode, Qt::Horizontal, qsl("Verträge können Auszahlend, Thesaurierend oder mit festem Zins vereinbart sein"), Qt::ToolTipRole);
    model->setHeaderData(cp_Interest, Qt::Horizontal, qsl("Angefallene, nicht bereits zur Laufzeit ausgezahlte Zinsen"), Qt::ToolTipRole);
    model->setHeaderData(cp_TotalDeposit, Qt::Horizontal, qsl("Summe aller Einzahlungen"), Qt::ToolTipRole);
    model->setHeaderData(cp_FinalPayout, Qt::Horizontal, qsl("Ausgezahltes finales Guthaben"), Qt::ToolTipRole);

    QTableView*& contractsTV = ui->contractsTableView;
    contractsTV->setModel(model);
    if ( not model->select()) {
        qCritical() << "Model selection failed" << model->lastError();
        return;
    }
    contractsTV->setEditTriggers(QTableView::NoEditTriggers);
    contractsTV->setSelectionMode(QAbstractItemView::SingleSelection);
    contractsTV->setSelectionBehavior(QAbstractItemView::SelectRows);
    contractsTV->setAlternatingRowColors(true);
    contractsTV->setSortingEnabled(true);
    contractsTV->setItemDelegateForColumn(cp_InitialValue, new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_Interest, new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_TotalDeposit, new CurrencyFormatter(contractsTV));
    contractsTV->setItemDelegateForColumn(cp_FinalPayout, new CurrencyFormatter(contractsTV));
    contractsTV->hideColumn(cp_vid);
    contractsTV->hideColumn(cp_Creditor_id);

    contractsTV->resizeColumnsToContents();

    connect(ui->contractsTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::currentChange_ctv);

    if( not model->rowCount()) {
        ui->bookingsTableView->setModel(new QSqlTableModel(this));
    } else
        contractsTV->setCurrentIndex(model->index(0, 1));
}
void MainWindow::prepare_contracts_list_view()
{   LOG_CALL;
    busycursor b;

    if( showDeletedContracts)
        prepare_deleted_contracts_list_view();
    else
        prepare_valid_contracts_list_view();
}
int  MainWindow::get_current_id_from_contracts_list()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid()) {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}
void MainWindow::on_le_ContractsFilter_editingFinished()
{   LOG_CALL;
    prepare_contracts_list_view();
}
void MainWindow::on_reset_contracts_filter_clicked()
{   LOG_CALL;
    ui->le_ContractsFilter->setText(QString());
    prepare_contracts_list_view();
}
void MainWindow::currentChange_ctv(const QModelIndex & newI, const QModelIndex & )
{
    // todo: do all init only once, this function should only do the
    // setFilter and the select()
    QModelIndex indexIndex = newI.siblingAtColumn(0);
    int index =ui->contractsTableView->model()->data(indexIndex).toInt();
    QSqlTableModel* model = new QSqlTableModel(this);
    if( showDeletedContracts) {
        model->setTable(qsl("exBuchungen"));
        model->setFilter(qsl("exBuchungen.VertragsId=") + QString::number(index));
    } else {
        model->setTable(qsl("Buchungen"));
        model->setFilter(qsl("Buchungen.VertragsId=") + QString::number(index));
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
void MainWindow::on_contractsTableView_customContextMenuRequested(QPoint pos)
{   LOG_CALL;
    if( showDeletedContracts)
        return;
    QTableView*& tv = ui->contractsTableView;
    QModelIndex index = tv->indexAt(pos).siblingAtColumn(0);

    contract c(index.data().toInt());
    bool gotTerminationDate = c.noticePeriod() == -1;

    QMenu menu( qsl("ContractContextMenu"), this);
    if(c.isActive())
    {
        if( gotTerminationDate)
            ui->action_cmenu_terminate_contract->setText(qsl("Vertrag beenden"));
        else
            ui->action_cmenu_terminate_contract->setText(qsl("Vertrag kündigen"));
        menu.addAction(ui->action_cmenu_terminate_contract);
        menu.addAction(ui->action_cmenu_change_contract);
    }
    else
    {
        menu.addAction(ui->action_cmenu_activate_contract);
        menu.addAction(ui->action_cmenu_delete_inactive_contract); // passive Verträge können gelöscht werden
    }
    menu.addAction(ui->action_cmenu_changeContractTermination);
    menu.addAction(ui->action_cmenu_Anmerkung_aendern);
    menu.addAction(ui->action_cmenu_assoc_investment);
    menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
    return;
}
void MainWindow::on_action_cmenu_activate_contract_triggered()
{   LOG_CALL;
    activateContract(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_terminate_contract_triggered()
{   LOG_CALL;
    terminateContract(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_delete_inactive_contract_triggered()
{   LOG_CALL;
    deleteInactiveContract(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_change_contract_triggered()
{   LOG_CALL;
    changeContractValue(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_Anmerkung_aendern_triggered()
{   LOG_CALL;
    changeContractComment(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_changeContractTermination_triggered()
{   LOG_CALL;
    changeContractTermination(get_current_id_from_contracts_list());
    updateListViews();
}
void MainWindow::on_action_cmenu_assoc_investment_triggered()
{   LOG_CALL;
    contract c(get_current_id_from_contracts_list());
    QVector<investment> invests =openInvestments(d2percent(c.interestRate()), c.conclusionDate());
    switch (invests.size())
    {
    case 0:
    {
        if( QMessageBox::Yes not_eq
            QMessageBox::question(this, qsl("Fehler"), qsl("Keine Geldanlage passt zu Zins und Vertragsdatum dieses Vertrags. Möchtest Du eine Anlage anlegen?")))
            return;
        int interest{d2percent(c.interestRate())};
        QDate from (c.conclusionDate());
        QDate to;
        qlonglong newIId =createInvestment(interest, from, to);
        if(newIId <= 0) break;
        if( investment(-1, interest, from, to).matchesContract(c)) {
            if( not c.updateInvestment(newIId))
                QMessageBox::information(this, qsl("Fehler"), qsl("Die Geldanlage konnte dem Vertrag nicht zugewiesen werden! \n Weiterführende Info können in der LoG Datei gefunden werden."));
        } else
            QMessageBox::information(this, qsl("Fehler"), qsl("Die angelegte Geldanlage passt nicht zu dem ausgewählten Vertrag \n Weiterführende Info können in der LoG Datei gefunden werden."));
        break;
    }
    case 1:
    {
        if( c.updateInvestment(invests[0].rowid))
            QMessageBox::information(this, qsl("Erfolg"), qsl("Die Geldanlage mit dem passenden Zins und Vertragsdatum wurde zugewiesen"));
        else
            QMessageBox::information(this, qsl("Fehler"), qsl("Die einzige Geldanlage konnte nicht zugewiesen werden. Schau bitte in die Log Datei."));
        break;
    }
    default:
    {
        QInputDialog id(this);
        QFont f=id.font(); f.setPointSize(10); id.setFont(f);
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
        if( not c.updateInvestment(rowId)) {
            qDebug() << "failed to save investment to contract" << c.toString() << "\n" << rowId;
        }
        break;
    }
    }; // Eo switch
    updateListViews();
}

/////////////////////////////////////////////////
// new creditor or contract from contract menu
/////////////////////////////////////////////////
void MainWindow::on_action_Neu_triggered()
{
    on_actionNeu_triggered();
}

/////////////////////////////////////////////////
// terminated contracts list
/////////////////////////////////////////////////
void MainWindow::on_actionBeendete_Vertr_ge_anzeigen_triggered()
{
    showDeletedContracts =true;
    prepare_contracts_list_view();
    if( not ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);
    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
