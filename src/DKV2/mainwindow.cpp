#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qapplication.h"
#include "qnamespace.h"
#include <QtGlobal>

#if defined(Q_OS_WIN)
#else
#include <stdlib.h>
#endif

#include "busycursor.h"
#include "helperfile.h"
#include "filewriter.h"
#include "helpersql.h"
#include "appconfig.h"
#include "investment.h"
#include "opendatabase.h"
#include "wiznewdatabase.h"
#include "dlgaskdate.h"
#include "dlgabout.h"
#include "uiitemformatter.h"
#include "dkdbhelper.h"
#include "dkdbviews.h"
#include "dkdbcopy.h"
#include "transaktionen.h"
#include "annual_letters.h"
#include "uebersichten.h"


QVariant InvestmentsTableModel::data(const QModelIndex& i, int role) const
{
    // change font color for number of contracts (3,5) and sum of contract (4,6) columns
    // change font color for outdated investments
    static QDate today =QDate::currentDate();
    if (role == Qt::ForegroundRole) {
        int col =i.column();
        if( col == 4 or col == 6) {
            int nbr =i.data().toInt();
            if(nbr >= iMax){
                //qInfo() << "nbr: " << nbr << " row: " << col;
                return QColor(Qt::red);
            }
        } else if (col == 5 or col == 7) {
            double sum =i.data().toDouble();
            if( sum >= dMax){
                //qInfo() << "sum: " << sum << " row: " << col;
                return QColor(Qt::red);
            }
        } else if (col == 2)  {
            // end date should be bigger than current date
            if( i.siblingAtColumn(8).data().toString() == qsl("Offen")) {
                QDate enddate =i.data().toDate();
                if( not enddate.isValid () or enddate == EndOfTheFuckingWorld)
                    return QColor(Qt::black);
                if( enddate <= today)
                    return QColor(Qt::red);
            }
        }
    }
    if( role == Qt::TextAlignmentRole) {
        switch(i.column()){
        case 0:
        case 1:
        case 2:
        case 4:
        case 6:
        case 8:
        case 9:
        case 10:
            return Qt::AlignCenter;
        case 5:
        case 7:
            return QVariant(Qt::AlignRight|Qt::AlignVCenter);
        case 3:
            return QVariant(Qt::AlignLeft|Qt::AlignVCenter);
        default:
            qInfo() << "wrong column " << i.column();
            return Qt::AlignCenter;
        }
    }
    return QSqlTableModel::data(i, role); // forward everthing else to the base class
}

void MainWindow::useDb(const QString& dbfile)
{   LOG_CALL;
    add_MRU_entry (dbfile);
    showDbInStatusbar(dbfile);
    prepare_startPage();
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}

// construction, destruction
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   LOG_CALL;
    ui->setupUi(this);
#ifndef QT_DEBUG
    ui->action_menu_debug_create_sample_data->setVisible(false);
#endif

#ifdef Q_OS_MAC
    ui->action_menu_contracts_statistics_view->setText(QStringLiteral("Uebe&rsichten"));
#endif

    ui->statusBar->addPermanentWidget(ui->statusLabel);
    setCentralWidget(ui->stackedWidget);
    ui->stackedWidget->setFont(QApplication::font ());
    // //////////////////
    const QString tableCellStyle {qsl("QTableView::item { padding-top: 5px; padding-bottom: 5px; padding-right: 10px; padding-left: 10px; }")};
    ui->CreditorsTableView->setStyleSheet(tableCellStyle);
    ui->contractsTableView->setStyleSheet(tableCellStyle);
    // re-resize columns and rows after sorting
    contractsSortingAdapter =std::make_unique<contractsHeaderSortingAdapter>(ui->contractsTableView);

    ui->bookingsTableView->setItemDelegateForColumn(2, new bookingTypeFormatter);
    ui->bookingsTableView->setStyleSheet(tableCellStyle);
    ui->InvestmentsTableView->setStyleSheet(tableCellStyle);
    ui->InvestmentsTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    ui->InvestmentsTableView->setStyleSheet(tableCellStyle);

    QSettings settings;
    restoreGeometry(settings.value(qsl("geometry")).toByteArray());
    restoreState(settings.value(qsl("windowState")).toByteArray());

    if( openDB_atStartup()) {
        useDb(appconfig::LastDb ());
        dbLoadedSuccessfully =true;
    }
}

MainWindow::~MainWindow()
{   LOG_CALL;
    deleteSignalFile();
    if (ui) delete ui;
}

void MainWindow::showDbInStatusbar( const QString &filename)
{   LOG_CALL_W (filename);
    Q_ASSERT( filename.size());
    QString statusstring ={qsl("%1 (%2)").arg(filename, dbConfig::readString(ZINSUSANCE))};
    ui->statusLabel->setText( statusstring);
}

// whenever the stackedWidget changes ...
void MainWindow::on_stackedWidget_currentChanged(int arg1)
{   LOG_CALL;
    if( arg1 < 0) {
        qWarning() << "stackedWidget changed to non existing page";
        return;
    }
//    switch(arg1)
//    {
//    case startPageIndex:
//        break;
//    case creditorsListPageIndex:
//        break;
//    case contractsListPageIndex:
//        break;
//    case overviewsPageIndex:
//        break;
//    case statisticsPageIndex:
//        break;
//    case printPreviewPageIndex:
//        break;
//    case investmentsPageIndex:
//        break;
//    default:
//        qWarning() << "stackedWidget current change not implemented for this index " << arg1;
//    }// e.o. switch
    return;
}

// when data was changed in a wizzard and the view has to be updated
void MainWindow::updateViews()
{
    QSqlTableModel* temp;

    if( ui->stackedWidget->currentIndex() == creditorsListPageIndex) {
        if( (temp =qobject_cast<QSqlTableModel*>(ui->CreditorsTableView->model())))
            temp->select();
    }
    if( ui->stackedWidget->currentIndex() == contractsListPageIndex) {
        if( showDeletedContracts) {
            prepare_deleted_contracts_list_view ();
        } else {
            prepare_contracts_list_view ();
//            ContractProxyModel *contractsTableProxy_ptr = qobject_cast<ContractProxyModel *>(ui->contractsTableView->model());
//            QSqlTableModel *bookingsTableModel_ptr = qobject_cast<QSqlTableModel *>(ui->bookingsTableView->model());
//            ContractTableModel *contractsTableModel_ptr = nullptr;
//            if (contractsTableProxy_ptr)
//                contractsTableModel_ptr = qobject_cast<ContractTableModel *>(contractsTableProxy_ptr->sourceModel());
//            if (contractsTableModel_ptr)
//                contractsTableModel_ptr->select();
//            if (bookingsTableModel_ptr)
//                bookingsTableModel_ptr->select();
//            ui->contractsTableView->resizeColumnsToContents();
//            ui->contractsTableView->resizeRowsToContents();
//            ui->bookingsTableView->resizeColumnsToContents();
//            if (contractsTableModel_ptr)
//                contractsTableModel_ptr->setCol13ExtraData();
        }
    }
    if( ui->stackedWidget->currentIndex() == investmentsPageIndex) {
        prepare_investmentsListView();
    }
    if( ui->stackedWidget->currentIndex() == overviewsPageIndex)
        on_action_menu_contracts_statistics_view_triggered();
    if( ui->stackedWidget->currentIndex() == overviewsPageIndex)
        updateUebersichtView(ui->comboUebersicht->currentIndex());
}

// the empty "welcome" page
void MainWindow::prepare_startPage()
{   LOG_CALL;
    busyCursor b;
    QString messageHtml {qsl("<h2>Willkommen zu DKV2- Deiner Verwaltung von Direktkrediten</h2>")};

    double allContractsValue =valueOfAllContracts();

    QString pName =dbConfig::readValue(projectConfiguration::GMBH_ADDRESS1).toString();
    if( pName.size()) {
         messageHtml += qsl("<p><b>DK Verwaltung für <font color=blue>%1</font></td>").arg(pName);
    }
    if( allContractsValue > 0) {
        QString valueRow = qsl("<p>Die Summer aller Direktkredite und Zinsen beträgt <big><font color=red>")
                + s_d2euro(allContractsValue) + qsl("</font></big>");
        messageHtml += valueRow;
    }
    messageHtml += qsl("<p>DB File: [ %1 ]").arg(ui->statusLabel->text ());
    ui->lblInfo->setText(messageHtml);
    }
void MainWindow::on_action_menu_database_start_triggered()
{   LOG_CALL;
    prepare_startPage();
    ui->stackedWidget->setCurrentIndex(startPageIndex);
}

// Database Menu
QString askUserFilenameForCopy(const QString& title, bool onlyExistingFiles=false)
{   // this function is used with creaetDbCopy, createAnony.DbCopy but NOT newDb
    LOG_CALL;
    wizFileSelectionWiz wiz(getMainWindow());
    wiz.title =title;
    wiz.subtitle =qsl("Mit dieser Dialogfolge wählst Du eine DKV2 Datenbank aus");
    wiz.fileTypeDescription =qsl("dk-DB Dateien (*.dkdb)");
    if( (wiz.existingFile=onlyExistingFiles))
        wiz.bffTitle =qsl("Wähle eine existierende dkdb Datei aus");
    else
        wiz.bffTitle =qsl("Wähle eine dkdb Datei aus oder gib einen neuen Dateinamen ein");

    QFileInfo lastdb (appconfig::LastDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::DocumentsLocation));

    wiz.exec();
    return wiz.field(qsl("selectedFile")).toString();
}

void MainWindow::on_action_menu_database_new_triggered()
{   LOG_CALL;
    if( askUserForNextDb()) {
        useDb(appconfig::LastDb ());
        return;
    }

    if( appconfig::LastDb ().isEmpty()) {
        QMessageBox::information( this, qsl("Abbruch"), qsl("Die Datenbank konnte nicht geöffnet werden."
                                        " Die Anwendung muss beendet werden."));
        exit(1);
    } else {
       qInfo() << "askUserForNextDb was not successfull but lastDB still there -> do nothing";
    }
}

void MainWindow::on_action_menu_database_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserFilenameForCopy(qsl("Dateiname der Kopie Datenbank angeben."));
    if( dbfile.isEmpty ())
        return;

    busyCursor b;
    if( copy_Database_fromDefaultConnection(dbfile)){
        b.finish ();
        QMessageBox::information(this, qsl("Kopie angelegt"), qsl("Die Kopie ") +dbfile +qsl(" wurde erfolgreich angelegt"));
    }
    else {
        b.finish ();
        QMessageBox::information(this, qsl("Fehler beim Kopieren"), qsl("Die Datenbankkopie konnte nicht angelegt werden. "
                                                               "Weitere Info befindet sich in der LOG Datei"));
        qCritical() << "creating copy failed";
    }
    return;
}
void MainWindow::on_action_menu_database_anonymous_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserFilenameForCopy(qsl("Dateiname der Anonymisierten Kopie angeben."));
    if( dbfile.isEmpty ())
        return;
    busyCursor b;
    if( not copy_database_fDC_mangled(dbfile)) {
        b.finish ();
        QMessageBox::information(this, qsl("Fehler beim Kopieren"),
                                 qsl("Die anonymisierte Datenbankkopie konnte nicht angelegt werden. "
                                     "Weitere Info befindet sich in der LOG Datei"));
        qCritical() << "creating depersonaliced copy failed";
    } else {
        b.finish ();
        QMessageBox::information(this, qsl("Kopie angelegt"), qsl("Die Kopie ohne personenbezogene Daten ") +dbfile +qsl(" wurde erfolgreich angelegt"));
    }

    return;
}
void MainWindow::on_actionProjektkonfiguration_ndern_triggered()
{   LOG_CALL;
    wizConfigureProjectWiz wiz(getMainWindow());
    if(wiz.exec() == QDialog::Accepted)
        wiz.updateDbConfig();
    updateViews();
}
void MainWindow::on_action_menu_database_configure_outdir_triggered()
{   LOG_CALL;
    QString dir{appconfig::Outdir()};
    dir = QFileDialog::getExistingDirectory(this, qsl("Ausgabeverzeichnis"), dir,
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    appconfig::setOutDir(dir);
}
void MainWindow::on_action_menu_database_program_exit_triggered()
{   LOG_CALL;
    ui->stackedWidget->setCurrentIndex(startPageIndex);
    this->close();
}

/////////////////////////////////////////////////
// List of investments
/////////////////////////////////////////////////
void MainWindow::prepare_investmentsListView()
{
    InvestmentsTableModel* model = new InvestmentsTableModel(this);
    model->setTable(vnInvestmentsView);

    QTableView* tv =ui->InvestmentsTableView;
    tv->setModel(model);
    tv->setItemDelegateForColumn(0, new PercentFrom100sItemFormatter);
    model->setHeaderData(0, Qt::Horizontal, qsl("Zinssatz"), Qt::DisplayRole);
    tv->setItemDelegateForColumn(1, new DateItemFormatter);
    model->setHeaderData(1, Qt::Horizontal, qsl("Begin"), Qt::DisplayRole);
    tv->setItemDelegateForColumn(2, new DateItemFormatter);
    model->setHeaderData(2, Qt::Horizontal, qsl("Ende"), Qt::DisplayRole);
    model->setHeaderData (3, Qt::Horizontal, qsl("Bezeichner"), Qt::DisplayRole);
    model->setHeaderData(4, Qt::Horizontal, qsl("Anzahl\n(alle)"), Qt::DisplayRole);
    tv->setItemDelegateForColumn(5, new CurrencyFormatter);
    model->setHeaderData(5, Qt::Horizontal, qsl("Summe\n(alle Vertr.)"), Qt::DisplayRole);
    model->setHeaderData(6, Qt::Horizontal, qsl("Anzahl\n(aktive)"), Qt::DisplayRole);
    tv->setItemDelegateForColumn(7, new CurrencyFormatter);
    model->setHeaderData(7, Qt::Horizontal, qsl("Summe\n(aktive)"), Qt::DisplayRole);
    tv->setItemDelegateForColumn (8, new CurrencyFormatter);
    model->setHeaderData (8, Qt::Horizontal, qsl("Summe\nEinzahlungen"), Qt::DisplayRole);
    tv->setItemDelegateForColumn (9, new CurrencyFormatter);
    model->setHeaderData (9, Qt::Horizontal, qsl("Summe\nincl. Zins"), Qt::DisplayRole);
    tv->hideColumn(11);
    tv->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tv->setAlternatingRowColors(true);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    tv->resizeColumnsToContents();
}
void MainWindow::on_actionAnlagen_verwalten_triggered()
{   LOG_CALL;
    prepare_investmentsListView();
    ui->stackedWidget->setCurrentIndex(investmentsPageIndex);
}
void MainWindow::on_btnCreateFromContracts_clicked()
{   LOG_CALL;

    QStringList options {qsl("Fortlaufende Geldanlagen erstellen"), qsl("Zeitlich abgeschlossene Anlagen (1 Jahr) erstellen")};
    QInputDialog id(this);
    id.setInputMode (QInputDialog::InputMode::TextInput);
    id.setWindowTitle (qsl("Art der Anlage auswählen"));
    id.setLabelText (qsl("Wähle aus, ob zeitlich befristete oder fortlaufende Anlagen erzeugt werden sollen."));
    id.setComboBoxItems (options);
    bool ok =id.exec ();

//    QString item =QInputDialog::getItem(this, qsl("Art der Anlage auswählen"), qsl("Wähle aus, ob zeitlich befristete oder fortlaufende Anlagen erzeugt werden sollen."), options, 0, false, &ok);
    if( !ok)
        return;

    int newInvestments =createNewInvestmentsFromContracts(id.textValue () == options[0]);
    if( newInvestments == -1) {
        QMessageBox::critical(this, qsl("Fehler"), qsl("Beim Anlegen der Geldanlagen ist ein Fehler aufgetreten"));
        return;
    }
    if( newInvestments) {
        int i =automatchInvestmentsToContracts();
        if( i not_eq newInvestments)
            qCritical() << qsl("nicht alle Verträgen (%1) konnten Anlagen (%2) zugeordnet werden").arg(i2s(i), i2s(newInvestments));

        QMessageBox::information(this, qsl("Neue Anlageformen"), qsl("Es wurden ") +i2s(newInvestments) +qsl(" Anlage(n) angelegt."));
        prepare_investmentsListView();
    }
    else
        QMessageBox::information(this, qsl("Neue Anlageformen"), qsl("Es wurden keine neuen Anlageformen angelegt."));
}
void MainWindow::on_btnNewInvestment_clicked()
{   LOG_CALL;
    createInvestment();
    QSqlTableModel* m =qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model());
    m->select();
    ui->InvestmentsTableView->resizeColumnsToContents();
}
void MainWindow::on_btnAutoClose_clicked()
{
    dlgAskDate ad(this);
    ad.setDate(QDate::currentDate());
    if( ad.exec() not_eq QDialog::Accepted) {
        qInfo() << "auto close was cancled";
        return;
    }
    int changedSets =closeInvestmentsPriorTo(ad.date());
    if( 0 <= changedSets)
        QMessageBox::information(this, qsl("Änderung durchgeführt"), qsl("Es wurden %1 Geldanlagen geschlossen").arg(changedSets));
    else
        QMessageBox::information(this, qsl("Änderung nicht durchgeführt"), qsl("Es ist ein Fehler aufgetreten"));

    prepare_investmentsListView();
}
void MainWindow::on_btnAutoMatch_clicked()
{
    busyCursor b;
    int i =automatchInvestmentsToContracts();
    b.finish ();
    QMessageBox::information(this, qsl("Zugeordnete Verträge"),
                             qsl("Es wurden %1 Verträge passenden Geldanlagen zugeordnet.").arg(i2s(i)));
    b.set();
    prepare_investmentsListView();
}
void MainWindow::on_btnAlleLoeschen_clicked()
{
    if( QMessageBox::Yes == QMessageBox::question(this, qsl("Alle Löschen"), qsl("Sollen alle Anlagen unwiderruflich gelöscht werden?"))) {
        executeSql_wNoRecords (qsl("DELETE FROM Geldanlagen"));
        prepare_investmentsListView ();
    }
}


void MainWindow::on_InvestmentsTableView_customContextMenuRequested(QPoint pos)
{   LOG_CALL;
    QTableView* tv =ui->InvestmentsTableView;
    QModelIndex index =tv->indexAt(pos);

    QMenu cmenu( qsl("investmentsContextMenu"), this);
    ui->actionInvestmentLoeschen->setData(index);
    cmenu.addAction(ui->actionInvestmentLoeschen);
    cmenu.addAction(ui->actionInvestmentSchliessen);
    cmenu.addAction(ui->actionTyp_Bezeichnung_aendern);
    cmenu.addAction (ui->action_cmenu_Vertraege_anzeigen);

    cmenu.exec(ui->InvestmentsTableView->mapToGlobal(pos));
}

void MainWindow::on_actionInvestmentLoeschen_triggered()
{
    QModelIndex index =ui->actionInvestmentLoeschen->data().toModelIndex();
    QSqlTableModel* tm =qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model());
    QSqlRecord rec =tm->record(index.row());
    QDate dAnfang =rec.value(qsl("Anfang")).toDate();
    QString anfang =dAnfang.toString(qsl("yyyy.MM.dd"));
    QDate dEnde =rec.value(qsl("Ende")).toDate();
    QString ende =dEnde.toString(qsl("yyyy.MM.dd"));
    int zinssatz =rec.value(qsl("ZSatz")).toInt();
    QString typ =rec.value(qsl("Typ")).toString();

    QString msg{qsl("Soll die Anlage mit <b>%1% Zins</b>, <br>vom <b>%2 zum %3</b><br> mit dem Bezeichner <br><b>'%4'</b><br> gelöscht werden?")};
    msg =msg.arg(QString::number(zinssatz/100., 'f', 2), anfang, ende, typ);

    if( not dEnde.isValid ())
        msg =qsl("Soll die fortlaufende Anlage <b>'%1'</b> mit <b>%2% Zins</b> gelöscht werden?").arg(typ, QString::number(zinssatz/100., 'f', 2));

    if( QMessageBox::Yes == QMessageBox::question(this, qsl("Löschen"), msg)) {
        // delete the entry, update the view
        if( deleteInvestment(rec.value(qsl("AnlagenId")).toLongLong ())) {
            qInfo() << "removed investment row " << index.row();
            //tm->submitAll();
            tm->select();
        } else {
            QMessageBox::information (this, qsl("Fehler"), qsl("Beim Löschen ist ein Fehler aufgetreten - bitte schau in die LOG Datei!"));
            qWarning() << tm->lastError();
        }
    }
}
void MainWindow::on_actionInvestmentSchliessen_triggered()
{
    QModelIndex index =ui->actionInvestmentLoeschen->data().toModelIndex();
    QSqlTableModel* tm =qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model());
    QSqlRecord rec =tm->record(index.row());
    bool currentStatus =rec.value(qsl("Offen")).toString() == qsl("Offen");
    QDate dAnfang =rec.value(qsl("Anfang")).toDate();
    QString anfang =dAnfang.toString(qsl("yyyy.MM.dd"));
    QDate dEnde =rec.value(qsl("Ende")).toDate();
    QString ende =dEnde.toString(qsl("yyyy.MM.dd"));
    int zinssatz =rec.value(qsl("ZSatz")).toInt();
    QString typ =rec.value(qsl("Typ")).toString();
    QString msg{currentStatus
                ? qsl("Zu einer geschlossenen Anlage können keine weiteren Verträge mehr hinzugefügt werden.<p>Soll die Anlage mit <b>%1%</b><br>im Zeitraum vom %2 zum %3 <br>mit dem Typ <br><i><&nbsp;><&nbsp;><&nbsp;>'%4'</i>  <br> geschlossen werden?")
                : qsl("Wenn Du die Geldanlage wieder öffnest, dann könnn weitere Verträge hinzugefügt werden.<p>Soll die Anlage mit <b>%1%</b><br>im Zeitraum vom %2 zum %3 <br>mit dem Typ <br><i><&nbsp;><&nbsp;><&nbsp;>'%4'</i>  <br> geöffnet werden?")
               };
    msg =msg.arg(QString::number(zinssatz/100., 'f', 2), anfang, ende, typ);

    qlonglong rowid =rec.value(qsl("AnlagenId")).toLongLong ();
    if( QMessageBox::Yes == QMessageBox::question(this, qsl("Status ändern"), msg)) {
        bool result =currentStatus ? closeInvestment(rowid) : openInvestment(rowid);
//        bool result = currentStatus ? closeInvestment(zinssatz, dAnfang, dEnde, typ) : openInvestment(zinssatz, dAnfang, dEnde, typ);
        if( result) {
            qInfo() << "Investment status from investment " << rowid << " was changed to " << (currentStatus ? "closed" : "open");
            tm->select();
        } else {
            qWarning() << tm->lastError();
        }
    }

}
void MainWindow::on_actionTyp_Bezeichnung_aendern_triggered()
{
    QModelIndex index =ui->actionInvestmentLoeschen->data().toModelIndex();
    QSqlTableModel* model {qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model())};
    QString typ =ui->InvestmentsTableView->model()->data(index.siblingAtColumn(3)).toString();;
//    QString zinssatz =ui->InvestmentsTableView->model()->data(index.siblingAtColumn(0)).toString();
    QString zinssatz =qobject_cast<PercentFrom100sItemFormatter*>(ui->InvestmentsTableView->itemDelegateForIndex(index.siblingAtColumn(0)))
            ->displayText(model->data(index.siblingAtColumn(0)), QLocale());

    QString von =doFormatDateItem(model->data(index.siblingAtColumn(1)));
    QString bis =doFormatDateItem(model->data(index.siblingAtColumn(2)));

    QString msg {qsl("<table><tr><th>Neue Bezeichnung für den Anlage </th></tr><tr><td style=\"text-align:center\">mit %1 Zins</td></tr>"
                     "<tr><td>von %2 bis %3.</tr>"
                     "<tr><td>alter Wert: <i>'%4'</i></td></tr></table>")};

    QInputDialog id(this);
    id.setInputMode(QInputDialog::InputMode::TextInput);
    id.setWindowTitle(qsl("Geldanlagen"));
    id.setLabelText(msg.arg(zinssatz, von, bis, typ));
    id.setTextValue(typ);
    QLineEdit* leText =id.findChild<QLineEdit*>();
    if(leText) leText->setMaxLength(25);

    int idOk =id.exec();
    QString txt = id.textValue().trimmed();
    if( not idOk or txt.isEmpty())
        return;

    QSqlTableModel* tm =qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model());
    QSqlRecord rec =tm->record(index.row());
    QString sql(qsl("UPDATE Geldanlagen SET Typ =? WHERE rowid =%1").arg(rec.value(qsl("AnlagenId")).toString()));
    if( executeSql_wNoRecords(sql, {QVariant(txt)}))
        tm->select();
}
void MainWindow::on_action_cmenu_Vertraege_anzeigen_triggered()
{
    QModelIndex index =ui->actionInvestmentLoeschen->data().toModelIndex();
    QSqlTableModel* tm =qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model());
    QSqlRecord rec =tm->record(index.row());
    qlonglong id =rec.value(qsl("AnlagenId")).toLongLong ();
    ui->le_ContractsFilter->setText (qsl("Anlage:%1").arg(id));
    on_le_ContractsFilter_editingFinished();
    on_action_menu_contracts_listview_triggered();
}

/////////////////////////////////////////////////
// Übersichten
/////////////////////////////////////////////////
void MainWindow::on_action_menu_contracts_statistics_view_triggered()
{   LOG_CALL;
    QComboBox* combo =ui->comboUebersicht;
    if(combo->count() == 0) {

        combo->addItems(QStringList({qsl("Kurzinfo"),
                                     qsl("Ausgezahlte Zinsen pro Jahr"),
                                     qsl("Anzahl auslaufender Verträge nach Jahr"),
                                     qsl("Anzahl Verträge nach Zinssatz und Jahr"),
                                     qsl("Anzahl Verträge nach Laufzeiten"),
                                     qsl("Verlauf fortlaufender Geldanlagen (Vertragswert)"),
                                     qsl("Verlauf fortlaufender Geldanlagen (Buchungen)")}));
    } else {
        updateUebersichtView(combo->currentIndex());
    }

    ui->stackedWidget->setCurrentIndex(overviewsPageIndex);
}
void MainWindow::updateUebersichtView(int uebersichtIndex)
{
    QTextDocument* td =new QTextDocument(); // ui->txtOverview->document();
    uebersichten ue(td);
    ue.renderDocument(uebersichten::fromInt(uebersichtIndex));
    ui->txtOverview->setDocument(td);
}

void MainWindow::on_comboUebersicht_currentIndexChanged(int i)
{   LOG_CALL_W(i2s(i));
    if(i == -1)
        return;
    busyCursor b;
    updateUebersichtView(i);
}
void MainWindow::on_pbPrint_clicked()
{   LOG_CALL;

    QString filename {QDate::currentDate().toString(qsl("yyyy-MM-dd_"))};
    filename += Statistics_Filenames[ui->comboUebersicht->currentIndex()] +qsl(".pdf");

    QPdfWriter write(appendFilenameToOutputDir(filename));
    ui->txtOverview->print(&write);
    showInExplorer(filename);
}

  ////////////////////////////////////////////////
 //             Verlaeufe                      //
////////////////////////////////////////////////
QVector<BookingDateData> dates;

QString descriptionFromType(const QString &bddType)
{
    if( bddType == qsl("VD") || bddType == qsl("VDex"))
        return qsl("Vertragsabschluß");
    if( bddType == qsl("AD") || bddType == qsl("ADex"))
        return qsl("Vertragsaktivierung");
    if( bddType == qsl("CT"))
        return qsl("Vertragsbeendigung");
    if( bddType == qsl("1"))
        return qsl("Einzahlung");
    if( bddType == qsl("2"))
        return qsl("Auszahlung");
    if( bddType == qsl("4"))
        return qsl("ZwischenZins für Ein- oder Auszahlung");
    if( bddType == qsl("8"))
        return qsl("Jahreszinsanrechnung");
    return (qsl("unknown activity type ") +bddType);

}
QString bookingDateDesc( const BookingDateData &bdd)
{
    QString date =bdd.date.toString(qsl("  dd.MM.yyyy  "));
    if( bdd.count == 1) {
        QString line {qsl("%1  <small>(%2)</small>")};
        return line.arg(date, descriptionFromType(bdd.type));
    }
    else {
        QString line {qsl("%1  <small>(%2, und %3 weitere)  </small>")};
        return line.arg(date, descriptionFromType(bdd.type),
                        i2s(bdd.count-1));
    }
}

qsizetype currentDateIndex =0;

void MainWindow::on_rbActive_toggled(bool checked)
{
    if( checked) prepare_statisticsPage();
}
void MainWindow::on_rbInactive_toggled(bool checked)
{
    if( checked) prepare_statisticsPage();
}
void MainWindow::on_rbFinished_toggled(bool checked)
{
    if( checked) prepare_statisticsPage();
}
void MainWindow::on_rbAll_toggled(bool checked)
{
    if( checked) prepare_statisticsPage();
}
void MainWindow::on_pbBack_clicked()
{
    // back increases the index
    const qsizetype maxIndex =dates.size() -1l;
    currentDateIndex = qMin(currentDateIndex +1l, maxIndex);
    ui->lblBookingDate->setText(bookingDateDesc(dates[currentDateIndex]));
    ui->pbBack->setEnabled(currentDateIndex < maxIndex);
    ui->pbNext->setEnabled(true);
    fillStatisticsTableView();
}
void MainWindow::on_pbNext_clicked()
{
    // next decreases the index
    currentDateIndex = qMax(currentDateIndex -1, 0);
    ui->lblBookingDate->setText(bookingDateDesc(dates[currentDateIndex]));
    ui->pbBack->setEnabled(true);
    ui->pbNext->setEnabled(currentDateIndex not_eq 0);
    fillStatisticsTableView();
}
void MainWindow::on_pbLetzter_clicked()
{
    currentDateIndex =0;
    on_pbNext_clicked();
    prepare_statisticsPage();
}

void MainWindow::getDatesFromContractStates()
{
    dates.clear();
    currentDateIndex =0;
    if( ui->rbActive->isChecked()){
        qInfo() << "init dates for active contracts";
        getActiveContracsBookingDates( dates);
    }
    else if (ui->rbInactive->isChecked()){
        qInfo() << "init dates for INactive contracts";
        getInactiveContractBookingDates( dates);
    }
    else if( ui->rbFinished->isChecked()) {
        qInfo() << "init dates for finished contracts";
        getFinishedContractBookingDates( dates);
    }
    else if (ui->rbAll->isChecked()){
        qInfo() << "init dates for ALL contracts";
        getAllContractBookingDates( dates);
    }
    else
        Q_ASSERT(not "never come here");
}
void MainWindow::fillStatisticsTableView()
{   LOG_CALL_W(dates[currentDateIndex].date.toString(Qt::ISODate));
    QString sql;
    if( ui->rbActive->isChecked()) {
        sql =sqlStat_activeContracts_byIMode_toDate;
    } else if (ui->rbInactive->isChecked()){
        sql =sqlStat_inactiveContracts_byIMode_toDate;
    } else if (ui->rbFinished->isChecked()) {
        sql =sqlStat_finishedContracts_toDate;
    }    else if (ui->rbAll->isChecked()) {
        sql =sqlStat_allContracts_byIMode_toDate;
    } else
        Q_ASSERT (not "never reach this point");
    sql.replace(qsl(":date"), dates[currentDateIndex].date.toString(Qt::ISODate)).replace(qsl("\n"), qsl(" "));

    QSqlQueryModel *mod =new QSqlQueryModel();
    mod->setQuery(sql);
    if( mod->lastError().type() not_eq QSqlError::NoError) {
        qInfo() << "SqlError: " << mod->lastError();
        qInfo().nospace() << mod->query().lastQuery();
        return;
    }
    mod->setHeaderData(0, Qt::Horizontal, qsl("Zinsmodus"));
    mod->setHeaderData(1, Qt::Horizontal, qsl("Anzahl\nVerträge"));
    mod->setHeaderData(2, Qt::Horizontal, qsl("Anzahl\nKreditor*innen"));
    mod->setHeaderData(3, Qt::Horizontal, qsl("Kreditvolumen"));
    mod->setHeaderData(4, Qt::Horizontal, qsl("jährl. Zinsen"));
    mod->setHeaderData(5, Qt::Horizontal, qsl("durchschn.\nZins"));

    ui->tvData->setModel(mod);
}
void MainWindow::prepare_statisticsPage()
{
    getDatesFromContractStates();
    if( dates.isEmpty()) {
        ui->tvData->reset(); ui->tvData->setModel(new QSqlTableModel());
        ui->pbNext->setEnabled(false);
        ui->pbBack->setEnabled(false);
        ui->pbLetzter->setEnabled(false);
        ui->lblBookingDate->setText(qsl("&nbr;-   <i>  Keine Daten  </i>   -&nbr;"));
        return;
    }

    ui->lblBookingDate->setText(bookingDateDesc(dates[currentDateIndex]));
    ui->pbNext->setEnabled(false);
    ui->pbBack->setEnabled(dates.size()>1);
    ui->pbLetzter->setEnabled(true);
    fillStatisticsTableView();
    ui->tvData->resizeColumnsToContents();
    ui->tvData->setItemDelegateForColumn(0/*iMode*/, new interestModeFormatter);
    ui->tvData->setItemDelegateForColumn(1/*mbrContr.*/, new centralAlignedTextFormatter);
    ui->tvData->setItemDelegateForColumn(2/*mbrcredi.*/, new centralAlignedTextFormatter);
    ui->tvData->setItemDelegateForColumn(3/*Volume*/, new CurrencyFormatter);
    ui->tvData->setItemDelegateForColumn(4/*annualCost*/, new CurrencyFormatter);
    ui->tvData->setItemDelegateForColumn(5/*avgInterest*/, new PercentItemFormatter);

}
void MainWindow::on_actionStatistik_triggered()
{
    prepare_statisticsPage();
    ui->stackedWidget->setCurrentIndex(statisticsPageIndex);
}

/////////////////////////////////////////////////
// annual settlement
/////////////////////////////////////////////////
void MainWindow::on_action_menu_contracts_annual_interest_settlement_triggered()
{   LOG_CALL;
    annualSettlement();
    updateViews();
}

void MainWindow::on_action_menu_contracts_interestLetters_triggered()
{
    LOG_CALL;
    annualSettlementLetters();
    updateViews();
}

/////////////////////////////////////////////////
// list creation csv, printouts
/////////////////////////////////////////////////

namespace{
QString contractListFilename() {
    QString projectName {dbConfig::readValue(projectConfiguration::GMBH_PROJECT).toString()};
    projectName =makeSafeFileName(projectName, 9);

    return qsl("%1_AktiveVerträge_%2.csv")
        .arg(QDate::currentDate().toString(Qt::ISODate), projectName);
}
}// EO namespace

void MainWindow::on_action_menu_contracts_print_lists_triggered()
{   LOG_CALL;
    QString csv =createCsvActiveContracts();
    if( csv.isEmpty()) {
        QMessageBox::critical(this, qsl("Fehler"), qsl("Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?"));
        return;
    }

    QString path {saveStringToUtf8File(contractListFilename(), csv)};
    if( path.isEmpty()) {
        QMessageBox::critical(this, qsl("Fehler"), qsl("Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?"));
        return;
    }
    else {
        showInExplorer(path);
    }
}

/////////////////////////////////////////////////
// debug funktions
/////////////////////////////////////////////////
void MainWindow::on_action_menu_debug_create_sample_data_triggered()
{   LOG_CALL;
    busyCursor b;
    create_sampleData();
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex());
    on_action_menu_contracts_listview_triggered();
}
void MainWindow::on_action_menu_debug_show_log_triggered()
{   LOG_CALL;
#if defined(Q_OS_WIN)
    ::ShellExecuteA(nullptr, "open", logFilePath().toUtf8(), "", QDir::currentPath().toUtf8(), 1);
#else
    #if defined(Q_OS_MAC)
    QString cmd = QStringLiteral("open -e ") + logFilePath();
    #else
    QString cmd = QStringLiteral("xdg-open ") + logFilePath();
    #endif
    if (system(cmd.toUtf8().constData())) {
        QString msg = qsl("Ich weiß nicht wie %1 geöffnet werden kann.\n" \
        "Benutze bitte einen Text-Editor wie gedit, kate oder ähnlich.").arg(logFilePath());
        QMessageBox::information(this, qsl("I n f o"), msg);
    }

#endif
}

void MainWindow::on_actionDatenbank_Views_schreiben_triggered()
{
    insertDKDB_Views ();
    insertDKDB_Indices ();
}
// about
void MainWindow::on_action_about_DKV2_triggered()
{   LOG_CALL;

    dlgAbout aboutDlg (this);
    aboutDlg.setHeader (qsl("<p>DKV2 Version: %1<br>"
                           "Betriebssystem: %2<br>"
                           "Prozess: %3<p>").arg(QApplication::applicationVersion()
                                , QSysInfo::prettyProductName()
                                , QSysInfo::buildAbi()));

    QString msg;
    msg = qsl("<b>Lieber Anwender!</b> <p>DKV2 wird von seinen Entwicklern kostenlos zur Verfügung gestellt. ");
    msg += qsl("Es wurde mit viel Arbeit und Sorgfalt entwickelt. Wenn Du es nützlich findest: Viel Spaß bei der Anwendung!!<br>");
    msg += qsl("Allerdings darfst Du es nicht verkaufen oder bezahlte Dienste wie etwa für Einrichtung, Schulung oder Unterstützung anbieten oder leisten.<br>");
    msg += qsl("Trotz aller Sorgfalt und Mühe: Wie jede Software enthält DKV2 sicher Fehler. Wenn Du sie uns mitteilst werden sie vielleicht ausgebessert.<br>");
    msg += qsl("Aber aus der Verwendung kannst Du keine Rechte ableiten. ");
    msg += qsl("Wenn Fehler und Probleme auftreten, so übernehmen wir weder Haftung für Folgen noch Verantwortung für Schäden - dafür hast Du sicher Verständnis.<br>");
    msg += qsl("<br><b>Viel Spaß mit DKV2 !</b>");
    aboutDlg.setMsg (msg);

    aboutDlg.exec();


}
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(qsl("geometry"), saveGeometry());
    settings.setValue(qsl("windowState"), saveState());

#ifndef QT_DEBUG
    if (QMessageBox::Yes not_eq QMessageBox::question(this, qsl("Beenden?"), qsl("Möchtest Du DKV2 beenden?")))
        event->ignore();
#else
    Q_UNUSED(event);
#endif
}

void MainWindow::on_actionSchriftgr_e_anpassen_triggered()
{
    bool OK =true;
    double current =appconfig::Zoom ();
    double input =QInputDialog::getDouble(this, qsl("Schriftgröße anpassen"),
                                           qsl("Skalierungsfaktor zwischen 0,8 und 1,5"),
                                           current, 0.8, 1.5, 1, &OK,
                                           Qt::WindowFlags(), 0.1);
    if( OK and (current not_eq input)) {
        appconfig::setZoom (input);
        QMessageBox::information (this, qsl("Neustart"), qsl("Damit eine neue Schriftgröße in allen Fenstern angewendet wird, muss das Programm eventuell neu gestartet werden."));
        QFont f =QApplication::font();
        f.setPointSizeF(appconfig::getSystemFontsize () *appconfig::Zoom());
        QApplication::setFont(f);
        setFont(f);
        ui->stackedWidget->setFont (f);
        ui->stackedWidget->currentWidget ()->setFont (f);
    }
}

