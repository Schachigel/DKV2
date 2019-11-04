#include <QtCore>

#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif
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

#include "askdatedlg.h"

#include "helper.h"
#include "filehelper.h"
#include "itemformatter.h"
#include "finhelper.h"
#include "vertrag.h"
#include "dkdbhelper.h"

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
    DbInStatuszeileAnzeigen();

    setCentralWidget(ui->stackedWidget);
    DatenbankZurAnwendungOeffnen();

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DbInStatuszeileAnzeigen()
{
    QSettings config;
    ui->statusLabel->setText(config.value("db/last").toString());
}

// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{    LOG_ENTRY_and_EXIT;
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
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
        return;

    DKDatenbankAnlegen(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    DatenbankZurAnwendungOeffnen();
    DbInStatuszeileAnzeigen();

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_actionDBoeffnen_triggered()
{LOG_ENTRY_and_EXIT;
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.dkdb", "dk-DB Dateien (*.dkdb)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde vom Anwender ausgewählt";
        return;
    }
    DatenbankZurAnwendungOeffnen(dbfile);

    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_actionProgramm_beenden_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    this->close();
}

// person list page
void MainWindow::preparePersonTableView()
{LOG_ENTRY_and_EXIT;
    QSqlTableModel* model = new QSqlTableModel(ui->PersonsTableView);
    model->setTable("Kreditoren");
    model->setFilter("Vorname LIKE '%" + ui->leFilter->text() + "%' OR Nachname LIKE '%" + ui->leFilter->text() + "%'");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->PersonsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->PersonsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->PersonsTableView->setAlternatingRowColors(true);
    ui->PersonsTableView->setSortingEnabled(true);
    ui->PersonsTableView->setModel(model);
    ui->PersonsTableView->hideColumn(0);
    ui->PersonsTableView->resizeColumnsToContents();
}
void MainWindow::on_action_Liste_triggered()
{LOG_ENTRY_and_EXIT;
    preparePersonTableView();
    if( !ui->PersonsTableView->currentIndex().isValid())
        ui->PersonsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}
void MainWindow::on_actionKreditgeber_l_schen_triggered()
{LOG_ENTRY_and_EXIT;
    QString msg( "Soll der Kreditgeber ");
    QModelIndex mi(ui->PersonsTableView->currentIndex());
    QString Vorname = ui->PersonsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->PersonsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->PersonsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    msg += Vorname + " " + Nachname + " (id " + index + ") mit allen Verträgen und Buchungen gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditgeber löschen?", msg))
        return;

    if( KreditorLoeschen(index))
        preparePersonTableView();
    else
        Q_ASSERT(!bool("could not remove kreditor and contracts"));
}
void MainWindow::on_actionVertraege_zeigen_triggered()
{
    QModelIndex mi(ui->PersonsTableView->currentIndex());
    QString index = ui->PersonsTableView->model()->data(mi.siblingAtColumn(0)).toString();
    ui->leVertrgeFilter->setText(index);
    on_actionListe_der_Vertr_ge_anzeigen_triggered();
}
void MainWindow::on_PersonsTableView_customContextMenuRequested(const QPoint &pos)
{LOG_ENTRY_and_EXIT;
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
            menu.addAction(ui->actionVertraege_zeigen);
            menu.exec(ui->PersonsTableView->mapToGlobal(pos));
        }
        else
            qCritical() << "Conversion error: model data is not int";
        return;
    }
}

// debug funktions
void MainWindow::on_actioncreateSampleData_triggered()
{LOG_ENTRY_and_EXIT;
    BeispieldatenAnlegen();
    preparePersonTableView();
    prepareContractListView();
}
void MainWindow::on_actionanzeigenLog_triggered()
{LOG_ENTRY_and_EXIT;
    #if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
    #else
    QString cmd = "open " + logFilePath().toUtf8();
    system(cmd.toUtf8().constData());
    #endif
}

// new DK Geber
void MainWindow::on_actionNeuer_DK_Geber_triggered()
{LOG_ENTRY_and_EXIT;
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}
bool MainWindow::KreditgeberSpeichern()
{LOG_ENTRY_and_EXIT;
    KreditorDaten p{ ui->leVorname->text(),
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
    return KreditorDatenSpeichern(p) != 0;
}
void MainWindow::KreditorFormulardatenLoeschen()
{LOG_ENTRY_and_EXIT;
    ui->leVorname->setText("");
    ui->leNachname->setText("");
    ui->leStrasse->setText("");
    ui->lePlz->setText("");
    ui->leStadt->setText("");
    ui->leIban->setText("");
    ui->leBic->setText("");
}
void MainWindow::on_saveNew_clicked()
{LOG_ENTRY_and_EXIT;
    if( KreditgeberSpeichern())
    {
        KreditorFormulardatenLoeschen();
    }
}
void MainWindow::on_saveList_clicked()
{LOG_ENTRY_and_EXIT;
    if( KreditgeberSpeichern())
    {
        KreditorFormulardatenLoeschen();
        on_action_Liste_triggered();
    }
}
void MainWindow::on_saveExit_clicked()
{LOG_ENTRY_and_EXIT;
    if( KreditgeberSpeichern())
    {
        KreditorFormulardatenLoeschen();
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
void MainWindow::on_cancel_clicked()
{LOG_ENTRY_and_EXIT;
    KreditorFormulardatenLoeschen();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

// new Contract
Vertrag MainWindow::VertragsdatenAusFormular()
{LOG_ENTRY_and_EXIT;
    int KreditorId = ui->comboKreditoren->itemData(ui->comboKreditoren->currentIndex()).toInt();
    QString Kennung = ui->leKennung->text();
    double Betrag = ui->leBetrag->text().toDouble();
    double Wert = Betrag;
    int ZinsId = ui->cbZins->itemData(ui->cbZins->currentIndex()).toInt();
    bool tesaurierend = ui->chkbTesaurierend->checkState() == Qt::Checked;
    QDate Vertragsdatum = ui->deVertragsabschluss->date();

    QDate LaufzeitEnde = ui->deLaufzeitEnde->date();
    QDate StartZinsberechnung = LaufzeitEnde;

    return Vertrag(KreditorId, Kennung, Betrag, Wert, ZinsId, Vertragsdatum,
                   false/*aktiv*/, tesaurierend, StartZinsberechnung, LaufzeitEnde);
}

bool MainWindow::saveNewContract()
{LOG_ENTRY_and_EXIT;
    Vertrag c =VertragsdatenAusFormular();

    QString errortext;
    if( c.Betrag() <= 0)
        errortext = "Der Kreditbetrag muss größer als null sein";
    if( c.KreditorId() <= 0 || c.ZinsId() <= 0)
        errortext = "Wähle den Kreditgeber und die Zinsen aus der gegebenen Auswahl. Ist die Auswahl leer müssen zuerst Kreditgeber und Zinswerte eingegeben werden";
    if( c.Kennung() =="")
        errortext = "Du solltest eine Kennung vergeben, damit der Kretit besser zugeordnet werden kann";
    if( errortext != "")
    {
        QMessageBox::information( this, "Fehler", errortext);
        return false;
    }
    return c.speichernAlsNeuenVertrag();
}
void MainWindow::clearNewContractFields()
{LOG_ENTRY_and_EXIT;
    ui->leKennung->setText("");
    ui->leBetrag->setText("");
    ui->chkbTesaurierend->setChecked(true);
}

// switch to "Vertrag anlegen"
void MainWindow::FillKreditorDropdown()
{LOG_ENTRY_and_EXIT;
    ui->comboKreditoren->clear();
    QList<KreditorAnzeigeMitId>PersonEntries; KreditorenFuerAuswahlliste(PersonEntries);
    for(KreditorAnzeigeMitId Entry :PersonEntries)
    {
        ui->comboKreditoren->addItem( Entry.second, QVariant((Entry.first)));
    }
}
void MainWindow::FillRatesDropdown()
{LOG_ENTRY_and_EXIT;
    QList<ZinsAnzeigeMitId> InterrestCbEntries; ZinssaetzeFuerAuswahlliste(InterrestCbEntries);
    ui->cbZins->clear();
    for(ZinsAnzeigeMitId Entry : InterrestCbEntries)
    {
        ui->cbZins->addItem(Entry.second, QVariant(Entry.first));
    }
}
void MainWindow::comboKreditorenAnzeigeNachKreditorenId(int KreditorenId)
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

void MainWindow::on_speichereVertragZurKreditorenListe_clicked()
{
    if( saveNewContract())
    {
        clearNewContractFields();
        ui->stackedWidget->setCurrentIndex(PersonListIndex);
        return;
    }
    // todo: issue a message an stop processing
}
void MainWindow::on_saveContractGoContracts_clicked()
{LOG_ENTRY_and_EXIT;
    if( saveNewContract())
    {
        clearNewContractFields();
        prepareContractListView();
        ui->stackedWidget->setCurrentIndex(ContractsListIndex);
        return;
    }
    // todo: issue a message an stop processing

}
void MainWindow::on_cancelCreateContract_clicked()
{LOG_ENTRY_and_EXIT;
    clearNewContractFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

int MainWindow::getPersonIdFromKreditorenList()
{LOG_ENTRY_and_EXIT;
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
{LOG_ENTRY_and_EXIT;
    FillKreditorDropdown();
    FillRatesDropdown();
    comboKreditorenAnzeigeNachKreditorenId( getPersonIdFromKreditorenList());
    Vertrag cd; // this is to get the defaults of the class definition
    ui->deLaufzeitEnde->setDate(cd.LaufzeitEnde());
    ui->deVertragsabschluss->setDate(cd.Vertragsabschluss());
    ui->lblBeginZinsphase->setText("");
    ui->chkbTesaurierend->setChecked(cd.Tesaurierend());

    ui->stackedWidget->setCurrentIndex(newContractIndex);
}

// List of contracts
void MainWindow::prepareContractListView()
{LOG_ENTRY_and_EXIT;
    QVector<dbfield> fields;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege"]["LaufzeitEnde"]);

    QSqlQueryModel* model = new QSqlQueryModel(ui->contractsTableView);
    model->setQuery(ContractList_SQL(fields, ui->leVertrgeFilter->text()));

    ui->contractsTableView->setModel(model);
    ui->contractsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->contractsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->contractsTableView->setAlternatingRowColors(true);
    ui->contractsTableView->setSortingEnabled(true);
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Betrag"]), new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Wert"]), new EuroItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Zinssaetze"]["Zinssatz"]), new PercentItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["Vertragsdatum"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["LaufzeitEnde"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]), new DateItemFormatter(ui->contractsTableView));
    ui->contractsTableView->setItemDelegateForColumn(fields.indexOf(dkdbstructur["Vertraege"]["aktiv"]), new ActivatedItemFormatter(ui->contractsTableView));
    ui->contractsTableView->resizeColumnsToContents();
    ui->contractsTableView->hideColumn(0);

    QSortFilterProxyModel *m=new QSortFilterProxyModel(this);
    m->setDynamicSortFilter(true);
    m->setSourceModel(model);
    ui->contractsTableView->setModel(m);
    ui->contractsTableView->setSortingEnabled(true);
}
void MainWindow::on_actionListe_der_Vertr_ge_anzeigen_triggered()
{LOG_ENTRY_and_EXIT;
    prepareContractListView();
    if( !ui->contractsTableView->currentIndex().isValid())
        ui->contractsTableView->selectRow(0);

    ui->stackedWidget->setCurrentIndex(ContractsListIndex);
}
void MainWindow::on_contractsTableView_customContextMenuRequested(const QPoint &pos)
{LOG_ENTRY_and_EXIT;
    QModelIndex indexClickTarget = ui->contractsTableView->indexAt(pos);
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
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(0));
    if( mi.isValid())
    {
        QVariant data(ui->contractsTableView->model()->data(mi));
        return data.toInt();
    }
    return -1;
}
QDate MainWindow::getContractDateFromContractsList()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex().siblingAtColumn(6));
    if( mi.isValid())
    {
        QVariant data{ui->contractsTableView->model()->data(mi)};
        return data.toDate();
    }
    return QDate();
}
void MainWindow::on_actionactivateContract_triggered()
{LOG_ENTRY_and_EXIT;
    QDate contractDate = getContractDateFromContractsList();
    askDateDlg dlg( this, contractDate);
    if( QDialog::Accepted == dlg.exec())
    {
        if( Vertrag::aktiviereVertrag(getContractIdStringFromContractsList(), dlg.getDate()))
            prepareContractListView();
    }
}
void MainWindow::on_actionVertrag_l_schen_triggered()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;
    QString Vorname = ui->contractsTableView->model()->data(mi.siblingAtColumn(1)).toString();
    QString Nachname = ui->contractsTableView->model()->data(mi.siblingAtColumn(2)).toString();
    QString index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toString();

    QString msg( "Soll der Vertrag von ");

    msg += Vorname + " " + Nachname + " (id " + index + ") gelöscht werden?";
    if( QMessageBox::Yes != QMessageBox::question(this, "Kreditvertrag löschen", msg))
        return;
    // passivenVertragLoeschen(index);
    Vertrag::passivenVertragLoeschen(index.toInt());
    prepareContractListView();
}

void MainWindow::on_leFilter_editingFinished()
{LOG_ENTRY_and_EXIT;
    preparePersonTableView();
}

void MainWindow::on_pbPersonFilterZurcksetzten_clicked()
{LOG_ENTRY_and_EXIT;
    ui->leFilter->setText("");
    preparePersonTableView();
}

void MainWindow::on_leVertrgeFilter_editingFinished()
{LOG_ENTRY_and_EXIT;
    prepareContractListView();
}

void MainWindow::on_FilterVertrgeZurcksetzten_clicked()
{LOG_ENTRY_and_EXIT;
    ui->leVertrgeFilter->setText("");
    prepareContractListView();
}

void MainWindow::on_actionVertrag_Beenden_triggered()
{LOG_ENTRY_and_EXIT;
    QModelIndex mi(ui->contractsTableView->currentIndex());
    if( !mi.isValid()) return;    // Vertrag beenden -> Zins berechnen und m Auszahlungsbetrag anzeigen, dann löschen
    int index = ui->contractsTableView->model()->data(mi.siblingAtColumn(0)).toInt();

    Vertrag v;
    v.ausDb(index, true);
    double WertBisHeute = v.Wert() + ZinsesZins(v.Zinsfuss(), v.Wert(), v.StartZinsberechnung(), QDate::currentDate(), v.Tesaurierend());
    QString getDateMsg("<h2>Wenn Sie einen Vertrag beenden wird der Zins abschließend"
                " berechnet und der Auszahlungsbetrag ermittelt.<br></h2>"
                "Um den Vertrag von %1 %2 mit dem aktuellen Wert %3 Euro jetzt zu beenden "
                "wählen Sie das Datum des Vertragendes ein und klicken Sie OK");
    getDateMsg = getDateMsg.arg(v.Vorname(), v.Nachname(), QString::number(WertBisHeute));

    askDateDlg dlg( this, QDate::currentDate());
    dlg.setMsg(getDateMsg);
    dlg.setDateLabel("Ende der Zinsberechnung:");
    if( QDialog::Accepted != dlg.exec())
    {
        qDebug() << "Delete contract was aborted by the user";
        return;
    }

    double davonZins =ZinsesZins(v.Zinsfuss(), v.Wert(), v.StartZinsberechnung(), dlg.getDate(), v.Tesaurierend());
    double neuerWert =v.Wert() +davonZins;

    QString confirmDeleteMsg("<h3>Vertragsabschluß</h3><br>Wert zum Vertragsende: %1 Euro<br>Zins der letzten Zinsphase: %2 Euro<br>"\
                             "Soll der Vertrag gelöscht werden?");
    confirmDeleteMsg = confirmDeleteMsg.arg(QString::number(neuerWert), QString::number(davonZins));
    if( QMessageBox::Yes != QMessageBox::question(this, "Vertrag löschen?", confirmDeleteMsg))
        return;
}

QString prepareOverviewPage()
{LOG_ENTRY_and_EXIT;
    // Summe der Direktkredite,
    // versprochene DK (inaktive DK)
    // Zinsen pa, davon auszuzahlen
    DbSummary dbs;
    berechneZusammenfassung(dbs);
    QString lbl ("<html><body><H1>Übersicht</H1>"
                "Summe aller Direktkredite: " + QString::number(dbs.aktiveDk) +"<br>" +
                "Summe der noch ausstehenden (passiven) DK: " + QString::number(dbs.passiveDk) +"<br>" +
                "Summe der DK inclusive erworbener Zinsen: "+ QString::number(dbs.WertAktiveDk) +
                "</body></html>");
    return lbl;
}

void MainWindow::on_action_bersicht_triggered()
{LOG_ENTRY_and_EXIT;
    ui->lblOverview->setText( prepareOverviewPage());
    ui->stackedWidget->setCurrentIndex(OverviewIndex);
}

