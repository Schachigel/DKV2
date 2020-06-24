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
#include "wizfileselection.h"
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
    ui->CreditorsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");
    ui->contractsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");

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

void MainWindow::currentChange_ctv(const QModelIndex & newI, const QModelIndex & )
{
    // todo: do all init only once, this function should only do the
    // setFilter and the select()
    QModelIndex indexIndex = newI.siblingAtColumn(0);
    int index =ui->contractsTableView->model()->data(indexIndex).toInt();
    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable("Buchungen");
    model->setFilter("Buchungen.VertragsId=" + QString::number(index));
    model->setSort(4, Qt::SortOrder::DescendingOrder);

    ui->bookingsTableView->setModel(model);
    model->select();
    ui->bookingsTableView->hideColumn(0);
    ui->bookingsTableView->hideColumn(1);
    ui->bookingsTableView->setItemDelegateForColumn(3, new EuroItemFormatter);
}
// generell functions
void MainWindow::setSplash(QSplashScreen* s)
{   LOG_CALL;
    splash = s;
    startTimer(2333);
}
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
    QString messageHtml = "<table width='100%'><tr><td><h2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</h2></td></tr>";
// todo: add DK status information
    messageHtml += "<tr><td><img src=\":/res/splash.png\"/></td></tr></table>";
    qDebug() <<"welcome Screen html: " << endl << messageHtml << endl;
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
    fileSelectionWiz wiz;
    wiz.title =title;
    wiz.subtitle ="Mit dieser Dialogfolge wählst Du eine DKV2 Datenbank aus";
    wiz.fileTypeDescription ="dk-DB Dateien (*.dkdb)";
    if( (wiz.existingFile=onlyExistingFiles))
        wiz.bffTitle ="Wähle eine existierende dkdb Datei aus";
    else
        wiz.bffTitle ="Wähle eine dkdb Datei aus oder gib einen neuen Dateinamen ein";

    QFileInfo lastdb (appConfig::CurrentDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::AppDataLocation));

    wiz.exec();
    return wiz.field("selectedFile").toString();
}
QString askUserNewDb()
{   LOG_CALL;
    newDatabaseWiz wiz;
    QFont f = wiz.font(); f.setPointSize(10); wiz.setFont(f);
    QFileInfo lastdb (appConfig::CurrentDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::AppDataLocation));
    wiz.title = "Neue DKV2 Datenbank Datei";
    wiz.exec();
    appConfig::setRuntimeData("gmbh.address1", wiz.field("address1").toString());
    appConfig::setRuntimeData("gmbh.address2", wiz.field("address2").toString());
    appConfig::setRuntimeData("gmbh.strasse",  wiz.field("strasse").toString());
    appConfig::setRuntimeData("gmbh.plz",      wiz.field("plz").toString());
    appConfig::setRuntimeData("gmbh.stadt",    wiz.field("stadt").toString());
    appConfig::setRuntimeData("gmbh.email",    wiz.field("email").toString());
    appConfig::setRuntimeData("gmbh.url",      wiz.field("url").toString());
    appConfig::setRuntimeData("ProjektInitialen", wiz.field("projekt").toString());
    appConfig::setRuntimeData("IdOffset",      wiz.field("IdOffset").toString());

    return wiz.field("selectedFile").toString();
}
void MainWindow::on_action_menu_database_new_triggered()
{   LOG_CALL;
    QString dbfile = askUserNewDb();
    if( dbfile == "") {
        qDebug() << "user canceled file selection";
        return;
    }
    busycursor b;
    closeDatabaseConnection();
    if( create_DK_databaseFile(dbfile) && useDb(dbfile)) {
        appConfig::setLastDb(dbfile);
    }
    else {
        QMessageBox::information(this, "Fehler", "Die neue Datenbankdatei konnte nicht angelegt und geöffnet werden.");
        return;
    }
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
void MainWindow::on_action_menu_database_open_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename("DKV2 Datenbank zum Öffnen auswählen.", true);
    if( dbfile == "") {
        qDebug() << "keine Datei wurde vom Anwender ausgewählt";
        QMessageBox::information(this, "Abbruch", "Es wurde keine Datenbankdatei ausgewählt");
        return;
    }
    busycursor b;
    if( useDb(dbfile))
        appConfig::setLastDb(dbfile);
    else {
        QMessageBox::information(this, "Fehler", "Die Datenbank konnte nicht geöffnet werden");
        if( !useDb(appConfig::CurrentDb())) {
            qFatal("alte und neue DB können nicht geöffnet werden -> abbruch");
            exit( 1);
        }
    }

    ui->stackedWidget->setCurrentIndex(startPageIndex);
}
void MainWindow::on_action_menu_database_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename( "Dateiname der Kopie Datenbank angeben.");
    if( dbfile == "")
        return;

    busycursor b;
    if( !create_DB_copy(dbfile, false)) {
        QMessageBox::information(this, "Fehler beim Kopieren", "Die Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei");
        qCritical() << "creating copy failed";
    }
    return;
}
void MainWindow::on_action_menu_database_anonymous_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename("Dateiname der Anonymisierten Kopie angeben.");
    if( dbfile == "")
        return;
    busycursor b;
    if( !create_DB_copy(dbfile, true)) {
        QMessageBox::information(this, "Fehler beim Kopieren", "Die anonymisierte Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei");
        qCritical() << "creating depersonaliced copy failed";
    }
    return;
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
    model->setTable("Kreditoren");
    model->setFilter("Vorname LIKE '%" + ui->le_CreditorsFilter->text() + "%' OR Nachname LIKE '%" + ui->le_CreditorsFilter->text() + "%'");
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
    ui->le_CreditorsFilter->setText("");
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
        QMenu menu( "PersonContextMenu", this);
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

    QString msg( "Soll der Kreditgeber %1 %2 (id %3) gelöscht werden?");
    msg =msg.arg(c.getValue("Vorname").toString())
            .arg(c.getValue("Nachname").toString())
            .arg(QString::number(index));

    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditgeber löschen?", msg))
        return;
    busycursor b;

    if( c.remove())
        prepare_CreditorsListPage();
    else
        QMessageBox::information(this, "Löschen unmöglich", "Ein Kreditor mit aktiven oder beendeten Verträgen kann nicht gelöscht werden");
}
void MainWindow::on_action_cmenu_go_contracts_triggered()
{   LOG_CALL;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QString index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->le_ContractsFilter->setText("kreditor:" +index);
    on_action_menu_contracts_listview_triggered();
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
        errortext = "Die Daten können nicht gespeichert werden: <br>" + errortext;
        QMessageBox::information(this, "Fehler", errortext );
        qDebug() << "prüfung der Kreditor Daten:" << errortext;
        return -1;
    }
    int kid = -1;
    if( ui->lblPersId->text() != "") {
        kid = ui->lblPersId->text().toInt();
        c.setId(kid);     // update not insert
        c.update();
    }
    else
       kid = c.save();

    if(kid == -1) {
        QMessageBox::information( this, "Fehler", "Der Datensatz konnte nicht gespeichert werden. "
                     "Ist die E-Mail Adresse einmalig? Gibt es die Adressdaten in der Datenbank bereits?"
                     "\nBitte überprüfen Sie ihre Eingaben");
        qCritical() << "Kreditgeber konnte nicht gespeichert werden";
        return -1;
    }

    return passNewCreditorIdToNewContract = kid;
}
void MainWindow::empty_create_creditor_form()
{   LOG_CALL;
    ui->leVorname->setText("");
    ui->leNachname->setText("");
    ui->leStrasse->setText("");
    ui->lePlz->setText("");
    ui->leStadt->setText("");
    ui->leEMail->setText("");
    ui->txtAnmerkung->setPlainText("");
    ui->leIban->setText("");
    ui->leBic->setText("");
    ui->lblPersId->setText("");
}
void MainWindow::init_creditor_form(int id)
{   LOG_CALL;
    busycursor b;
    QSqlRecord rec = executeSingleRecordSql(dkdbstructur["Kreditoren"].Fields(), "Id=" +QString::number(id));
    ui->leVorname->setText(rec.field("Vorname").value().toString());
    ui->leNachname->setText(rec.field("Nachname").value().toString());
    ui->leStrasse->setText(rec.field("Strasse").value().toString());
    ui->lePlz->setText(rec.field("Plz").value().toString());
    ui->leStadt->setText(rec.field("Stadt").value().toString());
    ui->leEMail->setText(rec.field("Email").value().toString());
    ui->txtAnmerkung->setPlainText(rec.field("Anmerkung").value().toString());
    ui->leIban  ->setText(rec.field("IBAN").value().toString());
    ui->leBic  ->setText(rec.field("BIC").value().toString());
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

// Contract List helper
QString filterFromFilterphrase(QString fph)
{
    if( fph.startsWith("kreditor:"))
    {
        bool conversionOK = true;
        qlonglong contractId = fph.right(fph.length()-9).toInt(&conversionOK);
        if( ! conversionOK)
            return "";
        else
            return "KreditorId=" + QString::number(contractId);
    }
    return fph.isEmpty() ? "" :
           ("Kreditorin LIKE '%" + fph + "%' OR Vertragskennung LIKE '%" + fph + "%'");
}
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
    ui->cbKFrist->addItem("festes Vertragsende", QVariant(-1));
    for (int i=3; i<12; i++)
        ui->cbKFrist->addItem(QString::number(i) + " Monate", QVariant(i));
    ui->cbKFrist->addItem("1 Jahr", QVariant(12));
    ui->cbKFrist->addItem("1 Jahr und 1 Monat", QVariant(13));
    for (int i=14; i<24; i++)
        ui->cbKFrist->addItem("1 Jahr und " + QString::number( i-12) + " Monate", QVariant(i));
    ui->cbKFrist->addItem("2 Jahre", QVariant(24));
}
// Contract List
void MainWindow::on_action_menu_contracts_listview_triggered()
{   LOG_CALL;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(contractsListPageIndex);
}
void MainWindow::prepare_contracts_list_view()
{   LOG_CALL;
    busycursor b;
    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable("WertAlleVertraege");
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
    tv->setItemDelegateForColumn(4, new EuroItemFormatter(tv));
    tv->setItemDelegateForColumn(5, new DateItemFormatter(tv));
    tv->setItemDelegateForColumn(6, new KFristItemFormatter(tv));
    tv->setItemDelegateForColumn(7, new DateItemFormatter(tv));
    tv->setItemDelegateForColumn(8, new thesaItemFormatter(tv));

    tv->resizeColumnsToContents();
    auto c = connect(ui->contractsTableView->selectionModel(),
            SIGNAL(currentChanged (const QModelIndex & , const QModelIndex & )),
            SLOT(currentChange_ctv(const QModelIndex & , const QModelIndex & )));
    qDebug() << c;

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
    ui->le_ContractsFilter->setText("");
    prepare_contracts_list_view();
}

// contract list context menu
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;

    QTableView*& tv = ui->contractsTableView;
    QModelIndex index = tv->indexAt(pos).siblingAtColumn(0);

    contract c(index.data().toInt());
    bool hatLaufzeitende = c.noticePeriod() == -1;

    QMenu menu( "ContractContextMenu", this);
    if(c.isActive())
    {
        if( hatLaufzeitende)
            ui->action_cmenu_terminate_contract->setText("Vertrag beenden");
        else
            ui->action_cmenu_terminate_contract->setText("Vertrag kündigen");
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

// new contract
void MainWindow::on_action_menu_contracts_create_triggered()
{   LOG_CALL;
    busycursor b;
    fill_creditors_dropdown();
    fill_rates_dropdown();
    ui->leKennung->setText( proposeKennung());
    if( ui->stackedWidget->currentIndex() == creditorsListPageIndex)
        set_creditors_combo_by_id(id_SelectedCreditor());
    if( ui->stackedWidget->currentIndex() == newCreditorPageIndex)
        set_creditors_combo_by_id(passNewCreditorIdToNewContract);
    else
        set_creditors_combo_by_id(-1);
    contract cd; // this is to get the defaults of the class definition
    ui->deLaufzeitEnde->setDate(cd.plannedEndDate());
    int cbkFristStartIndex = ui->cbKFrist->findText("6 Monate");
    if( cbkFristStartIndex <0)
        qDebug() << "error setting kFrist Combo default value";
    else
        ui->cbKFrist->setCurrentIndex(cbkFristStartIndex);
    ui->deVertragsabschluss->setDate(cd.conclusionDate());
    ui->chkbThesaurierend->setChecked(cd.reinvesting());

    ui->stackedWidget->setCurrentIndex(newContractPageIndex);
}
void ensureEndOfContractConsistency( int& monate, QDate& lze, const QDate )
{   LOG_CALL;
    // ensure consistency:
    // kfrist == -1 -> LaufzeitEnde has to be valid and NOT 31.12.9999
    // kfrist > 0 -> LaufzeitEnde == 31.12.9999
    if( !lze.isValid() || lze == QDate(2000,1,1)) // QDateEdit default
        lze = EndOfTheFuckingWorld;

    if( monate > 0) {
        if( lze == EndOfTheFuckingWorld)
            return;  // all good
        else {
            lze = EndOfTheFuckingWorld;
            qCritical() << "inconsistent contract end date with valid notice period resolved to " << lze;
            return;
        }
    }

    if( monate < 0) {
        monate = -1;
        if( lze == EndOfTheFuckingWorld) {
            // pathetic we have to choose
            lze = EndOfTheFuckingWorld;
            monate = 6;
            qCritical() << "incnosistent contract end reslved. notice period now " << monate;
            return;
        }
        else
            return;
    }
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
    c.setInterest100th(round2(double(interrestIndex)));
    c.setConclusionDate (ui->deVertragsabschluss->date());

    int kFrist = ui->cbKFrist->currentData().toInt();
    QDate LaufzeitEnde = ui->deLaufzeitEnde->date();  // QDateTimeEdit default: 2000/1/1
    ensureEndOfContractConsistency(kFrist, LaufzeitEnde, c.conclusionDate());
    return c;
}
bool MainWindow::save_new_contract()
{   LOG_CALL;
    contract c =get_contract_data_from_form();

    QString errortext;
    if( !c.validateAndSaveNewContract(errortext)) {
        QMessageBox::critical( this, "Fehler", errortext);
        return false;
    }
    else {
        if( !errortext.isEmpty())
            QMessageBox::information(this, "Warnung", errortext);
        return true;
    }
}
void MainWindow::empty_new_contract_form()
{   LOG_CALL;
    ui->leKennung->setText("");
    ui->leBetrag->setText("");
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

// helper: switch to "Vertrag anlegen"
void MainWindow::fill_creditors_dropdown()
{   LOG_CALL;
    ui->comboKreditoren->clear();
    QList<QPair<int, QString>> Personen;
    creditor k; k.KreditorenListeMitId(Personen);
    for(auto Entry :Personen) {
        ui->comboKreditoren->addItem( Entry.second, QVariant((Entry.first)));
    }
}
void MainWindow::fill_rates_dropdown()
{   LOG_CALL;
    ui->cbZins->clear();
    for(int i = 0; i < 200; i++) {
        ui->cbZins->addItem(QString::number(double(i)/100, 'f', 2), QVariant(i));
    }
    ui->cbZins->setCurrentIndex(101);
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

// statistics pages - Helper Fu
QString tableRow( QString left, QString center, QString center2, QString right)
{
    left    = "<td style='text-align: right;' >" + left    + "</td>";
    center  = "<td style='text-align: center;'>" + center  + "</td>";
    center2 = "<td style='text-align: center;'>" + center2 + "</td>";
    right   = "<td style='text-align: left;'  >" + right   + "</td>";
    return "<tr>" + left + center + center2 + right  + "</tr>";
}
QString tableRow( QString left, QString center, QString right)
{
    left   = "<td style='text-align: right;' >" + left   + "</td>";
    center = "<td style='text-align: center;'>" + center + "</td>";
    right  = "<td style='text-align: left;'  >" + right  + "</td>";
    return "<tr>" + left + center + right  + "</tr>";
}
QString tableRow(QString left, QString right)
{
    left = "<td style='text-align: right;'>" + left  + "</td>";
    right= "<td style='text-align: left;' >" + right + "</td>";
    return "<tr>" + left + right  + "</tr>";
}
QString emptyRow( )
{
    return "<tr><td style='padding: 1px; font-size: small;'></td><td style='padding: 1px; font-size: small';></td></tr>";
}
QString h2(QString v)
{
    return "<h2>" + v + "</h2>";
}
QString h1(QString v)
{
    return "<h1>" + v + "</h1>";
}
QString td( QString v)
{
    return "<td>" + v + "</td>";
}
QString startTable()
{
    return "<table cellpadding='8' bgcolor=#DDD>";
}
QString endTable()
{
    return "</table>";
}
QString row( QString cont)
{
    return "<tr>" + cont + "</tr>";
}
QString startRow()
{
    return "<tr>";
}
QString endRow()
{
    return "</t>";
}
QString newLine(QString line)
{
    return "<br>" + line;
}
// statistics overview
QString MainWindow::prepare_overview_page(Uebersichten u)
{   LOG_CALL;

    QString lbl ("<html><body>"
                 "<style>"
                 "table { border-width: 0px; font-family: Verdana; font-size: large; }"
                 "td { }"
                 "</style>");
    QLocale locale;

    switch( u )
    {
    case UEBERSICHT:
    {
        DbSummary dbs;
        calculateSummary(dbs);
        lbl += h1("Übersicht DKs und DK Geber")+ newLine( "Stand: " + QDate::currentDate().toString("dd.MM.yyyy<br>"));
        lbl += startTable();
        lbl += tableRow("Anzahl aktiver Direktkredite:" , QString::number(dbs.AnzahlAktive));
        lbl += tableRow("Anzahl DK Geber*innen von aktiven Verträgen:", QString::number(dbs.AnzahlDkGeber));
        lbl += tableRow("Summe aktiver Direktkredite:"  , locale.toCurrencyString(dbs.BetragAktive) + "<br><small>(Ø " + locale.toCurrencyString(dbs.BetragAktive/dbs.AnzahlAktive) + ")</small>");
        lbl += tableRow("Wert inklusive Zinsen:", locale.toCurrencyString(dbs.WertAktive) + "<br><small>(Ø " + locale.toCurrencyString(dbs.WertAktive/dbs.AnzahlAktive) + ")</small>");
        lbl += tableRow("Durchschnittlicher Zinssatz:<br><small>(Gewichtet mit Vertragswert)</small>", QString::number(dbs.DurchschnittZins, 'f', 3) + "%");
        lbl += tableRow("Jährliche Zinskosten:", locale.toCurrencyString(dbs.WertAktive * dbs.DurchschnittZins/100.));
        lbl += tableRow("Mittlerer Zinssatz:", QString::number(dbs.MittlererZins, 'f', 3) + "%");
        lbl += emptyRow();
        lbl += tableRow("Anzahl mit jährl. Zinsauszahlung:", QString::number(dbs.AnzahlAuszahlende));
        lbl += tableRow("Summe:", locale.toCurrencyString(dbs.BetragAuszahlende));
        lbl += emptyRow();
        lbl += tableRow("Anzahl ohne jährl. Zinsauszahlung:", QString::number(dbs.AnzahlThesaurierende));
        lbl += tableRow("Summe:", locale.toCurrencyString(dbs.BetragThesaurierende));
        lbl += tableRow("Wert inkl. Zinsen:", locale.toCurrencyString(dbs.WertThesaurierende));
        lbl += emptyRow();
        lbl += tableRow("Anzahl ausstehender (inaktiven) DK", QString::number(dbs.AnzahlPassive));
        lbl += tableRow("Summe ausstehender (inaktiven) DK", locale.toCurrencyString(dbs.BetragPassive));
        lbl += endTable();
        break;
    }
    case VERTRAGSENDE:
    {
        lbl += h1("Auslaufende Verträge") + newLine( "Stand: "  + QDate::currentDate().toString("dd.MM.yyyy<br>"));
        QVector<ContractEnd> ce;
        calc_contractEnd(ce);
        if( !ce.isEmpty()) {
            lbl += startTable();
            lbl += tableRow( h2("Jahr"), h2( "Anzahl"),  h2( "Summe"));
            for( auto x: ce)
                lbl += tableRow( QString::number(x.year), QString::number(x.count), locale.toCurrencyString(x.value));
            lbl += endTable();
        }
        else
            lbl += "<br><br><i>keine Verträge mit vorgemerktem Vertragsende</i>";
        break;
    }
    case ZINSVERTEILUNG:
    {
        QLocale locale;
        QVector<YZV> yzv;
        calc_anualInterestDistribution( yzv);
        if( !yzv.isEmpty()) {
            lbl += h1("Verteilung der Zinssätze pro Jahr") + "<br> Stand:"  + QDate::currentDate().toString("dd.MM.yyyy<br>");
            lbl += startTable() +  startRow();
            lbl += td(h2("Jahr")) + td( h2( "Zinssatz")) +td(h2("Anzahl")) + td( h2( "Summe"));
            lbl += endRow();
            for( auto x: yzv) {
                lbl += tableRow( QString::number(x.year), QString::number(x.intrest), QString::number(x.count), locale.toCurrencyString(x.sum));
            }
            lbl += endTable();
        }
        break;
    }
    case LAUFZEITEN:
    {
        lbl += h1("Vertragslaufzeiten") + "<br> Stand:" + QDate::currentDate().toString("dd.MM.yyyy<br>");
        lbl += startTable();
        QVector<rowData> rows = contractRuntimeDistribution();
        lbl += tableRow( h2(rows[0].text), h2(rows[0].value), h2(rows[0].number));
        for( int i = 1; i < rows.count(); i++)
            lbl += tableRow(rows[i].text, rows[i].value, rows[i].number);
    }
    }
    lbl += "</body></html>";
    qDebug() << "\n" << lbl << endl;
    return lbl;
}
void MainWindow::on_action_menu_contracts_statistics_view_triggered()
{   LOG_CALL;
    if(ui->comboUebersicht->count() == 0)
    {
        ui->comboUebersicht->clear();
        ui->comboUebersicht->addItem("Übersicht aller Kredite", QVariant(UEBERSICHT));
        ui->comboUebersicht->addItem("Anzahl auslaufender Verträge nach Jahr", QVariant(VERTRAGSENDE));
        ui->comboUebersicht->addItem("Anzahl Verträge nach Zinssatz und Jahr", QVariant(ZINSVERTEILUNG));
        ui->comboUebersicht->addItem("Anzahl Verträge nach Laufzeiten", QVariant(LAUFZEITEN));
        ui->comboUebersicht->setCurrentIndex(0);
    }
    int currentIndex = ui->comboUebersicht->currentIndex();
    Uebersichten u = static_cast<Uebersichten>( ui->comboUebersicht->itemData(currentIndex).toInt());
    ui->txtOverview->setText( prepare_overview_page(u));
    ui->stackedWidget->setCurrentIndex(overviewsPageIndex);
}
void MainWindow::on_comboUebersicht_currentIndexChanged(int )
{   LOG_CALL;
    on_action_menu_contracts_statistics_view_triggered();
}
void MainWindow::on_pbPrint_clicked()
{   LOG_CALL;
    QString filename = appConfig::Outdir();

    filename += "\\" + QDate::currentDate().toString("yyyy-MM-dd_");
    filename += Uebersichten_kurz[ui->comboUebersicht->currentIndex()];
    filename += ".pdf";
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
        QMessageBox::critical(this, "Fehler", "Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?");
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
    msg = "Lieber Anwender. \nDKV2 wird von seinen Entwicklern kostenlos zur Verfügung gestellt.\n";
    msg += "Es wurde mit viel Arbeit und Sorgfalt entwickelt. Wenn Du es nützlich findest: Viel Spaß bei der Anwendung!!\n";
    msg += "Allerdings darfst Du es nicht verkaufen oder bezahlte Dienste für Einrichtung oder Unterstützung anbieten.\n";
    msg += "DKV2 könnte Fehler enthalten. Wenn Du sie uns mitteilst werden sie vielleicht ausgebessert.\n";
    msg += "Aber aus der Verwendung kannst Du keine Rechte ableiten. Verwende DKV2 so, wie es ist - ";
    msg += "sollten Fehler auftreten übernehmen wir weder Haftung noch Verantwortung - dafür hast Du sicher Verständnis.\n";
    msg += "Viel Spaß mit DKV2 !";
    QMessageBox::information(this, "I n f o", msg);
}

