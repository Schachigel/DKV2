#include <QtGlobal>
#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif

#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlRelationalTableModel>
#include <QPdfWriter>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "appconfig.h"
#include "wiznewdatabase.h"
#include "appconfig.h"
#include "uiitemformatter.h"
#include "dkdbhelper.h"
#include "letters.h"
#include "transaktionen.h"

// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   LOG_CALL;
    ui->setupUi(this);
#ifndef QT_DEBUG
    ui->action_menu_debug_create_sample_data->setVisible(false);
#endif

    ui->leBetrag->setValidator(new QIntValidator(0,999999,this));
    ui->statusBar->addPermanentWidget(ui->statusLabel);

    setCentralWidget(ui->stackedWidget);
    if( appConfig::CurrentDb().isEmpty())
        on_action_menu_database_new_triggered();
    if( !useDb(appConfig::CurrentDb()))
        // there should be a valid DB - checked in main.cpp
        Q_ASSERT(!"useDb failed in construcor of mainwindow");

    ui->txtAnmerkung->setTabChangesFocus(true);
    ui->CreditorsTableView->setStyleSheet(qsl("QTableView::item { padding-right: 10px; padding-left: 10px; }"));
    ui->contractsTableView->setStyleSheet(qsl("QTableView::item { padding-right: 10px; padding-left: 10px; }"));

    ui->bookingsTableView->setItemDelegateForColumn(2, new bookingTypeFormatter);

    fillCombo_NoticePeriods();
    createButtonMenu_saveCreditorAnd();
    createBtnMenu_saveContractAnd();

    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
MainWindow::~MainWindow()
{   LOG_CALL;
    delete ui;
}

// generell functions
bool MainWindow::useDb(const QString& dbfile)
{   LOG_CALL;
    if( open_databaseForApplication(dbfile)) {
        appConfig::setCurrentDb(dbfile);
        showDbInStatusbar(dbfile);
        return true;
    }
    qCritical() << "the databse could not be used for this application";
    return false;
}
void MainWindow::showDbInStatusbar( QString filename)
{   LOG_CALL_W (filename);
    if( filename.isEmpty()) {
        filename = appConfig::CurrentDb();
    }
    ui->statusLabel->setText( filename);
}

// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{   LOG_CALL;
    if( arg1 < 0) {
        qWarning() << "stackedWidget changed to non existing page";
        return;
    }
    switch(arg1)
    {
    case startPageIndex:
        prepare_startPage();
        ui->action_menu_creditors_delete->setEnabled(false);
        break;
    case creditorsListPageIndex:
        ui->action_menu_creditors_delete->setEnabled(true);
        break;
    case newCreditorPageIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        break;
    case newContractPageIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        break;
    case contractsListPageIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        break;
    case bookingsListIndex:
        ui->action_menu_creditors_delete->setEnabled(false);
        break;
    default:
        qWarning() << "stackedWidget current change not implemented for this index";
    }// e.o. switch
    return;
}

// the empty "welcome" page
void MainWindow::prepare_startPage()
{   LOG_CALL;
    busycursor b;
    QString messageHtml {qsl( "<table width='100%'><tr><td><h2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</h2></td></tr>")};
    DbSummary dbs =calculateSummary();
    double allContracts = dbs.WertAktive + dbs.BetragPassive;
    if( allContracts > 0) {
        QLocale l;
        QString valueRow = qsl("<tr><td>Die Summer aller DK beträgt <big><font color=red>") + l.toCurrencyString(allContracts) + qsl("</font></big></td></tr>");
        messageHtml += valueRow;
    }
    messageHtml += qsl("<tr><td><img src=\":/res/splash.png\"/></td></tr></table>");
    qDebug() <<"welcome Screen html: " << Qt::endl << messageHtml << Qt::endl;
    ui->teWelcome->setText(messageHtml);
}
void MainWindow::on_action_menu_database_start_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}

// Database Menu
QString askUserDbFilename(QString title, bool onlyExistingFiles=false)
{   // this function is used with openDb, creaetDbCopy, createAnony.DbCopy but NOT newDb
    LOG_CALL;
    wizFileSelectionWiz wiz(getMainWindow());
    wiz.title =title;
    wiz.subtitle =qsl("Mit dieser Dialogfolge wählst Du eine DKV2 Datenbank aus");
    wiz.fileTypeDescription =qsl("dk-DB Dateien (*.dkdb)");
    if( (wiz.existingFile=onlyExistingFiles))
        wiz.bffTitle =qsl("Wähle eine existierende dkdb Datei aus");
    else
        wiz.bffTitle =qsl("Wähle eine dkdb Datei aus oder gib einen neuen Dateinamen ein");

    QFileInfo lastdb (appConfig::CurrentDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::AppDataLocation));

    wiz.exec();
    return wiz.field(qsl("selectedFile")).toString();
}
QString askUserNewDb()
{   LOG_CALL;
    wizNewDatabaseWiz wiz(getMainWindow());
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    QFileInfo lastdb (appConfig::CurrentDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::AppDataLocation));
    wiz.title = qsl("Neue DKV2 Datenbank Datei");
    if( wiz.exec() == QDialog::Accepted)
        wiz.updateDbConfig();

    return wiz.field(qsl("selectedFile")).toString();
}
void MainWindow::on_action_menu_database_new_triggered()
{   LOG_CALL;
    QString dbfile = askUserNewDb();
    if( dbfile == qsl("")) {
        qDebug() << "user canceled file selection";
        return;
    }
    busycursor b;
    closeDatabaseConnection();
    if( create_DK_databaseFile(dbfile) && useDb(dbfile)) {
        appConfig::setLastDb(dbfile);
    }
    else {
        QMessageBox::information(this, qsl("Fehler"), qsl("Die neue Datenbankdatei konnte nicht angelegt und geöffnet werden."));
        return;
    }
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
void MainWindow::on_action_menu_database_open_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename(qsl("DKV2 Datenbank zum Öffnen auswählen."), true);
    if( dbfile.isEmpty()) {
        qDebug() << "keine Datei wurde vom Anwender ausgewählt";
        QMessageBox::information(this, qsl("Abbruch"), qsl("Es wurde keine Datenbankdatei ausgewählt"));
        return;
    }
    busycursor b;
    if( useDb(dbfile))
        appConfig::setLastDb(dbfile);
    else {
        QMessageBox::information(this, qsl("Fehler"), qsl("Die Datenbank konnte nicht geöffnet werden"));
        if( !useDb(appConfig::CurrentDb())) {
            qFatal("alte und neue DB können nicht geöffnet werden -> abbruch");
            exit( 1);
        }
    }

    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
void MainWindow::on_action_menu_database_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename(qsl("Dateiname der Kopie Datenbank angeben."));
    if( dbfile == qsl(""))
        return;

    busycursor b;
    if( !create_DB_copy(dbfile, false)) {
        QMessageBox::information(this, qsl("Fehler beim Kopieren"), qsl("Die Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei"));
        qCritical() << "creating copy failed";
    }
    return;
}
void MainWindow::on_action_menu_database_anonymous_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename(qsl("Dateiname der Anonymisierten Kopie angeben."));
    if( dbfile == qsl(""))
        return;
    busycursor b;
    if( !create_DB_copy(dbfile, true)) {
        QMessageBox::information(this, qsl("Fehler beim Kopieren"),
                                 qsl("Die anonymisierte Datenbankkopie konnte nicht angelegt werden. "
                                     "Weitere Info befindet sich in der LOG Datei"));
        qCritical() << "creating depersonaliced copy failed";
    }
    return;
}
void MainWindow::on_actionProjektkonfiguration_ndern_triggered()
{   LOG_CALL;
    wizConfigureProjectWiz wiz(getMainWindow());
    if(wiz.exec() == QDialog::Accepted)
        wiz.updateDbConfig();
}
void MainWindow::on_action_menu_database_configure_outdir_triggered()
{   LOG_CALL;
    appConfig::setOutDirInteractive(this);
}
void MainWindow::on_action_menu_database_program_exit_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(startPageIndex);
    this->close();
}

// Creditor list view page
void MainWindow::on_action_menu_creditors_listview_triggered()
{   LOG_CALL;
    busycursor b;
    prepare_CreditorsListPage();
    if( ! ui->CreditorsTableView->currentIndex().isValid())
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
    on_action_cmenu_delete_creaditor_triggered();
}

// Context Menue in Creditor Table
void MainWindow::on_CreditorsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;
    QModelIndex index = ui->CreditorsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid()) {
        QMenu menu( qsl("PersonContextMenu"), this);
        menu.addAction(ui->action_cmenu_edit_creditor);
        menu.addAction(ui->action_menu_contracts_create);
        menu.addAction(ui->action_cmenu_delete_creaditor);
        menu.addAction(ui->action_cmenu_go_contracts);
        menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
        return;
    }
}
void MainWindow::on_action_cmenu_edit_creditor_triggered()
{   LOG_CALL;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QVariant index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0));
    ui->lblPersId->setText(index.toString());

    init_creditor_form(index.toInt());
    ui->stackedWidget->setCurrentIndex(newCreditorPageIndex);
}
void MainWindow::on_action_cmenu_delete_creaditor_triggered()
{   LOG_CALL;
    const QTableView * const tv = ui->CreditorsTableView;
    QModelIndex mi(tv->currentIndex());
    qlonglong index = tv->model()->data(mi.siblingAtColumn(0)).toLongLong();
    creditor c (index);

    QString msg( qsl("Soll der Kreditgeber %1 %2 (id %3) gelöscht werden?"));
    msg =msg.arg(c.getValue(qsl("Vorname")).toString(), c.getValue(qsl("Nachname")).toString(), QString::number(index));

    if( QMessageBox::Yes != QMessageBox::question(this, qsl("Kreditgeber löschen?"), msg))
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
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QString index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->le_ContractsFilter->setText(qsl("kreditor:") +index);
    on_action_menu_contracts_listview_triggered();
}

// new creditor and contract Wiz
void MainWindow::on_actionNeu_triggered()
{
    newCreditorAndContract();
}

// new Creditor page
void MainWindow::on_action_menu_creditors_create_triggered()
{   LOG_CALL;
    empty_create_creditor_form();
    ui->stackedWidget->setCurrentIndex(newCreditorPageIndex);
}
void MainWindow::createButtonMenu_saveCreditorAnd()
{   LOG_CALL;
    // Kreditor anlegen: "Speichern und ..." Menü anlegen
    menuSaveKreditorAnd = new QMenu;
    menuSaveKreditorAnd->addAction(ui->action_saveCreditor_go_contract);
    menuSaveKreditorAnd->addAction(ui->action_saveCreditor_go_creditors);
    menuSaveKreditorAnd->addAction(ui->action_saveCreditor_go_new_creditor);
    ui->customBtn_saveCreditorAnd->setMenu(menuSaveKreditorAnd);
    ui->customBtn_saveCreditorAnd->setDefaultAction(ui->action_saveCreditor_go_contract);
}
int  MainWindow::save_creditor()
{   LOG_CALL;
    creditor c;
    c.setFirstname(ui->leVorname->text().trimmed());
    c.setLastname(ui->leNachname->text().trimmed());
    c.setStreet(ui->leStrasse->text().trimmed());
    c.setPostalCode(ui->lePlz->text().trimmed());
    c.setCity(ui->leStadt->text().trimmed());
    c.setEmail(ui->leEMail->text().trimmed().toLower());
    c.setComment(ui->txtAnmerkung->toPlainText());
    c.setIban(ui->leIban->text().trimmed());
    c.setBic(ui->leBic->text().trimmed());

    QString errortext;
    if( !c.isValid(errortext)) {
        errortext = qsl("Die Daten können nicht gespeichert werden: <br>") + errortext;
        QMessageBox::information(this, qsl("Fehler"), errortext );
        qDebug() << qsl("prüfung der Kreditor Daten:") << errortext;
        return -1;
    }
    int kid = -1;
    if( ! ui->lblPersId->text().isEmpty()) {
        kid = ui->lblPersId->text().toInt();
        c.setId(kid);     // update not insert
        c.update();
    }
    else
       kid = c.save();

    if(kid == -1) {
        QMessageBox::information( this, qsl("Fehler"), qsl("Der Datensatz konnte nicht gespeichert werden. "
                     "Ist die E-Mail Adresse einmalig? Gibt es die Adressdaten in der Datenbank bereits?"
                     "\nBitte überprüfen Sie ihre Eingaben"));
        qCritical() << "Kreditgeber konnte nicht gespeichert werden";
        return -1;
    }

    return passNewCreditorIdToNewContract = kid;
}
void MainWindow::empty_create_creditor_form()
{   LOG_CALL;
    QString empty;
    ui->leVorname->setText(empty);
    ui->leNachname->setText(empty);
    ui->leStrasse->setText(empty);
    ui->lePlz->setText(empty);
    ui->leStadt->setText(empty);
    ui->leEMail->setText(empty);
    ui->txtAnmerkung->setPlainText(empty);
    ui->leIban->setText(empty);
    ui->leBic->setText(empty);
    ui->lblPersId->setText(empty);
}
void MainWindow::init_creditor_form(int id)
{   LOG_CALL;
    busycursor b;
    QSqlRecord rec = executeSingleRecordSql(dkdbstructur[qsl("Kreditoren")].Fields(), qsl("Id=") +QString::number(id));
    ui->leVorname->setText(rec.field(qsl("Vorname")).value().toString());
    ui->leNachname->setText(rec.field(qsl("Nachname")).value().toString());
    ui->leStrasse->setText(rec.field(qsl("Strasse")).value().toString());
    ui->lePlz->setText(rec.field(qsl("Plz")).value().toString());
    ui->leStadt->setText(rec.field(qsl("Stadt")).value().toString());
    ui->leEMail->setText(rec.field(qsl("Email")).value().toString());
    ui->txtAnmerkung->setPlainText(rec.field(qsl("Anmerkung")).value().toString());
    ui->leIban  ->setText(rec.field(qsl("IBAN")).value().toString());
    ui->leBic  ->setText(rec.field(qsl("BIC")).value().toString());
}
void MainWindow::on_cancelCreateCreditor_clicked()
{   LOG_CALL;
    empty_create_creditor_form();
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
void MainWindow::on_action_saveCreditor_go_contract_triggered()
{   LOG_CALL;
    int kid = save_creditor();
    if(  kid != -1) {
        empty_create_creditor_form();
        on_action_menu_contracts_create_triggered();
    }
}
void MainWindow::on_action_saveCreditor_go_creditors_triggered()
{   LOG_CALL;
    if( save_creditor() != -1) {
        empty_create_creditor_form();
        on_action_menu_creditors_listview_triggered();
    }
}
void MainWindow::on_action_saveCreditor_go_new_creditor_triggered()
{   LOG_CALL;
    if( save_creditor() == -1) {
        qDebug() << "Fehler beim Speichern eines Kreditors";
        return;
    }
    on_action_menu_creditors_create_triggered();
}

// Contract List
void MainWindow::on_action_menu_contracts_listview_triggered()
{   LOG_CALL;
    showDeletedContracts =false;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
QString filterFromFilterphrase(QString fph)
{
    if( fph.startsWith(qsl("kreditor:")))
    {
        bool conversionOK = true;
        qlonglong contractId = fph.rightRef(fph.length()-9).toInt(&conversionOK);
        if( ! conversionOK)
            return "";
        else
            return qsl("KreditorId=") + QString::number(contractId);
    }
    return fph.isEmpty() ? QString() :
           (qsl("Kreditorin LIKE '%") + fph + qsl("%' OR Vertragskennung LIKE '%") + fph + qsl("%'"));
}
void MainWindow::prepare_contracts_list_view()
{   LOG_CALL;
    busycursor b;
    QSqlTableModel* model = new QSqlTableModel(this);
    if( showDeletedContracts)
        model->setTable(qsl("WertBeendeteVertraege"));
    else
        model->setTable(qsl("WertAlleVertraege"));
    model->setFilter( filterFromFilterphrase(ui->le_ContractsFilter->text()));
    qDebug() << "contract list model filter: " << model->filter();

    QTableView*& tv = ui->contractsTableView;
    tv->setModel(model);
    model->select();
    tv->setEditTriggers(QTableView::NoEditTriggers);
    tv->setSelectionMode(QAbstractItemView::SingleSelection);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setAlternatingRowColors(true);
    tv->setSortingEnabled(true);
    tv->hideColumn(0);
    tv->hideColumn(9);
    tv->setItemDelegateForColumn(3, new PercentItemFormatter(tv));
    tv->setItemDelegateForColumn(4, new ContractValueItemFormatter(tv));
    tv->setItemDelegateForColumn(5, new DateItemFormatter(tv));
    tv->setItemDelegateForColumn(6, new KFristItemFormatter(tv));
    tv->setItemDelegateForColumn(7, new DateItemFormatter(tv));
    tv->setItemDelegateForColumn(8, new thesaItemFormatter(tv));

    tv->resizeColumnsToContents();
    auto c = connect(ui->contractsTableView->selectionModel(),
            SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
            SLOT(currentChange_ctv(const QModelIndex & , const QModelIndex & )));

    if( ! model->rowCount()) {
        ui->bookingsTableView->setModel(nullptr);
    } else
        tv->setCurrentIndex(model->index(0, 1));
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
    model->setSort(0, Qt::SortOrder::AscendingOrder);

    ui->bookingsTableView->setModel(model);
    model->select();
    ui->bookingsTableView->setSortingEnabled(false);
    ui->bookingsTableView->hideColumn(0);
    ui->bookingsTableView->hideColumn(1);
    ui->bookingsTableView->setItemDelegateForColumn(2, new DateItemFormatter);
    ui->bookingsTableView->setItemDelegateForColumn(3, new bookingTypeFormatter);
    ui->bookingsTableView->setItemDelegateForColumn(4, new BookingAmountItemFormatter);
}

// contract list context menu
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;
    if( showDeletedContracts)
        return;
    QTableView*& tv = ui->contractsTableView;
    QModelIndex index = tv->indexAt(pos).siblingAtColumn(0);

    contract c(index.data().toInt());
    bool hatLaufzeitende = c.noticePeriod() == -1;

    QMenu menu( qsl("ContractContextMenu"), this);
    if(c.isActive())
    {
        if( hatLaufzeitende)
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
    menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
    return;
}
void MainWindow::on_action_cmenu_activate_contract_triggered()
{   LOG_CALL;
    activateContract(get_current_id_from_contracts_list());
    prepare_contracts_list_view();
}
void MainWindow::on_action_cmenu_terminate_contract_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    int index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toInt();
    terminateContract(index);

    prepare_contracts_list_view();
}
void MainWindow::on_action_cmenu_delete_inactive_contract_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;

    deleteInactiveContract(ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toLongLong());
    prepare_contracts_list_view();
}
void MainWindow::on_action_cmenu_change_contract_triggered()
{   // deposit or payout...
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    qlonglong contractId = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toLongLong();
    changeContractValue(contractId);
    on_action_menu_contracts_listview_triggered();
}
// new creditor or contract from contract menu
void MainWindow::on_action_Neu_triggered()
{
    newCreditorAndContract();
}
// terminated contracts list
void MainWindow::on_actionBeendete_Vertr_ge_anzeigen_triggered()
{
    showDeletedContracts =true;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);
    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
// new Contract helper
void MainWindow::createBtnMenu_saveContractAnd()
{   LOG_CALL;
    // Vertrag anlegen: "Speichern und ... " Menü anlegen
    menuSaveContractAnd = new QMenu;
    menuSaveContractAnd->addAction(ui->action_save_contract_new_contract);
    menuSaveContractAnd->addAction(ui->action_save_contract_go_kreditors);
    menuSaveContractAnd->addAction(ui->action_save_contract_go_contracts);
    ui->saveContractAnd->setMenu(menuSaveContractAnd);
    ui->saveContractAnd->setDefaultAction(ui->action_save_contract_go_kreditors);
}
void MainWindow::fillCombo_NoticePeriods()
{   LOG_CALL;
    // combo box für Kündigungsfristen füllen
    ui->cbKFrist->addItem(qsl("festes Vertragsende"), QVariant(-1));
    for (int i=3; i<12; i++)
        ui->cbKFrist->addItem(QString::number(i) + qsl(" Monate"), QVariant(i));
    ui->cbKFrist->addItem(qsl("1 Jahr"), QVariant(12));
    ui->cbKFrist->addItem(qsl("1 Jahr und 1 Monat"), QVariant(13));
    for (int i=14; i<24; i++)
        ui->cbKFrist->addItem(qsl("1 Jahr und ") + QString::number( i-12) + qsl(" Monate"), QVariant(i));
    ui->cbKFrist->addItem(qsl("2 Jahre"), QVariant(24));
}
// new contract
void MainWindow::on_action_menu_contracts_create_triggered()
{   LOG_CALL;
    busycursor b;
    fill_creditors_dropdown();
    fill_rates_dropdown();
    ui->leKennung->setText( proposeContractLabel());
    if( ui->stackedWidget->currentIndex() == creditorsListPageIndex)
        set_creditors_combo_by_id(id_SelectedCreditor());
    if( ui->stackedWidget->currentIndex() == newCreditorPageIndex)
        set_creditors_combo_by_id(passNewCreditorIdToNewContract);
    else
        set_creditors_combo_by_id(-1);
    contract cd; // this is to get the defaults of the class definition
    ui->deLaufzeitEnde->setDate(cd.plannedEndDate());
    int cbkFristStartIndex = ui->cbKFrist->findText(qsl("6 Monate"));
    if( cbkFristStartIndex <0)
        qDebug() << "error setting kFrist Combo default value";
    else
        ui->cbKFrist->setCurrentIndex(cbkFristStartIndex);
    ui->deVertragsabschluss->setDate(cd.conclusionDate());
    ui->chkbThesaurierend->setChecked(cd.reinvesting());

    ui->stackedWidget->setCurrentIndex(newContractPageIndex);
}
contract MainWindow::get_contract_data_from_form()
{   LOG_CALL;
    contract c;
    c.setCreditorId (ui->comboKreditoren->itemData(ui->comboKreditoren->currentIndex()).toInt());
    c.setLabel(ui->leKennung->text());
    c.setPlannedInvest (ui->leBetrag->text().remove('.').toDouble());
    c.setReinvesting (ui->chkbThesaurierend->checkState() == Qt::Checked);

    // interface to comboBox -> 1/100th of the itemdata
    int interrestIndex= ui->cbZins->itemData(ui->cbZins->currentIndex()).toInt();
    c.setInterestRate(round2(double(interrestIndex) /100.));
    c.setConclusionDate (ui->deVertragsabschluss->date());

    int kFrist = ui->cbKFrist->currentData().toInt();
    QDate LaufzeitEnde = ui->deLaufzeitEnde->date();  // QDateTimeEdit default: 2000/1/1
    c.setPlannedEndDate(LaufzeitEnde);
    c.setNoticePeriod(kFrist);
    return c;
}
bool MainWindow::save_new_contract()
{   LOG_CALL;
    contract c =get_contract_data_from_form();

    QString errortext;
    if( !c.validateAndSaveNewContract(errortext)) {
        QMessageBox::critical( this, qsl("Fehler"), errortext);
        return false;
    }
    else {
        if( !errortext.isEmpty())
            QMessageBox::information(this, qsl("Warnung"), errortext);
        return true;
    }
}
void MainWindow::empty_new_contract_form()
{   LOG_CALL;
    ui->leKennung->setText(QString());
    ui->leBetrag->setText(QString());
    ui->chkbThesaurierend->setChecked(true);
}
void MainWindow::on_deLaufzeitEnde_userDateChanged(const QDate &date)
{   LOG_CALL;
    if( date == EndOfTheFuckingWorld) {
        if( ui->cbKFrist->currentIndex() == 0)
            ui->cbKFrist->setCurrentIndex(6);
    }
    else
        ui->cbKFrist->setCurrentIndex(0);
}
void MainWindow::on_cbKFrist_currentIndexChanged(int index)
{   LOG_CALL;
    if( -1 == ui->cbKFrist->itemData(index).toInt()) {
        // Vertragsende wird fest vorgegeben
        if( EndOfTheFuckingWorld == ui->deLaufzeitEnde->date())
            ui->deLaufzeitEnde->setDate(QDate::currentDate().addYears(5));
    }
    else {
        // Vertragsende wird durch Kündigung eingeleitet
        ui->deLaufzeitEnde->setDate(EndOfTheFuckingWorld);
    }
}
void MainWindow::on_leBetrag_editingFinished()
{   LOG_CALL;
    double userInput = ui->leBetrag->text().toDouble();
    ui->leBetrag->setText(QString("%L1").arg(round2(userInput)));
}

// helper: switch to "new contract"
void MainWindow::fill_creditors_dropdown()
{   LOG_CALL;
    ui->comboKreditoren->clear();
    QList<QPair<int, QString>> Personen;
    KreditorenListeMitId(Personen);
    for(auto& Entry :qAsConst(Personen)) {
        ui->comboKreditoren->addItem( Entry.second, QVariant((Entry.first)));
    }
}
void MainWindow::fill_rates_dropdown()
{   LOG_CALL;
    ui->cbZins->clear();
    for(int i = 0; i < 200; i++) {
        ui->cbZins->addItem(QString::number(double(i)/100, 'f', 2), QVariant(i));
    }
    ui->cbZins->setCurrentIndex(100);
}
void MainWindow::set_creditors_combo_by_id(int KreditorenId)
{   LOG_CALL;
    if( KreditorenId < 0) return;
    // select the correct person
    for( int i = 0; i < ui->comboKreditoren->count(); i++) {
        if( KreditorenId == ui->comboKreditoren->itemData(i)) {
            ui->comboKreditoren->setCurrentIndex(i);
            break;
        }
    }
}
// leave new contract
void MainWindow::on_cancelCreateContract_clicked()
{   LOG_CALL;
    empty_new_contract_form();
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
void MainWindow::on_action_save_contract_go_contracts_triggered()
{   LOG_CALL;
    if( save_new_contract()) {
        empty_new_contract_form();
        prepare_contracts_list_view();
        ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
    }
}
void MainWindow::on_action_save_contract_go_kreditors_triggered()
{   LOG_CALL;
    if( save_new_contract()) {
        empty_new_contract_form();
        on_action_menu_creditors_listview_triggered();
    }
}
void MainWindow::on_action_save_contract_new_contract_triggered()
{   LOG_CALL;
    if( save_new_contract()) {
        empty_new_contract_form();
        on_action_menu_contracts_create_triggered();
    }
}
// statistics
void MainWindow::on_action_menu_contracts_statistics_view_triggered()
{   LOG_CALL;
    QComboBox* combo =ui->comboUebersicht;
    if(combo->count() == 0) {
        combo->clear();
        combo->addItem(qsl("Übersicht aller Kredite"),                QVariant(OVERVIEW));
        combo->addItem(qsl("Anzahl auslaufender Verträge nach Jahr"), QVariant(BY_CONTRACT_END));
        combo->addItem(qsl("Anzahl Verträge nach Zinssatz und Jahr"), QVariant(INTEREST_DISTRIBUTION));
        combo->addItem(qsl("Anzahl Verträge nach Laufzeiten"),        QVariant(CONTRACT_TERMS));
        // combo->addItem("Gesamtübersicht aller aktiven Verträge", QVariant(ALL_CONTRACT_INFO));
        combo->setCurrentIndex(0);
    }
    combo->setCurrentIndex(combo->currentIndex());
    ui->stackedWidget->setCurrentIndex(overviewsPageIndex);
}
void MainWindow::on_comboUebersicht_currentIndexChanged(int i)
{   LOG_CALL;
    ui->txtOverview->setText( reportHtml(static_cast<Uebersichten>( i)));
}
void MainWindow::on_pbPrint_clicked()
{   LOG_CALL;
    QString filename = appConfig::Outdir();
    filename += qsl("\\") + QDate::currentDate().toString("yyyy-MM-dd_");
    filename += Uebersichten_kurz[ui->comboUebersicht->currentIndex()];
    filename += qsl(".pdf");
    QPdfWriter write(filename);
    ui->txtOverview->print(&write);
    showFileInFolder(filename);
}
// anual settlement
void MainWindow::on_action_menu_contracts_anual_interest_settlement_triggered()
{   LOG_CALL;
    annualSettlement();
    on_action_menu_contracts_listview_triggered( );
}
// list creation csv, printouts
void MainWindow::on_action_menu_contracts_print_lists_triggered()
{   LOG_CALL;
    if( !createCsvActiveContracts())
        QMessageBox::critical(this, qsl("Fehler"), qsl("Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?"));
}
// debug funktions
void MainWindow::on_action_menu_debug_create_sample_data_triggered()
{   LOG_CALL;
    busycursor b;
    create_sampleData();
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex());
    on_action_menu_contracts_listview_triggered();
}
void MainWindow::on_action_menu_debug_show_log_triggered()
{   LOG_CALL;
    #if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
    #else
    QString cmd = "open " + logFilePath().toUtf8();
    system(cmd.toUtf8().constData());
    #endif
}
void MainWindow::on_actionDatenbank_Views_schreiben_triggered()
{
    insert_views(QSqlDatabase::database());
}
// about
void MainWindow::on_action_about_DKV2_triggered()
{   LOG_CALL;
    QString msg;
    msg = qsl("Lieber Anwender. \nDKV2 wird von seinen Entwicklern kostenlos zur Verfügung gestellt.\n");
    msg += qsl("Es wurde mit viel Arbeit und Sorgfalt entwickelt. Wenn Du es nützlich findest: Viel Spaß bei der Anwendung!!\n");
    msg += qsl("Allerdings darfst Du es nicht verkaufen oder bezahlte Dienste für Einrichtung oder Unterstützung anbieten.\n");
    msg += qsl("DKV2 könnte Fehler enthalten. Wenn Du sie uns mitteilst werden sie vielleicht ausgebessert.\n");
    msg += qsl("Aber aus der Verwendung kannst Du keine Rechte ableiten. Verwende DKV2 so, wie es ist - ");
    msg += qsl("sollten Fehler auftreten übernehmen wir weder Haftung noch Verantwortung - dafür hast Du sicher Verständnis.\n");
    msg += qsl("Viel Spaß mit DKV2 !");
    QMessageBox::information(this, qsl("I n f o"), msg);
}

