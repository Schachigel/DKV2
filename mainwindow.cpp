#include <QtCore>

#include "windows.h"
#include <qpair.h>
#include <qfiledialog.h>
#include <QRandomGenerator>
#include <QMessageBox>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qsqltablemodel.h>
#include <qsqlquerymodel.h>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <qmap.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "activatecontractdlg.h"

#include "dkdbhelper.h"
#include "filehelper.h"
#include "itemformatter.h"


void MainWindow::setCurrentDbInStatusBar()
{
    QSettings config;
    ui->statusLabel->setText(config.value("db/last").toString());
}

// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef QT_DEBUG
    ui->menuDebug->setTitle("Debug");
#endif

    ui->statusBar->addPermanentWidget(ui->statusLabel);
    setCurrentDbInStatusBar();

    setCentralWidget(ui->stackedWidget);
    openAppDefaultDb();

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
MainWindow::~MainWindow()
{
    delete ui;
}

// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    if( arg1 < 0)
    {
        qWarning() << "stackedWidget changed to non existing page";
        return;
    }
    switch(arg1)
    {
    case emptyPageIndex:
        ui->actionKreditgeber_l_schen->setEnabled(false);
        ui->actionVertrag_l_schen->setEnabled(false);
        break;
    case PersonListIndex:
        ui->actionKreditgeber_l_schen->setEnabled(true);
        ui->actionVertrag_l_schen->setEnabled(false);
        break;
    case newPersonIndex:
        ui->actionKreditgeber_l_schen->setEnabled(false);
        ui->actionVertrag_l_schen->setEnabled(false);
        break;
    case newContractIndex:
        ui->actionKreditgeber_l_schen->setEnabled(false);
        ui->actionVertrag_l_schen->setEnabled(false);
        break;
    case ContractsListIndex:
        ui->actionKreditgeber_l_schen->setEnabled(false);
        ui->actionVertrag_l_schen->setEnabled(true);
        break;
    default:
    {
        qWarning() << "stackedWidget current change not implemented for this index";
        return;
    }
    }// e.o. switch
}

// file menu
void MainWindow::on_action_Neue_DB_anlegen_triggered()
{
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
        return;

    createDKDB(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    openAppDefaultDb();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    setCurrentDbInStatusBar();
}
void MainWindow::on_actionDBoeffnen_triggered()
{
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde ausgewählt";
        return;
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    openAppDefaultDb(dbfile);
}
void MainWindow::on_actionProgramm_beenden_triggered()
{
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

// person list page
void MainWindow::preparePersonTableView()
{   QSqlTableModel* model = new QSqlTableModel(ui->PersonsTableView);
    model->setTable("Kreditoren");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->PersonsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->PersonsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->PersonsTableView->setModel(model);
    ui->PersonsTableView->hideColumn(0);
    ui->PersonsTableView->resizeColumnsToContents();
}
void MainWindow::on_action_Liste_triggered()
{
    preparePersonTableView();
    ui->stackedWidget->setCurrentIndex(PersonListIndex);
    if( !ui->PersonsTableView->currentIndex().isValid())
        ui->PersonsTableView->selectRow(0);
}
void MainWindow::on_actionKreditgeber_l_schen_triggered()
{
    QString msg( "Soll der Kreditgeber ");
    QModelIndex mi(ui->PersonsTableView->currentIndex());
    QString Vorname = ui->PersonsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->PersonsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->PersonsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    msg += Vorname + " " + Nachname + " (id " + index + ") mit allen Verträgen und Buchungen gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditgeber löschen?", msg))
        return;
    QSqlQuery deleteQ;

    if( !deleteQ.exec("DELETE FROM [Vertraege] WHERE [KreditorId]=" +index))
        qCritical() << "Delete Kreditor failed "<< deleteQ.lastError() << "\n" << deleteQ.lastQuery();

/*    if( !deleteQ.exec("DELETE FROM [Kreditoren] WHERE [id]=" + index))
    {
        qCritical() << "Delete Kreditor failed "<< deleteQ.lastError()<< "\n" << deleteQ.lastQuery();
        QSqlDatabase::database().rollback();
    }
    else
        QSqlDatabase::database().commit();
*/
    preparePersonTableView();
}
void MainWindow::on_PersonsTableView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->PersonsTableView->indexAt(pos).siblingAtColumn(0);
    if( index.isValid())
    {
        QVariant data(ui->PersonsTableView->model()->data(index));
        bool canConvert(false); data.toInt(&canConvert);
        if( canConvert)
        {
            QMenu menu( "PersonContextMenu", this);
            menu.addAction(ui->actionVertrag_anlegen);
            menu.addAction( ui->actionKreditgeber_l_schen);
            menu.exec(ui->PersonsTableView->mapToGlobal(pos));
        }
        else
            qCritical() << "Conversion error: model data is not int";
        return;
    }
}

// debug funktions
void MainWindow::on_actioncreateSampleData_triggered()
{
    createSampleDkDatabaseData();
    preparePersonTableView();
    prepareContractListView();
}
void MainWindow::on_actionanzeigenLog_triggered()
{
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
}


// new DK Geber
void MainWindow::on_actionNeuer_DK_Geber_triggered()
{
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}
bool MainWindow::savePerson()
{
    PersonData p{ -1/*unused*/, ui->leVorname->text(),
                 ui->leNachname->text(),
                 ui->leStrasse->text(),
                 ui->lePlz->text(),
                 ui->leStadt->text(),
                 ui->leIban->text(),
                 ui->leBic->text()};
    if( p.Vorname == "" || p.Nachname == "" || p.Strasse =="" || p.Plz == "" || p.Stadt == "")
    {
        QMessageBox(QMessageBox::Warning, "Daten nicht gespeichert", "Namens - und Adressfelder dürfen nicht leer sein");
        return false;
    }
    return savePersonDataToDb(p) != 0;
}
void MainWindow::clearEditPersonFields()
{
    ui->leVorname->setText("");
    ui->leNachname->setText("");
    ui->leStrasse->setText("");
    ui->lePlz->setText("");
    ui->leStadt->setText("");
    ui->leIban->setText("");
    ui->leBic->setText("");
}
void MainWindow::on_saveNew_clicked()
{
    if( savePerson())
    {
        clearEditPersonFields();
    }
}
void MainWindow::on_saveList_clicked()
{
    if( savePerson())
    {
        clearEditPersonFields();
        on_action_Liste_triggered();
    }
}
void MainWindow::on_saveExit_clicked()
{
    if( savePerson())
    {
        clearEditPersonFields();
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_cancel_clicked()
{
    clearEditPersonFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

// new Contract
void MainWindow::setContractFromUi(VertragsDaten& c)
{
    c.KreditorId = ui->comboKreditoren->itemData(ui->comboKreditoren->currentIndex()).toInt();
    c.Kennung = ui->leKennung->text();
    c.Betrag = ui->leBetrag->text().toFloat();
    c.Wert = c.Betrag;
    c.Zins = ui->cbZins->itemData(ui->cbZins->currentIndex()).toInt();
    c.tesaurierend = ui->chkbTesaurierend->checkState() == Qt::Checked;
    c.Vertragsdatum = ui->deVertragsabschluss->date();
    c.active = false;
    c.LaufzeitEnde = ui->deLaufzeitEnde->date();
    c.StartZinsberechnung = c.LaufzeitEnde;
}

bool MainWindow::saveNewContract()
{
    VertragsDaten c;
    setContractFromUi(c);

    QString errortext;
    if( c.Betrag <= 0)
        errortext = "Der Kreditbetrag muss größer als null sein";
    if( c.KreditorId <= 0 || c.Zins <= 0)
        errortext = "Wähle den Kreditgeber und die Zinsen aus der gegebenen Auswahl. Ist die Auswahl leer müssen zuerst Kreditgeber und Zinswerte eingegeben werden";
    if( c.Kennung =="")
        errortext = "Du solltest eine Kennung vergeben, damit der Kretit besser zugeordnet werden kann";
    if( errortext != "")
    {
        QMessageBox::information( this, "Fehler", errortext);
        return false;
    }
    return verbucheVertrag(c);
}
void MainWindow::clearNewContractFields()
{
    ui->leKennung->setText("");
    ui->leBetrag->setText("");
    ui->chkbTesaurierend->setChecked(true);
}

// switch to "Vertrag anlegen"
void MainWindow::FillKreditorDropdown()
{
    ui->comboKreditoren->clear();
    QList<PersonDispStringWithId>PersonEntries; AllPersonsForSelection(PersonEntries);
    for(PersonDispStringWithId Entry :PersonEntries)
    {
        ui->comboKreditoren->addItem( Entry.second, QVariant((Entry.first)));
    }
}
void MainWindow::FillRatesDropdown()
{
    QList<ZinsDispStringWithId> InterrestCbEntries; AllInterestRatesForSelection(InterrestCbEntries);
    ui->cbZins->clear();
    for(ZinsDispStringWithId Entry : InterrestCbEntries)
    {
        ui->cbZins->addItem(Entry.second, QVariant(Entry.first));
    }
}
void MainWindow::comboKreditorenAnzeigeNachKreditorenId(int KreditorenId)
{
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

void MainWindow::on_speichereVertragZurKreditorenListe_clicked()
{
    if( saveNewContract())
    {
        clearNewContractFields();
        ui->stackedWidget->setCurrentIndex(PersonListIndex);
    }
}
void MainWindow::on_saveContractGoContracts_clicked()
{
    if( saveNewContract())
    {
        clearNewContractFields();
        prepareContractListView();
        ui->stackedWidget->setCurrentIndex(ContractsListIndex);
    }
}
void MainWindow::on_cancelCreateContract_clicked()
{
    clearNewContractFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

int MainWindow::getPersonIdFromKreditorenList()
{
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->PersonsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->PersonsTableView->model()->data(mi));
        return data.toInt();
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return -1;
}
void MainWindow::on_actionVertrag_anlegen_triggered()
{
    FillKreditorDropdown();
    FillRatesDropdown();
    comboKreditorenAnzeigeNachKreditorenId( getPersonIdFromKreditorenList());
    VertragsDaten cd;
    ui->deLaufzeitEnde->setDate(cd.LaufzeitEnde);
    ui->deVertragsabschluss->setDate(cd.Vertragsdatum);
    ui->lblBeginZinsphase->setText("");
    ui->chkbTesaurierend->setChecked(cd.tesaurierend);

    ui->stackedWidget->setCurrentIndex(newContractIndex);
}

// List of contracts
void MainWindow::prepareContractListView()
{
    // why use a record here? so that we can use the indexOf function to set the formatter (just in case we need to reorder columns
    QSqlRecord record;
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("id"));
    record.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Vorname"));
    record.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Nachname"));
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("Betrag"));
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("Wert"));
    record.append(dkdbstructure.getTable("Zinssaetze").getQSqlFieldByName("Zinssatz"));
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("Vertragsdatum"));
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("LetzteZinsberechnung"));
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("aktiv"));
    record.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("LaufzeitEnde"));

    QString sql("SELECT ");
    for( int i = 0; i < record.count(); i++)
    {
        if( i) sql +=", ";
        sql += record.field(i).tableName() +"." +record.field(i).name();
    }
    sql += " FROM Vertraege, Kreditoren, Zinssaetze "
            "WHERE Kreditoren.id = Vertraege.KreditorId AND Vertraege.ZSatz = Zinssaetze.id "
            "ORDER BY Vertraege.aktiv, Kreditoren.Nachname";
    qDebug() << "sql to populate contract list: " << sql;

    QSqlQueryModel* model = new QSqlQueryModel(ui->contractsTableView);
    model->setQuery(sql);
    ui->contractsTableView->setModel(model);
    ui->contractsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->contractsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Vertraege.Betrag"), new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Vertraege.Wert"), new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Zinssaetze.Zinssatz"), new PercentItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Vertraege.Vertragsdatum"), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Vertraege.LaufzeitEnde"), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Vertraege.LetzteZinsberechnung"), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(record.indexOf("Vertraege.aktiv"), new ActivatedItemFormatter(ui->contractsTableView));
    ui->contractsTableView->resizeColumnsToContents();
    ui->contractsTableView->hideColumn(0);
}
void MainWindow::on_actionListe_der_Vertr_ge_anzeigen_triggered()
{
    prepareContractListView();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(ContractsListIndex);
}
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex indexClickTarget = ui->contractsTableView->indexAt(pos);
//    QModelIndex indexContractId = indexClickTarget.siblingAtColumn(0);
    QModelIndex indexActive = indexClickTarget.siblingAtColumn(8); // contract active

    QMenu menu( "PersonContextMenu", this);
    bool isActive (ui->contractsTableView->model()->data(indexActive).toInt() ? true : false);
    if( isActive)
    {
        menu.addAction(ui->actionVertrag_Beenden);
    }
    else
    {
        menu.addAction(ui->actionactivateContract);
        menu.addAction(ui->actionVertrag_l_schen); // passive Verträge können gelöscht werden
    }
    menu.exec(ui->PersonsTableView->mapToGlobal(pos));
    return;
}
int MainWindow::getContractIdStringFromContractsList()
{
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}
QDate MainWindow::getContractDateFromContractsList()
{
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(6));
    if( mi.isValid())
    {
        QVariant data{ui->contractsTableView->model()->data(mi)};
        return data.toDate();
    }
    return QDate();
}
void MainWindow::on_actionactivateContract_triggered()
{
    QDate contractDate = getContractDateFromContractsList();
    activateContractDlg dlg( this, contractDate);
    if( QDialog::Accepted == dlg.exec())
    {
        if( activateContract(getContractIdStringFromContractsList(), dlg.getActivationDate()))
            prepareContractListView();
    }
}
void MainWindow::on_actionVertrag_l_schen_triggered()
{
    QModelIndex mi(ui->contractsTableView->currentIndex());
    QString Vorname = ui->contractsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->contractsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toString();

    QString msg( "Soll der Vertrag von ");

    msg += Vorname + " " + Nachname + " (id " + index + ") gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditvertrag löschen", msg))
        return;
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Vertraege WHERE id=" + index))
    {
        qCritical() << "failed to delete person: " << deleteQ.lastError();
    }
    prepareContractListView();
}

