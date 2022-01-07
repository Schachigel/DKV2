#include <QtGlobal>

#if defined(Q_OS_WIN)
#include "windows.h"
#else
#include <stdlib.h>
#endif

// the qt header for preCompiled Header feature
#include "pch.h"

#include "busycursor.h"

#include "appconfig.h"
#include "investment.h"
#include "wiznewdatabase.h"
#include "wizopenornewdatabase.h"
#include "dlgaskdate.h"
#include "uiitemformatter.h"
#include "dkdbcopy.h"
#include "contracttablemodel.h"
#include "transaktionen.h"
#include "uebersichten.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"


// generell functions (used for contruction)
bool getValidDatabaseFromCommandline(QString& path)
{
    QString dbPath =getDbFileFromCommandline();
    if( dbPath.isEmpty()){
        qInfo() << "no file given on the commandline";
        return false;
    }
    qInfo() << "Path from CmdLine " << dbPath;
    QFileInfo fi{dbPath};
    dbPath = fi.canonicalFilePath();
    if( checkSchema_ConvertIfneeded(dbPath)) {
        path =dbPath;
    } else {
        // execution will not reach here: conversation error will lead to a soft crash
        // do not continue if a db was given on the cmd line but is not valid
    }
    // db was given on the commandline -> no alternate file search
    return true;
}

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

QString MainWindow::findValidDatabaseToUse()
{   LOG_CALL;
    // a db from the command line would be stored as currentDb...
    QString dbPath;
    if( getValidDatabaseFromCommandline( dbPath))
        return dbPath;
    // NO db given on the commandline - use LastDb if available
    dbPath =appConfig::LastDb();
    if(QFile(dbPath).exists()) {
        if(checkSchema_ConvertIfneeded(dbPath)) {
            qInfo() << "last db will be reopened " << dbPath;
            return dbPath;
        }
    }
    // last used db is not valid -> ask for another one
    qInfo() << dbPath << " aus der Konfiguration ist keine valide Datenbank";
    dbPath =askUserForNextDb();
    // // //
    if(checkSchema_ConvertIfneeded(dbPath)){
        // a valid database was given -> all good
        return dbPath;
    } else {
        // there should be a valid DB
        QMessageBox::critical(this, qsl("Ganz schlecht"), qsl("Ohne valide Datenbank kann Dkv2 nicht laufen"));
        return QString();
    }
}

QString MainWindow::askUserForNextDb()
{   LOG_CALL;
    wizOpenOrNewDb wizOpenOrNew (getMainWindow());
    if( QDialog::Accepted not_eq wizOpenOrNew.exec()) {
        qInfo() << "wizard OpenOrNew was canceled by the user";
        return QString();
    }
    QString selectedDbPath {absoluteCanonicalPath(wizOpenOrNew.selectedFile)};
    { // busycursor scope
        busycursor b;
        if( not wizOpenOrNew.field(qsl("createNewDb")).toBool()) {
            // the UI does not allow an empty string here
            qInfo() << "existing db " << selectedDbPath << "was selected";
            return selectedDbPath;
        }
        // a new db should be created -> ask project details
        // closeAllDatabaseConnections();
        if( not createNewDatabaseFileWDefaultContent(selectedDbPath, (wizOpenOrNew.field(qsl("Zinssusance")).toBool() ? zs30360 : zs_actact))) {
            QMessageBox::critical(this, qsl("Fehler"), qsl("Die neue Datenbank konnte nicht angelegt werden. Die Ausführung wird abgebrochen"));
            return QString();
        }
    }// busycursor livetime
    dbCloser closer{qsl("conWriteConfig")};
    wizConfigureNewDatabaseWiz wizProjectData(this);
    if( wizProjectData.Accepted == wizProjectData.exec()) {
        wizProjectData.updateDbConfig(selectedDbPath);
    }
    return selectedDbPath;
}

void treat_DbIsAlreadyInUse_File(QString filename)
{
    QMessageBox::StandardButton result = QMessageBox::NoButton;

    while (checkSignalFile(filename))
    {
        result = QMessageBox::information((QWidget *)nullptr, qsl("Datenbank bereits geöffnet?"),
                                 qsl("Es scheint, als sei die Datenbank bereits geöffnet. Das kommt vor, "
                                     "wenn DKV2 abgestürzt ist oder bereits läuft.<p>"
                                     "Falls die Datenbank auf einem Fileserver läuft, kann auch eine "
                                     "andere Benutzerin die Datenbank gerade verwenden."
                                     "<p>Retry: Wenn das andere Programm beendet ist."
                                     "<p>Cancel: Um dieses Programm zu beenden."
                                     "<p>Ignore: Wenn du sicher bist, dass kein anderes "
                                     "Programm läuft. (auf eigene Gefahr!)"),
                                 QMessageBox::Cancel | QMessageBox::Retry | QMessageBox::Ignore);

        if (result == QMessageBox::Cancel)
            exit(1);

        if (result == QMessageBox::Ignore)
            break;

        /* QMessageBox::Retry repeats the file check */
    }
    return createSignalFile (filename);
}


bool MainWindow::useDb(const QString& dbfile)
{   LOG_CALL;
    if( open_databaseForApplication(dbfile)) {
        add_MRU_entry (dbfile);
        treat_DbIsAlreadyInUse_File (dbfile);
        appConfig::setLastDb(dbfile);
        showDbInStatusbar(dbfile);
        return true;
    }
    qCritical() << "the databse could not be used for this application";
    return false;
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

    ui->statusBar->addPermanentWidget(ui->statusLabel);
    setCentralWidget(ui->stackedWidget);

    QString dbPath =findValidDatabaseToUse();
    if( dbPath.isEmpty()) {
        return;
    }

    // if we come here, dbPath contains a valid Databse, lets use it
    if( not useDb(dbPath)) {
        QMessageBox::critical(nullptr, qsl("FEHLER"), qsl("Die angegebene Datenbank kann nicht verwendet werden. DKV2 wird beendet"));
        return;
    }
    // WE ARE READY TO GO
    dbLoadedSuccessfully =true;

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

    ui->fontComboBox->setEditable(false);
    ui->fontComboBox->setWritingSystem(QFontDatabase::Latin);
    ui->fontComboBox->setFontFilters(QFontComboBox::ScalableFonts | QFontComboBox::ProportionalFonts);
    ui->spinFontSize->setMinimum(8);
    ui->spinFontSize->setMaximum(11);
    connect(ui->wPreview, &QPrintPreviewWidget::paintRequested, this, &MainWindow::doPaint);

    QSettings settings;
    restoreGeometry(settings.value(qsl("geometry")).toByteArray());
    restoreState(settings.value(qsl("windowState")).toByteArray());

    prepare_startPage();
    ui->stackedWidget->setCurrentIndex(startPageIndex);
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
        ContractProxyModel *contractsTableProxy_ptr = qobject_cast<ContractProxyModel *>(ui->contractsTableView->model());
        QSqlTableModel *bookingsTableModel_ptr = qobject_cast<QSqlTableModel *>(ui->bookingsTableView->model());
        ContractTableModel *contractsTableModel_ptr = nullptr;
        if (contractsTableProxy_ptr)
            contractsTableModel_ptr = qobject_cast<ContractTableModel *>(contractsTableProxy_ptr->sourceModel());
        if (contractsTableModel_ptr)
            contractsTableModel_ptr->select();
        if (bookingsTableModel_ptr)
            bookingsTableModel_ptr->select();
        ui->contractsTableView->resizeColumnsToContents();
        ui->contractsTableView->resizeRowsToContents();
        ui->bookingsTableView->resizeColumnsToContents();
        if (contractsTableModel_ptr)
            contractsTableModel_ptr->setCol13ExtraData();
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
    busycursor b;
    QString messageHtml {qsl("<table width='100%'><tr></tr><tr><td width='20%'></td><td width='60%'><center><h2>Willkommen zu DKV2- Deiner Verwaltung von Direktkrediten</h2></center></td><td width='20%'></td></tr>")};

    double allContractsValue =valueOfAllContracts();

    QString pName =dbConfig::readValue(projectConfiguration::GMBH_ADDRESS1).toString();
    if( pName.size()) {
        messageHtml += qsl("<tr></tr><tr><td></td><td><b>DK Verwaltung für <font color=blue>%1</font></td><td></td></tr>").arg(pName);
    }
    if( allContractsValue > 0) {
        QLocale l;
        QString valueRow = qsl("<tr><td></td><td>Die Summer aller Direktkredite und Zinsen beträgt <big><font color=red>")
                + l.toCurrencyString(allContractsValue) + qsl("</font></big></td><td></td></tr>");
        messageHtml += valueRow;
    }
    messageHtml += qsl("<tr><td></td><td>[%1]</td><td></td></tr>").arg(ui->statusLabel->text ());
    messageHtml += qsl("</table>");
    qDebug() <<"welcome Screen html: " << Qt::endl << messageHtml << Qt::endl;
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

    QFileInfo lastdb (appConfig::LastDb());
    if( lastdb.exists())
        wiz.openInFolder=lastdb.path();
    else
        wiz.openInFolder =QStandardPaths::writableLocation((QStandardPaths::DocumentsLocation));

    wiz.exec();
    return wiz.field(qsl("selectedFile")).toString();
}

void MainWindow::open_Database(const QString& dbFile)
{
    busycursor bc;
    if( appConfig::LastDb () == dbFile)
    {
        bc.finish();
        QMessageBox::information( this, qsl("Abbruch"), qsl("Die ausgewählte Datei ist bereits geöffnet."));
        return;
    }
    if( not checkSchema_ConvertIfneeded(dbFile)) {
        // selected file is not valid or can not be converted
        // do nothing
        bc.finish();
        QMessageBox::information( this, qsl("Abbruch"), qsl("Die ausgewählte Datei ist keine gültige Datenbank."));
        return;
    }
    if( useDb(dbFile)) {
        prepare_startPage();
        ui->stackedWidget->setCurrentIndex(startPageIndex);
        return;
    } else {
        appConfig::delLastDb();
        bc.finish();
        QMessageBox::critical(this, qsl("Großes Problem"), qsl("Die gewählte Datenbank konnte nicht verwendet werden. "
                                       "Suche in der dkv2.log Datei im %temp% Verzeichnis nach der Ursache!"));
        // useDb has closed the openDb -> without old and new db we could not run
        close();
    }
}

void MainWindow::on_action_menu_database_new_triggered()
{   LOG_CALL;
    QString dbFile =askUserForNextDb();
    if( dbFile.isEmpty()) {
        // askUserNextDb was not successful - maybe canceled.
        // do nothing
        QMessageBox::information( this, qsl("Abbruch"), qsl("Die Dateiauswahl wurde abgebrochen."));
        return;
    }
    return open_Database (dbFile);
}

void MainWindow::on_action_menu_database_copy_triggered()
{   LOG_CALL;
    QString dbfile = askUserFilenameForCopy(qsl("Dateiname der Kopie Datenbank angeben."));
    if( dbfile == qsl(""))
        return;

    busycursor b;
    if( copy_dkdb_database(QSqlDatabase::database().databaseName(), dbfile)){
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
    if( dbfile == qsl(""))
        return;
    busycursor b;
    if( not copy_database_mangled(dbfile)) {
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
    appConfig::setOutDirInteractive(this);
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
    model->setHeaderData(4, Qt::Horizontal, qsl("Anzahl\n(alle)"), Qt::DisplayRole);
    tv->setItemDelegateForColumn(5, new CurrencyFormatter);
    model->setHeaderData(5, Qt::Horizontal, qsl("Summe\n(alle)"), Qt::DisplayRole);
    model->setHeaderData(6, Qt::Horizontal, qsl("Anzahl\n(aktive)"), Qt::DisplayRole);
    tv->setItemDelegateForColumn(7, new CurrencyFormatter);
    model->setHeaderData(7, Qt::Horizontal, qsl("Summe\n(aktive)"), Qt::DisplayRole);
    tv->hideColumn(9);
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
    int newInvestments =createNewInvestmentsFromContracts();
    if( newInvestments == -1) {
        QMessageBox::critical(this, qsl("Fehler"), qsl("Beim Anlegen der Geldanlagen ist ein Fehler aufgetreten"));
        return;
    }
    if( newInvestments) {
        int i =automatchInvestmentsToContracts();
        if( i not_eq newInvestments)
            qCritical() << qsl("nicht alle Verträgen (%1) konnten Anlagen (%2) zugeordnet werden").arg(QString::number(i), QString::number(newInvestments));

        QMessageBox::information(this, qsl("Neue Anlageformen"), qsl("Es wurden ") +QString::number(newInvestments) +qsl(" Anlage(n) angelegt."));
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
    busycursor b;
    int i =automatchInvestmentsToContracts();
    b.finish ();
    QMessageBox::information(this, qsl("Zugeordnete Verträge"),
                             qsl("Es wurden %1 Verträge passenden Geldanlagen zugeordnet.").arg(QString::number(i)));
    b.set();
    prepare_investmentsListView();
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
    QString msg{qsl("Soll die Anlage mit <b>%1%</b>, vom %2 zum %3 mit dem Typ <br>  > %4 <  <br> gelöscht werden?")};
    msg =msg.arg(QString::number(zinssatz/100., 'f', 2), anfang, ende, typ);

    if( QMessageBox::Yes == QMessageBox::question(this, qsl("Löschen"), msg)) {
        // delete the entry, update the view
        if( deleteInvestment(zinssatz, dAnfang, dEnde, typ)) {
            qInfo() << "removed investment row " << index.row();
            //tm->submitAll();
            tm->select();
        } else {
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

    if( QMessageBox::Yes == QMessageBox::question(this, qsl("Status ändern"), msg)) {
        bool result = currentStatus ? closeInvestment(zinssatz, dAnfang, dEnde, typ) : openInvestment(zinssatz, dAnfang, dEnde, typ);
        if( result) {
            qInfo() << "Investment status from investment " << index.row() << " was changed to " << (currentStatus ? "closed" : "open");
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
    QString zinssatz =qobject_cast<PercentFrom100sItemFormatter*>(ui->InvestmentsTableView->itemDelegate(index.siblingAtColumn(0)))
            ->displayText(model->data(index.siblingAtColumn(0)), QLocale());

    QString von =doFormatDateItem(model->data(index.siblingAtColumn(1)));
    QString bis =doFormatDateItem(model->data(index.siblingAtColumn(2)));

    QString msg {qsl("<table><tr><th>Neue Bezeichnung für den Anlage </th></tr><tr><td style=\"align:center\">mit %1 Zins</td></tr>"
                     "<tr><td>von %2 bis %3.</tr>"
                     "<tr><td>alter Wert: <i>'%4'</i></td></tr></table>")};

    QInputDialog id(this);
    QFont f =id.font(); f.setPointSize(10); id.setFont(f);
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
    QString sql(qsl("UPDATE Geldanlagen SET Typ =? WHERE rowid =%1").arg(rec.value(qsl("rowid")).toString()));
    if( executeSql_wNoRecords(sql, {QVariant(txt)}))
        tm->select();
}
void MainWindow::on_action_cmenu_Vertraege_anzeigen_triggered()
{
    QModelIndex index =ui->actionInvestmentLoeschen->data().toModelIndex();
    QSqlTableModel* tm =qobject_cast<QSqlTableModel*>(ui->InvestmentsTableView->model());
    QSqlRecord rec =tm->record(index.row());
    qlonglong id =rec.value(qsl("rowid")).toLongLong ();
    ui->le_ContractsFilter->setText (qsl("Anlage:%1").arg(id));
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
                                     qsl("Fortlaufende Geldanlagen")}));
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
{   LOG_CALL_W(QString::number(i));
    if(i == -1)
        return;
    busycursor b;
    updateUebersichtView(i);
}
void MainWindow::on_pbPrint_clicked()
{   LOG_CALL;
    QString filename = appConfig::Outdir();
    filename += qsl("/") + QDate::currentDate().toString(qsl("yyyy-MM-dd_"));
    filename += Statistics_Filenames[ui->comboUebersicht->currentIndex()];
    filename += qsl(".pdf");
    QPdfWriter write(filename);
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
                        QString::number(bdd.count-1));
    }
}

int currentDateIndex =0;

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
    const int maxIndex =dates.size() -1;
    currentDateIndex = qMin(currentDateIndex +1, maxIndex);
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
    qDebug() << sql;

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
    interestLetters();
    updateViews();
}

/////////////////////////////////////////////////
// list creation csv, printouts
/////////////////////////////////////////////////
void MainWindow::on_action_menu_contracts_print_lists_triggered()
{   LOG_CALL;
    if( not createCsvActiveContracts())
        QMessageBox::critical(this, qsl("Fehler"), qsl("Die Datei konnte nicht angelegt werden. Ist sie z.B. in Excel geöffnet?"));
}

/////////////////////////////////////////////////
// debug funktions
/////////////////////////////////////////////////
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
    QString cmd = QStringLiteral("xdg-open ") + logFilePath();
    if (system(cmd.toUtf8().constData())) {
        QString msg = qsl("Ich weiß nicht wie %1 geöffnet werden kann.\n" \
        "Benutze bitte einen Text-Editor wie gedit, kate oder ähnlich.").arg(logFilePath());
        QMessageBox::information(this, qsl("I n f o"), msg);
    }

#endif
}
void MainWindow::on_actionDatenbank_Views_schreiben_triggered()
{
    insertDKDB_Views(QSqlDatabase::database());
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
void MainWindow::on_actionTEST_triggered()
{
    LOG_CALL;
//    // input nec. to display the dialog: a Vector of bookings
//    toBePrinted.clear();
//    toBePrinted = bookings::getAnnualSettelments(2019);
//    if ( not toBePrinted.size()) {
//        qWarning() << "nothing to be printed";
//        return;
//    }
//    currentBooking = toBePrinted.begin();

//    prepare_printPreview();
//    ui->stackedWidget->setCurrentIndex(printPreviewPageIndex);
}
/////////////////////////////////////////////////
//              PRINTING wprev.                //
/////////////////////////////////////////////////
QString letterName(const booking &b)
{
    QString txt = qsl("<table width=100%><tr><td align=center style='padding-top:5px;padding-bottom:5px;'>%1, %2; %3<br><b>%4</b></td></tr></table>");
    contract cont(b.contractId);
    creditor cred(cont.creditorId());
    QString lettertype = booking::displayString(b.type) +qsl(" ");
    lettertype += (b.type == booking::Type::annualInterestDeposit) ? QString::number(b.date.year() - 1) : b.date.toString();

    txt = txt.arg(cred.lastname(), cred.firstname(), cont.label(), lettertype);
    return txt;
}

void MainWindow::prepare_printPreview()
{
    LOG_CALL;
    ui->btnPrevBooking->setEnabled(currentBooking not_eq toBePrinted.cbegin());
    ui->btnNextBooking->setEnabled((currentBooking +1) not_eq toBePrinted.cend());

    ui->lblLetter->setText (letterName(*currentBooking));

    QFont f(qsl("Verdana"));
    ui->fontComboBox->setCurrentFont(f);
    ui->spinFontSize->setValue(10);
}

void MainWindow::on_btnNextBooking_clicked()
{
    LOG_CALL;
    if ((currentBooking+1) == toBePrinted.cend())
        return;
    else
        currentBooking = currentBooking +1;
    prepare_printPreview();
}

void MainWindow::on_btnPrevBooking_clicked()
{
    LOG_CALL;
    if (currentBooking == toBePrinted.cbegin())
        return;
    else
        currentBooking = currentBooking -1;
    prepare_printPreview();
}

void MainWindow::on_btnUpdatePreview_clicked()
{
    ui->wPreview->updatePreview();
}

void MainWindow::doPaint(QPrinter* pri)
{
    QPainter p(dynamic_cast<QPaintDevice*>(pri));
    p.drawText(QPoint(100, 100), qsl("Hallo World"));

    // Logo
    // Adresse
    // Datum
    // Anrede
    // Fußzeile
    // text(e)
}

void MainWindow::closeEvent(QCloseEvent *event)
{

#ifndef QT_DEBUG
    QMessageBox::StandardButton mr = QMessageBox::question(this, qsl("Beenden?"), qsl("Möchtest Du DKV2 beenden?"));
    if (mr == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
#endif

    QSettings settings;
    settings.setValue(qsl("geometry"), saveGeometry());
    settings.setValue(qsl("windowState"), saveState());
    event->accept();

}

