#include <QtCore>

#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif
#include <QPair>
#include <QFileDialog>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QMap>

#include <QPdfWriter>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "askdatedlg.h"
#include "helper.h"
#include "filehelper.h"
#include "itemformatter.h"
#include "sqlhelper.h"
#include "dkdbhelper.h"
#include "letters.h"
#include "jahresabschluss.h"
#include "frmjahresabschluss.h"
#include "transaktionen.h"

// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{LOG_ENTRY_and_EXIT;
    ui->setupUi(this);
//#ifdef QT_DEBUG
    ui->menuDebug->setTitle("Debug");
//#endif

    ui->leBetrag->setValidator(new QIntValidator(0,999999,this));
    ui->statusBar->addPermanentWidget(ui->statusLabel);

    setCentralWidget(ui->stackedWidget);
    open_databaseForApplication();
    showDbInStatusbar();
    prepareWelcomeMsg();

    ui->txtAnmerkung->setTabChangesFocus(true);
    ui->CreditorsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");
    ui->contractsTableView->setStyleSheet("QTableView::item { padding-right: 10px; padding-left: 15px; }");

    // combo box für Kündigungsfristen füllen
    ui->cbKFrist->addItem("festes Vertragsende", QVariant(-1));
    for (int i=3; i<25; i++)
        ui->cbKFrist->addItem(QString::number(i), QVariant(i));

    // Kreditor anlegen: "Speichern und ..." Menü anlegen
    menuSaveKreditorAnd = new QMenu;
    menuSaveKreditorAnd->addAction(ui->action_save_contact_go_contract);
    menuSaveKreditorAnd->addAction(ui->action_save_contact_go_creditors);
    menuSaveKreditorAnd->addAction(ui->action_save_contact_go_new_creditor);
    ui->saveAnd->setMenu(menuSaveKreditorAnd);
    ui->saveAnd->setDefaultAction(ui->action_save_contact_go_contract);

    // Vertrag anlegen: "Speichern und ... " Menü anlegen
    menuSaveContractAnd = new QMenu;
    menuSaveContractAnd->addAction(ui->action_save_contract_new_contract);
    menuSaveContractAnd->addAction(ui->action_save_contract_go_kreditors);
    menuSaveContractAnd->addAction(ui->action_save_contract_go_contracts);
    ui->saveContractAnd->setMenu(menuSaveContractAnd);
    ui->saveContractAnd->setDefaultAction(ui->action_save_contract_go_kreditors);

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

MainWindow::~MainWindow()
{LOG_ENTRY_and_EXIT;
    delete ui;
}

void MainWindow::setSplash(QSplashScreen* s)
{LOG_ENTRY_and_EXIT;
    splash = s;
    startTimer(3333);
}

void MainWindow::showDbInStatusbar()
{LOG_ENTRY_and_EXIT;
    QSettings config;
    ui->statusLabel->setText(config.value("db/last").toString());
}

void MainWindow::prepareWelcomeMsg()
{LOG_ENTRY_and_EXIT;
    busycursor b;
    QString message="<H2>Willkommen zu DKV2- Deiner Verwaltung von Direktrediten</H2>";

    QStringList warnings;
    check_DbConsistency( warnings);

    foreach(QString warning, warnings)
    {
        message += "<br><font color='red'>" +warning +"</font>";
    }
    message += "<br><img src=':/res/logo.png'>";
    ui->label->setText(message);
}
// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{LOG_ENTRY_and_EXIT;
    if( arg1 < 0)
    {
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
    {
        qWarning() << "stackedWidget current change not implemented for this index";
        return;
    }
    }// e.o. switch
}

// file menu
void MainWindow::on_action_back_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_create_new_DB_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
        return;
    busycursor b;
    closeDatabaseConnection();
    if( !create_DK_database(dbfile))
        exit(0x80070020);
    QSettings config;
    config.setValue("db/last", dbfile);
    open_databaseForApplication();
    showDbInStatusbar();

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_open_DB_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde vom Anwender ausgewählt";
        return;
    }
    busycursor b;
    open_databaseForApplication(dbfile);
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_create_anonymous_copy_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getSaveFileName(this, "Anonymisierte Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
        return;
    busycursor b;
    if( !create_DB_copy(dbfile, true))
    {
        QMessageBox::information(this, "Fehler beim Kopieren", "Die anonymisierte Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei");
        qCritical() << "creating depersonaliced copy failed";
    }
    return;
}
void MainWindow::on_action_create_copy_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getSaveFileName(this, "Kopie der Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
        return;

    busycursor b;
    if( !create_DB_copy(dbfile, false))
    {
        QMessageBox::information(this, "Fehler beim Kopieren", "Die Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei");
        qCritical() << "creating depersonaliced copy failed";
    }
    return;

}
void MainWindow::on_action_store_output_directory_triggered()
{LOG_ENTRY_and_EXIT;
    QString dir;
    QSettings config;
    dir = config.value("outdir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    dir = QFileDialog::getExistingDirectory(this, "Ausgabeverzeichnis", dir,
                                        QFileDialog::ShowDirsOnly
                                            | QFileDialog::DontResolveSymlinks);
    config.setValue("outdir", dir);
}
void MainWindow::on_action_exit_program_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

// person list page
void MainWindow::prepareCreditorsTableView()
{LOG_ENTRY_and_EXIT;
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
{LOG_ENTRY_and_EXIT;
    busycursor b;
    prepareCreditorsTableView();
    if( !ui->CreditorsTableView->currentIndex().isValid())
        ui->CreditorsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}

// helper fu
int MainWindow::getIdFromCreditorsList()
{LOG_ENTRY_and_EXIT;
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->CreditorsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->CreditorsTableView->model()->data(mi));
        return data.toInt();
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return -1;
}
// Kontext Menue in Kreditoren Tabelle
void MainWindow::on_CreditorsTableView_customContextMenuRequested(const QPoint &pos)
{LOG_ENTRY_and_EXIT;
    QModelIndex index = ui->CreditorsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid())
    {
        QVariant data(ui->CreditorsTableView->model()->data(index));
        bool canConvert(false); data.toInt(&canConvert);
        if( canConvert)
        {
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
{LOG_ENTRY_and_EXIT;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QVariant index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0));
    ui->lblPersId->setText(index.toString());

    init_creditor_form(index.toInt());
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}
void MainWindow::on_action_create_contract_for_creditor_triggered(int id)
{LOG_ENTRY_and_EXIT;
    busycursor b;
    fill_creditors_dropdown();
    fill_rates_dropdown();
    ui->leKennung->setText( proposeKennung());
    set_creditors_combo_by_id( id != -1 ? id : getIdFromCreditorsList());
    Contract cd; // this is to get the defaults of the class definition
    ui->deLaufzeitEnde->setDate(cd.LaufzeitEnde());
    ui->cbKFrist->setCurrentIndex(ui->cbKFrist->findText("6"));
    ui->deVertragsabschluss->setDate(cd.Vertragsabschluss());
    ui->chkbThesaurierend->setChecked(cd.Thesaurierend());

    ui->stackedWidget->setCurrentIndex(newContractIndex);
}
void MainWindow::on_action_delete_creditor_triggered()
{LOG_ENTRY_and_EXIT;
    QString msg( "Soll der Kreditgeber ");
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QString Vorname = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    msg += Vorname + " " + Nachname + " (id " + index + ") mit allen Verträgen und Buchungen gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditgeber löschen?", msg))
        return;
    busycursor b;
    if( Kreditor::Loeschen(index.toInt()))
        prepareCreditorsTableView();
    else
        Q_ASSERT(!bool("could not remove kreditor and contracts"));
}
void MainWindow::on_action_show_contracts_triggered()
{LOG_ENTRY_and_EXIT;
    busycursor b;
    QModelIndex mi(ui->CreditorsTableView->currentIndex());
    QString index = ui->CreditorsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->leVertraegeFilter->setText(index);
    on_action_show_list_of_contracts_triggered();
}
void MainWindow::on_leFilter_editingFinished()
{LOG_ENTRY_and_EXIT;
    busycursor b;
    prepareCreditorsTableView();
}
void MainWindow::on_pbPersonFilterZuruecksetzen_clicked()
{LOG_ENTRY_and_EXIT;
    busycursor b;
    ui->leFilter->setText("");
    prepareCreditorsTableView();
}

// new DK Geber
void MainWindow::on_action_create_new_creditor_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}
int  MainWindow::save_creditor()
{LOG_ENTRY_and_EXIT;

    Kreditor k;
    k.setValue("Vorname", ui->leVorname->text().trimmed());
    k.setValue("Nachname", ui->leNachname->text().trimmed());
    k.setValue("Strasse", ui->leStrasse->text().trimmed());
    k.setValue("Plz", ui->lePlz->text().trimmed());
    k.setValue("Stadt", ui->leStadt->text().trimmed());
    k.setUniqueDbValue("Email", ui->leEMail->text().trimmed().toLower());
    k.setValue("Anmerkung", ui->txtAnmerkung->toPlainText());
    k.setValue("IBAN", ui->leIban->text().trimmed());
    k.setValue("BIC", ui->leBic->text().trimmed());

    QString errortext;
    if( !k.isValid(errortext))
    {
        errortext = "Die Daten konnten nicht gespeichert werden: <br>" + errortext;
        QMessageBox::information(this, "Fehler", errortext );
        qDebug() << "prüfung der Kreditor Daten:" << errortext;
        return -1;
    }
    int kid = -1;
    if( ui->lblPersId->text() != "")
    {
        kid = ui->lblPersId->text().toInt();
        k.setValue("Id", kid);     // update not insert
        k.Update();
    }
    else
       kid = k.Speichern();

    if(kid == -1)
    {
        QMessageBox::information( this, "Fehler", "Der Datensatz konnte nicht gespeichert werden. "
                     "Ist die E-Mail Adresse einmalig? Gibt es die Adressdaten in der Datenbank bereits?"
                     "\nBitte überprüfen Sie ihre Eingaben");
        qCritical() << "Kreditgeber konnte nicht gespeichert werden";
        return -1;
    }

    return kid;
}
void MainWindow::empty_create_creditor_form()
{LOG_ENTRY_and_EXIT;
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
{LOG_ENTRY_and_EXIT;
    busycursor b;
    QSqlRecord rec = ExecuteSingleRecordSql(dkdbstructur["Kreditoren"].Fields(), "Id=" +QString::number(id));
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
{LOG_ENTRY_and_EXIT;
    empty_create_creditor_form();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_save_contact_go_contract_triggered()
{LOG_ENTRY_and_EXIT;
    int kid = save_creditor();
    if(  kid != -1)
    {
        empty_create_creditor_form();
        on_action_create_contract_for_creditor_triggered(kid);
    }
}
void MainWindow::on_action_save_contact_go_creditors_triggered()
{LOG_ENTRY_and_EXIT;
    if( save_creditor() != -1)
    {
        empty_create_creditor_form();
        on_action_Liste_triggered();
    }
}
void MainWindow::on_action_save_contact_go_new_creditor_triggered()
{LOG_ENTRY_and_EXIT;
    if( save_creditor() != -1)
        empty_create_creditor_form();
}

// neuer Vertrag
Contract MainWindow::get_contract_data_from_form()
{LOG_ENTRY_and_EXIT;
    int KreditorId = ui->comboKreditoren->itemData(ui->comboKreditoren->currentIndex()).toInt();
    QString Kennung = ui->leKennung->text();
    double Betrag = ui->leBetrag->text().remove('.').toDouble();
    bool thesaurierend = ui->chkbThesaurierend->checkState() == Qt::Checked;
    double Wert = thesaurierend ? Betrag : 0.;
    int ZinsId = ui->cbZins->itemData(ui->cbZins->currentIndex()).toInt();
    QDate Vertragsdatum = ui->deVertragsabschluss->date();

    int kFrist = ui->cbKFrist->currentData().toInt();
    QDate LaufzeitEnde = ui->deLaufzeitEnde->date();
    // ensure consistency:
    // kfrist == -1 -> LaufzeitEnde has to be valid and not 31.12.9999
    // kfrist > 0 -> LaufzeitEnde == 31.12.9999
    if( !LaufzeitEnde.isValid())
    {
        qDebug() << "LaufzeitEnde ungültig -> defaulting";
        if( kFrist != -1)
            LaufzeitEnde = EndOfTheFuckingWorld;
        else
            LaufzeitEnde = Vertragsdatum.addYears(5);
    }
    if( kFrist != -1 && LaufzeitEnde.isValid() && LaufzeitEnde != EndOfTheFuckingWorld)
    {
        qDebug() << "LaufzeitEnde gesetzt, aber KFrist nicht -1 -> kfrist korrigiert";
        kFrist = -1;
    }
    QDate StartZinsberechnung = EndOfTheFuckingWorld;

    return Contract(KreditorId, Kennung, Betrag, Wert, ZinsId, Vertragsdatum,
                   thesaurierend, false/*aktiv*/,StartZinsberechnung, kFrist, LaufzeitEnde);
}
bool MainWindow::save_new_contract()
{LOG_ENTRY_and_EXIT;
    Contract c =get_contract_data_from_form();

    QString errortext;
    if( !c.validateAndSaveNewContract(errortext))
    {
        QMessageBox::critical( this, "Fehler", errortext);
        return false;
    }
    else
    {
        if( !errortext.isEmpty())
            QMessageBox::information(this, "Warnung", errortext);
        return true;
    }
}
void MainWindow::empty_new_contract_form()
{LOG_ENTRY_and_EXIT;
    ui->leKennung->setText("");
    ui->leBetrag->setText("");
    ui->chkbThesaurierend->setChecked(true);
}
void MainWindow::on_deLaufzeitEnde_userDateChanged(const QDate &date)
{
    if( date == EndOfTheFuckingWorld)
    {
        if( ui->cbKFrist->currentIndex() == 0)
            ui->cbKFrist->setCurrentIndex(6);
    }
    else
        ui->cbKFrist->setCurrentIndex(0);
}
void MainWindow::on_cbKFrist_currentIndexChanged(int index)
{LOG_ENTRY_and_EXIT;
    if( -1 == ui->cbKFrist->itemData(index).toInt())
    {   // Vertragsende wird fest vorgegeben
        if( EndOfTheFuckingWorld == ui->deLaufzeitEnde->date())
        {
            ui->deLaufzeitEnde->setDate(QDate::currentDate().addYears(5));
        }
    }
    else
    {   // Vertragsende wird durch Kündigung eingeleitet
        ui->deLaufzeitEnde->setDate(EndOfTheFuckingWorld);
    }
}
void MainWindow::on_leBetrag_editingFinished()
{LOG_ENTRY_and_EXIT;
    ui->leBetrag->setText(QString("%L1").arg(ui->leBetrag->text().toDouble()));
}

// helper: switch to "Vertrag anlegen"
void MainWindow::fill_creditors_dropdown()
{LOG_ENTRY_and_EXIT;
    ui->comboKreditoren->clear();
    QList<QPair<int, QString>> Personen;
    Kreditor k; k.KreditorenMitId(Personen);
    for(auto Entry :Personen)
    {
        ui->comboKreditoren->addItem( Entry.second, QVariant((Entry.first)));
    }
}
void MainWindow::fill_rates_dropdown()
{LOG_ENTRY_and_EXIT;
    QList<ZinsAnzeigeMitId> InterrestCbEntries; interestRates_for_dropdown(InterrestCbEntries);
    ui->cbZins->clear();
    for(ZinsAnzeigeMitId Entry : InterrestCbEntries)
    {
        ui->cbZins->addItem(Entry.second, QVariant(Entry.first));
    }
    ui->cbZins->setCurrentIndex(InterrestCbEntries.count()-1);
}
void MainWindow::set_creditors_combo_by_id(int KreditorenId)
{LOG_ENTRY_and_EXIT;
    if( KreditorenId < 0) return;
    // select the correct person
    for( int i = 0; i < ui->comboKreditoren->count(); i++)
    {
        if( KreditorenId == ui->comboKreditoren->itemData(i))
        {
            ui->comboKreditoren->setCurrentIndex(i);
            break;
        }
    }
}

// leave new contract
void MainWindow::on_cancelCreateContract_clicked()
{LOG_ENTRY_and_EXIT;
    empty_new_contract_form();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_action_save_contract_go_contracts_triggered()
{LOG_ENTRY_and_EXIT;
    if( save_new_contract())
    {
        empty_new_contract_form();
        prepare_contracts_list_view();
        ui->stackedWidget->setCurrentIndex(ContractsListIndex);
    }
}
void MainWindow::on_action_save_contract_go_kreditors_triggered()
{LOG_ENTRY_and_EXIT;
    if( save_new_contract())
    {
        empty_new_contract_form();
        on_action_Liste_triggered();
    }
}
void MainWindow::on_action_save_contract_new_contract_triggered()
{LOG_ENTRY_and_EXIT;
    if( save_new_contract())
    {
        empty_new_contract_form();
        on_action_create_contract_for_creditor_triggered();
    }
}

// Liste der Verträge
void MainWindow::prepare_contracts_list_view()
{LOG_ENTRY_and_EXIT;
    busycursor b;
    QVector<dbfield> fields;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    fields.append(dkdbstructur["Vertraege"]["Kennung"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege"]["LaufzeitEnde"]);
    fields.append(dkdbstructur["Vertraege"]["Kfrist"]);
    tmp_ContractsModel = new QSqlQueryModel(ui->contractsTableView);
    tmp_ContractsModel->setQuery(contractList_SQL(fields, ui->leVertraegeFilter->text()));
    ui->contractsTableView->setModel(tmp_ContractsModel);
    ui->contractsTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->contractsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->contractsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contractsTableView->setAlternatingRowColors(true);
    ui->contractsTableView->setSortingEnabled(true);
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Betrag"]), new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Wert"]), new WertEuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Zinssaetze"]["Zinssatz"]), new PercentItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Vertragsdatum"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["LaufzeitEnde"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["aktiv"]), new ActivatedItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Kfrist"]), new KFristItemFormatter(ui->contractsTableView));
    ui->contractsTableView->resizeColumnsToContents();
    ui->contractsTableView->hideColumn(0);

    QSortFilterProxyModel *m=new QSortFilterProxyModel(this);
    m->setDynamicSortFilter(true);
    m->setSourceModel(tmp_ContractsModel);
    ui->contractsTableView->setModel(m);
    ui->contractsTableView->setSortingEnabled(true);
}
void MainWindow::on_action_show_list_of_contracts_triggered()
{LOG_ENTRY_and_EXIT;
    prepare_contracts_list_view();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(ContractsListIndex);
}
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{LOG_ENTRY_and_EXIT;
    QSqlRecord rec = tmp_ContractsModel->record(); // ugly, but qobject_cast does not work
    int indedOf_active_inModel = rec.indexOf("aktiv");
    int indexOf_kfrist_inModel = rec.indexOf("Kfrist");

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
int MainWindow::get_current_id_from_contracts_list()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}
QString MainWindow::prepare_overview_page(Uebersichten u)
{LOG_ENTRY_and_EXIT;

    QString lbl ("<html><body>"
                 "<style>h1 { padding: 2em; font-family: Verdana; font-size: large;} "
                 "table, td { padding: 10px; border-width: 1px; border-style: solid; border-color: black; } "
                 "th { padding: 10px; border-width: 1px; border-style: solid; border-color: black; } "
                "</style>");
    QLocale locale;

    switch( u )
    {
    case UEBERSICHT:
    {
        DbSummary dbs;
        calculateSummary(dbs);
        lbl += QString("<h2>Übersicht DKs und DK Geber</h2><br> Stand: ") + QDate::currentDate().toString("dd.MM.yyyy<br>") +
          "<table>" +

          "<tr><td>Anzahl DK Geber*innen: </td><td style='text-align: left;'>" + QString::number(dbs.AnzahlDkGeber) +"</td></tr>" +
          "<tr><td>Anzahl Direktkredite: </td><td style='text-align: left;'>" + QString::number(dbs.AnzahlAktive) +"</td></tr>" +
          "<tr><td>Summe Direktkredite:  </td><td style='text-align: right:'>" + locale.toCurrencyString(dbs.BetragAktive) +"</td></tr>" +
          "<tr><td>Wert der DK inklusive Zinsen</td><td style='text-align: right'>"+ locale.toCurrencyString(dbs.WertAktive) + "</td></tr>" +
          "<tr><td>Durchschnittlicher Zinssatz <small>(Gewichtet mit Vertragswert)</small></td><td style='text-align: right'>"+ QString::number(dbs.DurchschnittZins, 'f', 3) + "%</td></tr>" +
          "<tr><td>Jährliche Zinskosten</td><td style='text-align: right'>"+ locale.toCurrencyString(dbs.WertAktive * dbs.DurchschnittZins/100.) + "</td></tr>" +
          "<tr><td>Mittlerer Zinssatz</td><td style='text-align: right'>"+ QString::number(dbs.MittlererZins, 'f', 3) + "%</td></tr>" +
          "<tr></tr>" +
          "<tr><td>Anzahl der DK mit jährl. Zinsauszahlung: </td><td align:left>" + QString::number(dbs.AnzahlAuszahlende) +"</td></tr>" +
          "<tr><td>Summe: </td><td align:right>" + locale.toCurrencyString(dbs.BetragAuszahlende) +"</td></tr>" +
          "<tr></tr>" +

          "<tr><td>Anzahl der DK ohne jährl. Zinsauszahlung: </td><td align:left>" + QString::number(dbs.AnzahlThesaurierende) +"</td></tr>" +
          "<tr><td>Summe: </td><td align:right>" + locale.toCurrencyString(dbs.BetragThesaurierende) +"</td></tr>" +
          "<tr><td>Wert inkl. Zinsen: </td><td align:right>" + locale.toCurrencyString(dbs.WertThesaurierende) +"</td></tr>" +

          "<tr></tr>" +
          "<tr><td>Anzahl noch ausstehender (inaktiven) DK </td><td align:left>" + QString::number(dbs.AnzahlPassive) +"</td></tr>" +
          "<tr><td>Summe noch ausstehender (inaktiven) DK </td><td align:right>" + locale.toCurrencyString(dbs.BetragPassive) +"</td></tr>" +
               "</table>";
        break;
    }
    case VERTRAGSENDE:
    {
        lbl += "<h2>Auslaufende Verträge </h2> Stand:"  + QDate::currentDate().toString("dd.MM.yyyy<br>");
        QVector<ContractEnd> ce;
        calc_contractEnd(ce);
        if( !ce.isEmpty())
        {
            lbl += "<table><thead><tr><td> Jahr </td><td> Anzahl </td><td> Summe </td></tr></thead>";
            for( auto x: ce)
            {
                lbl += "<tr><td align:left>" + QString::number(x.year) + "</td><td align:center>" +
                       QString::number(x.count) + "</td><td align:right>" + locale.toCurrencyString(x.value) + "</td></tr>";
            }
            lbl += "</table>";
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
        if( !yzv.isEmpty())
        {
            lbl += "<h2>Verteilung der Zinssätze pro Jahr </h2> Stand:"  + QDate::currentDate().toString("dd.MM.yyyy<br>") +
             "<table>" +
             "<thead><tr><td style=\"padding:4px;\"> Jahr </td><td style=\"padding:4px;\"> Zinssatz </td><td style=\"padding:4px;\"> Anzahl</td><td style=\"padding:4px;\"> Summe </td></tr></thead>";
            for( auto x: yzv)
            {
                lbl += "<tr><td>" + QString::number(x.year) + "</td><td align:center>" +
                       QString::number(x.intrest) + " %</td><td align:center>" + QString::number(x.count) + "</td><td style='text-align:right'>" + locale.toCurrencyString(x.sum) + "</td></tr>";
            }
            lbl += "</table>";
        }
        break;
    }
    case LAUFZEITEN:
    {
        lbl += "<h2>Vertragslaufzeiten</h2> Stand:" + QDate::currentDate().toString("dd.MM.yyyy<br>");
        lbl += "<br>" + contractTerm_distirbution_html();
    }
//    default:
//    {
//        ;
//    }
    }
    lbl += "</body></html>";
    qDebug() << "\n" << lbl << endl;
    return lbl;
}
void MainWindow::on_action_contracts_statistics_triggered()
{LOG_ENTRY_and_EXIT;
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
{LOG_ENTRY_and_EXIT;
    on_action_contracts_statistics_triggered();
}
void MainWindow::on_pbPrint_clicked()
{LOG_ENTRY_and_EXIT;
    QSettings config;
    QString filename = config.value("outdir").toString();

    filename += "\\" + QDate::currentDate().toString("yyyy-MM-dd_");
    filename += Uebersichten_kurz[ui->comboUebersicht->currentIndex()];
    filename += ".pdf";
    QPdfWriter write(filename);
    ui->txtOverview->print(&write);
    showFileInFolder(filename);
}
void MainWindow::on_action_anual_interest_settlement_triggered()
{LOG_ENTRY_and_EXIT;
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
{LOG_ENTRY_and_EXIT;
    CsvActiveContracts();
}
// contract list context menu
void MainWindow::on_action_activate_contract_triggered()
{LOG_ENTRY_and_EXIT;

    if( !aktiviereVertrag(get_current_id_from_contracts_list()))
    {
        QMessageBox::information(nullptr, "Fehler beim Aktivieren des Vertrags",
           "Es ist ein Fehler bei der Aktivierung aufgetreten, bitte überprüfen sie das LOG");
    }
    prepare_contracts_list_view();
}
void MainWindow::on_action_loeschePassivenVertrag_triggered()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;

    QString index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    beendeVertrag(index.toInt());

    prepare_contracts_list_view();
}
void MainWindow::on_leVertraegeFilter_editingFinished()
{LOG_ENTRY_and_EXIT;
    prepare_contracts_list_view();
}
void MainWindow::on_reset_contracts_filter_clicked()
{LOG_ENTRY_and_EXIT;
    ui->leVertraegeFilter->setText("");
    prepare_contracts_list_view();
}
void MainWindow::on_action_terminate_contract_triggered()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    int index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toInt();
    bool ret = beendeVertrag(index);
    if( !ret)
    {
        QMessageBox::information(nullptr, "Fehler beim Beenden des Vertrags", "Es ist ein Fehler aufgetreten, bitte überprüfen sie das LOG");
    } else
    {
        QMessageBox::information(nullptr, "Beenden des Vertrags", "Das Beenden des Vertrags wurde erfolgreich gespeichert");
    }
    prepare_contracts_list_view();
}

// debug funktions
void MainWindow::on_action_create_sample_data_triggered()
{LOG_ENTRY_and_EXIT;
    create_sampleData();
    prepareCreditorsTableView();
    prepare_contracts_list_view();
    if( ui->stackedWidget->currentIndex() == OverviewIndex)
        on_action_contracts_statistics_triggered();
}
void MainWindow::on_action_log_anzeigen_triggered()
{LOG_ENTRY_and_EXIT;
    #if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
    #else
    QString cmd = "open " + logFilePath().toUtf8();
    system(cmd.toUtf8().constData());
    #endif
}
// bookings- bisher Debug stuff
void MainWindow::on_tblViewBookingsSelectionChanged(const QItemSelection& to, const QItemSelection& )
{
    QString json =ui->tblViewBookings->model()->data(to.indexes().at(0).siblingAtColumn(6)).toString();
    ui->lblYson->setText(json);
}
void MainWindow::on_actionShow_Bookings_triggered()
{LOG_ENTRY_and_EXIT;

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
