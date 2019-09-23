#include <QtCore>

#include "dkdbhelper.h"
#include "filehelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
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
#include <qmap.h>

void MainWindow::setCurrentDbInStatusBar()
{
    QSettings config;
    ui->statusLabel->setText(config.value("db/last").toString());
}

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

void MainWindow::on_action_Neue_DB_anlegen_triggered()
{
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr);
    if( dbfile == "")
        return;

    backupFile(dbfile);
    createDKDB(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    openAppDefaultDb();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    setCurrentDbInStatusBar();
}

void MainWindow::on_actionProgramm_beenden_triggered()
{
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

void MainWindow::on_actionDBoeffnen_triggered()
{
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde ausgewählt";
        return;
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    openAppDefaultDb(dbfile);
}

void MainWindow::preparePersonTableView()
{   QSqlTableModel* model = new QSqlTableModel(ui->PersonsTableView);
    //model->setQuery("SELECT Vorname, Name, Strasse, PLZ, Stadt FROM DkGeber");
    model->setTable("DKGeber");
    model->select();

    ui->PersonsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->PersonsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->PersonsTableView->setModel(model);
    ui->PersonsTableView->hideColumn(0);
}

void MainWindow::on_action_Liste_triggered()
{
    preparePersonTableView();
    ui->stackedWidget->setCurrentIndex(PersonListIndex);
    if( !ui->PersonsTableView->currentIndex().isValid())
        ui->PersonsTableView->selectRow(0);
}

void MainWindow::on_actioncreateSampleData_triggered()
{
    createSampleDkDatabaseData();
    static_cast<QSqlTableModel*>(ui->PersonsTableView->model())->select();
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

bool MainWindow::saveNewContract()
{
    ContractData c;
    c.DKGeberId = ui->cbDKGeber->itemData(ui->cbDKGeber->currentIndex()).toInt();
    c.Kennung = ui->leKennung->text();
    c.Betrag = ui->leBetrag->text().toFloat();
    c.Wert = c.Betrag;
    c.Zins = ui->cbZins->itemData(ui->cbZins->currentIndex()).toInt();
    c.tesaurierend = ui->chkbTesaurierend->checkState() == Qt::Checked;
    c.Vertragsdatum = ui->deVertragsabschluss->date();
    c.active = false;
    c.LaufzeitEnde = ui->deLaufzeitEnde->date();
    c.StartZinsberechnung = c.LaufzeitEnde;

    QString errortext;
    if( c.Betrag <= 0)
        errortext = "Der Kreditbetrag muss größer als null sein";
    if( c.DKGeberId <= 0 || c.Zins <= 0)
        errortext = "Wähle den Kreditgeber und die Zinsen aus der gegebenen Auswahl. Ist die Auswahl leer müssen zuerst Kreditgeber und Zinswerte eingegeben werden";
    if( c.Kennung =="")
        errortext = "Du solltest eine Kennung vergeben, damit der Kretit besser zugeordnet werden kann";
    if( errortext != "")
    {
        QMessageBox::information( this, "Fehler", errortext);
        return false;
    }
    return saveContractDataToDb(c);
}

void MainWindow::on_actionNeuer_DK_Geber_triggered()
{
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}

void MainWindow::on_saveExit_clicked()
{
    if( savePerson())
    {
        clearEditPersonFields();
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
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

void MainWindow::clearNewContractFields()
{
    ui->leKennung->setText("");
    ui->leBetrag->setText("");
    ui->chkbTesaurierend->setChecked(true);
}


// Buttons on "DKGeber anlegen
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

void MainWindow::on_cancel_clicked()
{
    clearEditPersonFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
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
        break;
    case PersonListIndex:
        break;
    case newPersonIndex:
        break;
    case newContractIndex:
        break;
    default:
    {
        qWarning() << "stackedWidget current change not implemented for this index";
        return;
    }
    }// e.o. switch
}

// switch to "Vertrag anlegen"

void MainWindow::FillDKGeberCombo()
{
    ui->cbDKGeber->clear();
    QList<PersonDispStringWithId>PersonEntries; AllPersonsForSelection(PersonEntries);
    for(PersonDispStringWithId Entry :PersonEntries)
    {
        ui->cbDKGeber->addItem( Entry.second, QVariant((Entry.first)));
    }
}

void MainWindow::FillRatesCombo()
{
    QList<ZinsDispStringWithId> InterrestCbEntries; AllInterestRatesForSelection(InterrestCbEntries);
    ui->cbZins->clear();
    for(ZinsDispStringWithId Entry : InterrestCbEntries)
    {
        ui->cbZins->addItem(Entry.second, QVariant(Entry.first));
    }
}

void MainWindow::SelectcbDKGeberComboByPersonId(int DKGeberId)
{
    if( DKGeberId < 0) return;
    // select the correct person
    for( int i = 0; i < ui->cbDKGeber->count(); i++)
    {
        if( DKGeberId == ui->cbDKGeber->itemData(i))
        {
            ui->cbDKGeber->setCurrentIndex(i);
            break;
        }
    }
}

int MainWindow::getPersonIdFromDKGeberList()
{
    // What is the persId of the currently selected person in the person?
    QModelIndex mi(ui->PersonsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->PersonsTableView->model()->data(mi));
        bool canConvert(false); data.toInt(&canConvert);
        if( canConvert)
            return data.toInt();
        qCritical() << "Conversion error: model data is not int";
        return -1;
    }
    qCritical() << "Index der Personenliste konnte nicht bestimmt werden";
    return -1;
}

void MainWindow::on_actionVertrag_anlegen_triggered()
{
    FillDKGeberCombo();
    FillRatesCombo();
    SelectcbDKGeberComboByPersonId( getPersonIdFromDKGeberList());
    ContractData cd;
    ui->deLaufzeitEnde->setDate(cd.LaufzeitEnde);
    ui->deVertragsabschluss->setDate(cd.Vertragsdatum);
    ui->lblBeginZinsphase->setText("");
    ui->chkbTesaurierend->setChecked(cd.tesaurierend);

    ui->stackedWidget->setCurrentIndex(newContractIndex);
}

// Buttons on the "Vertrag anlegen" page
void MainWindow::on_cancelCreateContract_clicked()
{
    // nicht speichern. welchseln zur leeren Seite
    clearNewContractFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

void MainWindow::on_saveContractGoToDKGeberList_clicked()
{
    saveNewContract();
    clearNewContractFields();
    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}

void MainWindow::on_saveContractGoContracts_clicked()
{
    // speichern und zur Liste der Verträge wechseln
}

void MainWindow::prepareContractListView()
{
    QSqlQueryModel* model = new QSqlQueryModel(ui->contractsTableView);
    model->setQuery("SELECT DKGeber.Vorname, DKGeber.Nachname, DKVertrag.Betrag, DKVertrag.Wert, "
                    "DKZinssaetze.Zinssatz, DKVertrag.Vertragsdatum, DKVertrag.LaufzeitEnde, "
                    "DKZinssaetze.Zinssatz FROM DKGeber, DKVertrag, DKZinssaetze "
                    "WHERE DKGeber.id = DKVertrag.DKGeberId AND DKVertrag.ZSatz = DKZinssaetze.id");
    ui->contractsTableView->setModel(model);
}

void MainWindow::on_actionListe_der_Vertr_ge_anzeigen_triggered()
{
    prepareContractListView();
    ui->stackedWidget->setCurrentIndex(ContractsListIndex);
}
