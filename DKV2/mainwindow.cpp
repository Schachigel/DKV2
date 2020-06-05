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

#include "askdatedlg.h"
#include "appconfig.h"
#include "itemformatter.h"
#include "dkdbhelper.h"
#include "letters.h"
#include "jahresabschluss.h"
#include "frmjahresabschluss.h"
#include "transaktionen.h"

// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   LOG_CALL;
    ui->setupUi(this);
#ifndef QT_DEBUG
    ui->action_create_sample_data->setVisible(false);
#endif

    ui->leBetrag->setValidator(new QIntValidator(0,999999,this));
    ui->statusBar->addPermanentWidget(ui->statusLabel);

    setCentralWidget(ui->stackedWidget);
    if( !useDb(appConfig::CurrentDb()))
        // there should be a valid DB - checked in main.cpp
        Q_ASSERT(!"useDb failed in construcor of mainwindow");

    ui->txtAnmerkung->setTabChangesFocus(true);
    ui->CreditorsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");
    ui->contractsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");

    fillCombo_NoticePeriods();
    createButtonMenu_saveKreditorAnd();
    createButtonMenu_saveContractAnd();

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
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

void MainWindow::createButtonMenu_saveKreditorAnd()
{   LOG_CALL;
    // Kreditor anlegen: "Speichern und ..." Menü anlegen
    menuSaveKreditorAnd = new QMenu;
    menuSaveKreditorAnd->addAction(ui->action_save_contact_go_contract);
    menuSaveKreditorAnd->addAction(ui->action_save_contact_go_creditors);
    menuSaveKreditorAnd->addAction(ui->action_save_contact_go_new_creditor);
    ui->saveAnd->setMenu(menuSaveKreditorAnd);
    ui->saveAnd->setDefaultAction(ui->action_save_contact_go_contract);
}
void MainWindow::createButtonMenu_saveContractAnd()
{   LOG_CALL;
    // Vertrag anlegen: "Speichern und ... " Menü anlegen
    menuSaveContractAnd = new QMenu;
    menuSaveContractAnd->addAction(ui->action_save_contract_new_contract);
    menuSaveContractAnd->addAction(ui->action_save_contract_go_kreditors);
    menuSaveContractAnd->addAction(ui->action_save_contract_go_contracts);
    ui->saveContractAnd->setMenu(menuSaveContractAnd);
    ui->saveContractAnd->setDefaultAction(ui->action_save_contract_go_kreditors);
}

MainWindow::~MainWindow()
{   LOG_CALL;
    delete ui;
}

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

void MainWindow::prepareWelcomeMsg()
{   LOG_CALL;
    busycursor b;
    QString messageHtml = "<table width='100%'><tr><td><h2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</h2></td></tr>";
// todo: add DK status information
    messageHtml += "<tr><td><img src=\":/res/splash.png\"/></td></tr></table>";
    qDebug() <<"welcome Screen html: " << endl << messageHtml << endl;
    ui->teWelcome->setText(messageHtml);
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
    case emptyPageIndex:
        prepareWelcomeMsg();
        ui->action_delete_creditor->setEnabled(false);
        ui->action_loeschePassivenVertrag->setEnabled(false);
        break;
    case PersonListIndex:
        ui->action_delete_creditor->setEnabled(true);
        ui->action_loeschePassivenVertrag->setEnabled(false);
        break;
    case newPersonIndex:
        ui->action_delete_creditor->setEnabled(false);
        ui->action_loeschePassivenVertrag->setEnabled(false);
        break;
    case newContractIndex:
        ui->action_delete_creditor->setEnabled(false);
        ui->action_loeschePassivenVertrag->setEnabled(false);
        break;
    case ContractsListIndex:
        ui->action_delete_creditor->setEnabled(false);
        ui->action_loeschePassivenVertrag->setEnabled(true);
        break;
    case bookingsListIndex:
        ui->action_delete_creditor->setEnabled(false);
        ui->action_loeschePassivenVertrag->setEnabled(false);
        break;
    default:
        qWarning() << "stackedWidget current change not implemented for this index";
    }// e.o. switch
    return;
}

// file menu
QString askUserDbFilename(QString title, bool onlyExistingFiles=false)
{   LOG_CALL;
    QString folder;
    QFileInfo lastdb (appConfig::CurrentDb());
    if( lastdb.exists())
        folder = lastdb.path();
    else
        folder = QStandardPaths::writableLocation((QStandardPaths::AppDataLocation));

    if( onlyExistingFiles)
        return QFileDialog::getOpenFileName(nullptr, title, folder, "dk-DB Dateien (*.dkdb)", nullptr);
    else
        return QFileDialog::getSaveFileName(nullptr, title, folder, "dk-DB Dateien (*.dkdb)", nullptr);
}
void MainWindow::on_action_back_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_create_new_DB_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename("Neue DkVerarbeitungs Datenbank");
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
        QMessageBox::information(this, "Fehler", "Die neue Datenbankdatei konnte nicht angelegt und geöffnet werden");
        return;
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_open_DB_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename("DkVerarbeitungs Datenbank", true);
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

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_create_anonymous_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename("Anonymisierte Datenbank");
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
void MainWindow::on_action_create_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserDbFilename( "Kopie der Datenbank");
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
void MainWindow::on_action_store_output_directory_triggered()
{   LOG_CALL;
    appConfig::setOutDirInteractive(this);
}
void MainWindow::on_action_exit_program_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

// person list page
void MainWindow::prepareCreditorsTableView()
{   LOG_CALL;
    busycursor b;
    QSqlTableModel* model = new QSqlTableModel(ui->CreditorsTableView);
    model->setTable("Kreditoren");
    model->setFilter("Vorname LIKE '%" + ui->leFilter->text() + "%' OR Nachname LIKE '%" + ui->leFilter->text() + "%'");
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
void MainWindow::on_action_Liste_triggered()
{   LOG_CALL;
    busycursor b;
    prepareCreditorsTableView();
    if( ! ui->CreditorsTableView->currentIndex().isValid())
        ui->CreditorsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}

// helper fu
int MainWindow::currentIdFromCreditorsList()
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

// Kontext Menue in Kreditoren Tabelle
void MainWindow::on_CreditorsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;
    QModelIndex index = ui->CreditorsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid()) {
        QVariant data(ui->CreditorsTableView->model()->data(index));
        if( data.canConvert(QVariant::Int)) {
            QMenu menu( "PersonContextMenu", this);
            menu.addAction(ui->action_edit_Creditor);
            menu.addAction(ui->action_create_contract_for_creditor);
            menu.addAction( ui->action_delete_creditor);
            menu.addAction(ui->action_show_contracts);
            menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
        }
        else
            qCritical() << "Conversion error: model data is not int";
        return;
    }
}
void MainWindow::on_action_edit_Creditor_triggered()
{   LOG_CALL;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QVariant index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0));
    ui->lblPersId->setText(index.toString());

    init_creditor_form(index.toInt());
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}
void MainWindow::on_action_create_contract_for_creditor_triggered()
{   LOG_CALL;
    busycursor b;
    fill_creditors_dropdown();
    fill_rates_dropdown();
    ui->leKennung->setText( proposeKennung());
    if( ui->stackedWidget->currentIndex() == PersonListIndex)
        set_creditors_combo_by_id(currentIdFromCreditorsList());
    if( ui->stackedWidget->currentIndex() == newPersonIndex)
        set_creditors_combo_by_id(passNewCreditorIdToNewContract);
    else
        set_creditors_combo_by_id(-1);
    contract cd; // this is to get the defaults of the class definition
    ui->deLaufzeitEnde->setDate(cd.plannedEndDate());
    ui->cbKFrist->setCurrentIndex(ui->cbKFrist->findText("6"));
    ui->deVertragsabschluss->setDate(cd.conclusionDate());
    ui->chkbThesaurierend->setChecked(cd.reinvesting());

    ui->stackedWidget->setCurrentIndex(newContractIndex);
}
void MainWindow::on_action_delete_creditor_triggered()
{   LOG_CALL;
    const QTableView * const tv = ui->CreditorsTableView;
    QModelIndex mi(tv->currentIndex());
    qlonglong index = tv->model()->data(mi.siblingAtColumn(0)).toLongLong();
    creditor c (index);
    if( c.hasActiveContracts()) {
        QMessageBox::information(this, "Löschen unmöglich", "Ein Kreditor mit aktiven Verträgen kann nicht gelöscht werden");
        return;
    }
    // todo: type save access functions as in contract
    QString Vorname = c.getValue("Vorname").toString(); // ui->CreditorsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = c.getValue("Nachname").toString(); // ui->CreditorsTableView->model()->data(mi.siblingAtColumn(2)).toString();

    QString msg( "Soll der Kreditgeber ");
    msg += Vorname + " " + Nachname + " (id " + QString::number(index) + ")  gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditgeber löschen?", msg))
        return;
    busycursor b;

    if( c.Delete())
        prepareCreditorsTableView();
    else
        Q_ASSERT(!bool("could not remove kreditor and contracts"));
}
void MainWindow::on_action_show_contracts_triggered()
{   LOG_CALL;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QString index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->leVertraegeFilter->setText("index:" +index);
    on_action_show_list_of_contracts_triggered();
}
void MainWindow::on_leFilter_editingFinished()
{   LOG_CALL;
    busycursor b;
    prepareCreditorsTableView();
}
void MainWindow::on_pbPersonFilterZuruecksetzen_clicked()
{   LOG_CALL;
    busycursor b;
    ui->leFilter->setText("");
    prepareCreditorsTableView();
}

// new DK Geber
void MainWindow::on_action_create_new_creditor_triggered()
{   LOG_CALL;
    empty_create_creditor_form();
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
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
void MainWindow::on_cancel_clicked()
{   LOG_CALL;
    empty_create_creditor_form();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_save_contact_go_contract_triggered()
{   LOG_CALL;
    int kid = save_creditor();
    if(  kid != -1) {
        empty_create_creditor_form();
        on_action_create_contract_for_creditor_triggered();
    }
}
void MainWindow::on_action_save_contact_go_creditors_triggered()
{   LOG_CALL;
    if( save_creditor() != -1) {
        empty_create_creditor_form();
        on_action_Liste_triggered();
    }
}
void MainWindow::on_action_save_contact_go_new_creditor_triggered()
{   LOG_CALL;
    if( save_creditor() != -1)
        empty_create_creditor_form();
}

// neuer Vertrag
void ensureConsistentContractEnding( int& monate, QDate& lze, const QDate )
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
    c.setInterest100th(round2(double(interrestIndex)/100));
    c.setConclusionDate (ui->deVertragsabschluss->date());

    int kFrist = ui->cbKFrist->currentData().toInt();
    QDate LaufzeitEnde = ui->deLaufzeitEnde->date();  // QDateTimeEdit default: 2000/1/1
    ensureConsistentContractEnding(kFrist, LaufzeitEnde, c.conclusionDate());
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
    ui->leBetrag->setText(QString("%L1").arg(ui->leBetrag->text().toDouble()));
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
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_save_contract_go_contracts_triggered()
{   LOG_CALL;
    if( save_new_contract()) {
        empty_new_contract_form();
        prepare_contracts_list_view();
        ui->stackedWidget->setCurrentIndex(ContractsListIndex);
    }
}
void MainWindow::on_action_save_contract_go_kreditors_triggered()
{   LOG_CALL;
    if( save_new_contract()) {
        empty_new_contract_form();
        on_action_Liste_triggered();
    }
}
void MainWindow::on_action_save_contract_new_contract_triggered()
{   LOG_CALL;
    if( save_new_contract()) {
        empty_new_contract_form();
        on_action_create_contract_for_creditor_triggered();
    }
}

// Liste der Verträge
QString filterFromFilterphrase(QString fph)
{
    if( fph.startsWith("index:"))
    {
        bool conversionOK = true;
        qlonglong contractId = fph.right(fph.length()-6).toInt(&conversionOK);
        if( ! conversionOK)
            return "";
        else
            return "id=" + QString::number(contractId);
    }
    return fph.isEmpty() ? "" :
           ("Kreditorin LIKE '%" + fph + "%' OR Vertragskennung LIKE '%" + fph + "%'");
}
void MainWindow::prepare_contracts_list_view()
{   LOG_CALL;
    busycursor b;
    QSqlTableModel* model = new QSqlTableModel(this);
    model->setTable("WertAlleVertraege");
    model->setFilter( filterFromFilterphrase(ui->leVertraegeFilter->text()));
    qDebug() << "contract list model filter: " << model->filter();

    ui->contractsTableView->setModel(model);
    model->select();

    ui->contractsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->contractsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->contractsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contractsTableView->setAlternatingRowColors(true);
    ui->contractsTableView->setSortingEnabled(true);
    ui->contractsTableView->hideColumn(0);
    ui->contractsTableView->setItemDelegateForColumn(3, new PercentItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(4, new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(5, new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(6, new KFristItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(7, new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(8, new thesaItemFormatter(ui->contractsTableView));

    ui->contractsTableView->resizeColumnsToContents();
}
void MainWindow::on_action_show_list_of_contracts_triggered()
{   LOG_CALL;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(ContractsListIndex);
}
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{   LOG_CALL;
//    QSqlRecord rec = tmp_ContractsModel->record(); // ugly, but qobject_cast does not work
    int indedOf_active_inModel = 1;//  = rec.indexOf("aktiv");
    int indexOf_kfrist_inModel = 1;// = rec.indexOf("Kfrist");

    QModelIndex indexClickTarget = ui->contractsTableView->indexAt(pos);
    QModelIndex index_IsActive = indexClickTarget.siblingAtColumn(indedOf_active_inModel); // contract active
    QVariant ContractIsActive = ui->contractsTableView->model()->data(index_IsActive);
    if( !ContractIsActive.isValid())
        return; // clicked outside the used lines
    QModelIndex index_Kfrist = indexClickTarget.siblingAtColumn(indexOf_kfrist_inModel);
    QVariant Kfrist = ui->contractsTableView->model()->data(index_Kfrist);
    bool hatLaufzeitende = false;
    if( Kfrist == -1)
        hatLaufzeitende = true;

    QMenu menu( "PersonContextMenu", this);
    if(ContractIsActive.toBool())
    {
        if( hatLaufzeitende)
            ui->action_terminate_contract->setText("Vertrag beenden");
        else
            ui->action_terminate_contract->setText("Vertrag kündigen");
        menu.addAction(ui->action_terminate_contract);
    }
    else
    {
        menu.addAction(ui->action_activate_contract);
        menu.addAction(ui->action_loeschePassivenVertrag); // passive Verträge können gelöscht werden
    }
    menu.exec(ui->CreditorsTableView->mapToGlobal(pos));
    return;
}
int  MainWindow::get_current_id_from_contracts_list()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}

// contract list context menu
void MainWindow::on_action_activate_contract_triggered()
{   LOG_CALL;

    aktiviereVertrag(get_current_id_from_contracts_list());
    prepare_contracts_list_view();
}
void MainWindow::on_action_loeschePassivenVertrag_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;

    QString index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    beendeVertrag(index.toInt());

    prepare_contracts_list_view();
}
void MainWindow::on_leVertraegeFilter_editingFinished()
{   LOG_CALL;
    prepare_contracts_list_view();
}
void MainWindow::on_reset_contracts_filter_clicked()
{   LOG_CALL;
    ui->leVertraegeFilter->setText("");
    prepare_contracts_list_view();
}
void MainWindow::on_action_terminate_contract_triggered()
{   LOG_CALL;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    int index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toInt();
    beendeVertrag(index);

    prepare_contracts_list_view();
}


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
void MainWindow::on_action_contracts_statistics_triggered()
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
    ui->stackedWidget->setCurrentIndex(OverviewIndex);
}
void MainWindow::on_comboUebersicht_currentIndexChanged(int )
{   LOG_CALL;
    on_action_contracts_statistics_triggered();
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
void MainWindow::on_action_anual_interest_settlement_triggered()
{   LOG_CALL;
    jahresabschluss Abschluss;

    QString msg = "Der Jahresabschluss für das Jahr "
                  + QString::number(Abschluss.abzuschliessendesJahr())
                  + " kann gemacht werden\n\n";
    msg += "Dabei werden die Zinsen für alle Verträge berechnet. Der Wert von thesaurierenden Verträgen wird angepasst\n";
    msg += "Dieser Vorgang kann nicht rückgängig gemacht werden. Möchtest Du fortfahren?";

    if( QMessageBox::Yes != QMessageBox::question(this, "Jahresabschluss", msg))
        return;
    Abschluss.execute();
    frmJahresabschluss dlgJA(Abschluss, this);
    dlgJA.exec();
    on_action_show_list_of_contracts_triggered( );
}
void MainWindow::on_action_create_active_contracts_csv_triggered()
{   LOG_CALL;
    if( !createCsvActiveContracts())
        QMessageBox::critical(this, "Fehler", "Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?");
}
// debug funktions
void MainWindow::on_action_create_sample_data_triggered()
{   LOG_CALL;
    busycursor b;
    create_sampleData();
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex());
}
void MainWindow::on_action_log_anzeigen_triggered()
{   LOG_CALL;
    #if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
    #else
    QString cmd = "open " + logFilePath().toUtf8();
    system(cmd.toUtf8().constData());
    #endif
}
// bookings- bisher Debug stuff
void MainWindow::on_tblViewBookingsSelectionChanged(const QItemSelection& to, const QItemSelection& )
{   LOG_CALL;
    QString json =ui->tblViewBookings->model()->data(to.indexes().at(0).siblingAtColumn(6)).toString();
    ui->lblYson->setText(json);
}
void MainWindow::on_actionShow_Bookings_triggered()
{   LOG_CALL;

    QSqlRelationalTableModel* model = new QSqlRelationalTableModel(ui->tblViewBookings);
    model->setTable("Buchungen");
    model->setSort(0,Qt::DescendingOrder );
    model->setRelation(2, QSqlRelation("Buchungsarten", "id", "Art"));

    model->select();

    ui->tblViewBookings->setModel(model);
    ui->tblViewBookings->hideColumn(6);
    ui->tblViewBookings->resizeColumnsToContents();

    connect(ui->tblViewBookings->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(on_tblViewBookingsSelectionChanged(const QItemSelection&, const QItemSelection&)));

    ui->stackedWidget->setCurrentIndex(bookingsListIndex);
}
// about
void MainWindow::on_action_ber_DKV2_triggered()
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
