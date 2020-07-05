#include <windows.h>

#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QFileDialog>
#include <QRandomGenerator>

#include "helper.h"
#include "appconfig.h"
#include "dkdbhelper.h"
#include "wiznewdatabase.h"

/*
 * general page to select a file name
 * NOT used for newDB
*/
wizFileSelection_IntroPage::wizFileSelection_IntroPage(QWidget* p) : QWizardPage(p) {
    QLineEdit* le = new QLineEdit;
    registerField("selectedFile", le);
    QLabel* l =new QLabel("Klicke auf 'durchsuchen' um eine Dateiauswahlfenster zu öffnen.");
    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

void wizFileSelection_IntroPage::initializePage()
{
    wizFileSelectionWiz* wiz = dynamic_cast<wizFileSelectionWiz*>(wizard());
    if( wiz) {
        setTitle(wiz->title);
        setSubTitle(wiz->subtitle);
    } else {
        wizNewDatabaseWiz* nDbWiz = dynamic_cast<wizNewDatabaseWiz*>(wizard());
        setTitle(nDbWiz->title);
        setSubTitle(nDbWiz->subtitle);
    }
}

void wizFileSelection_IntroPage::browseButtonClicked() {
    wizFileSelectionWiz* wiz = dynamic_cast<wizFileSelectionWiz*>(wizard());

    QString selectedFile =( wiz->existingFile) ? QFileDialog::getOpenFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr)
            : QFileDialog::getSaveFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr);
    if(selectedFile.isEmpty())
        return;
    setField( "selectedFile", selectedFile);
}

bool wizFileSelection_IntroPage::validatePage() {
    wizFileSelectionWiz* wiz = dynamic_cast<wizFileSelectionWiz*>(wizard());
    if( wiz->existingFile) {
        return QFile::exists(field("selectedFile").toString());
    }
    else
        return ! field("selectedFile").toString().isEmpty();
}

void wizFileSelection_IntroPage::setVisible(bool v) {
    QWizardPage::setVisible(v);
    wizard()->setOption(QWizard::HaveCustomButton1, v);
    if( v) {
        wizard()->setButtonText(QWizard::CustomButton1, "durch&suchen");
        connect(wizard(), &QWizard::customButtonClicked,
                this, &wizFileSelection_IntroPage::browseButtonClicked);
    } else {
        disconnect(wizard(), &QWizard::customButtonClicked,
                this, &wizFileSelection_IntroPage::browseButtonClicked);
    }
}

/*
 * minimal wizard to select a file
 * for copyDb or openDb
*/
wizFileSelectionWiz::wizFileSelectionWiz(QWidget* p) : QWizard(p) {
    addPage(new wizFileSelection_IntroPage);
}

/*
 * page to select a file name used for newDB
*/
wizFileSelectionNewDb_IntroPage::wizFileSelectionNewDb_IntroPage(QWidget* p) : QWizardPage(p) {
    QLineEdit* le = new QLineEdit;
    registerField("selectedFile", le);
    QLabel* l =new QLabel("Klicke auf 'durchsuchen' um eine Dateiauswahl Fenster zu öffnen.");
    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

void wizFileSelectionNewDb_IntroPage::initializePage()
{
    wizNewDatabaseWiz* wiz = dynamic_cast<wizNewDatabaseWiz*>(wizard());
    setTitle(wiz->title);
    setSubTitle(wiz->subtitle);
}

void wizFileSelectionNewDb_IntroPage::browseButtonClicked() {
    wizNewDatabaseWiz* wiz = dynamic_cast<wizNewDatabaseWiz*>(wizard());

    QString selectedFile =( wiz->existingFile) ? QFileDialog::getOpenFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr)
            : QFileDialog::getSaveFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr);
    if(selectedFile.isEmpty())
        return;
    setField( "selectedFile", selectedFile);
}

bool wizFileSelectionNewDb_IntroPage::validatePage() {
    wizNewDatabaseWiz* wiz = dynamic_cast<wizNewDatabaseWiz*>(wizard());
    if( wiz->existingFile) {
        return QFile::exists(field("selectedFile").toString());
    }
    else
        return ! field("selectedFile").toString().isEmpty();
}

void wizFileSelectionNewDb_IntroPage::setVisible(bool v) {
    QWizardPage::setVisible(v);
    wizard()->setOption(QWizard::HaveCustomButton1, v);
    if( v) {
        wizard()->setButtonText(QWizard::CustomButton1, "durch&suchen");
        connect(wizard(), &QWizard::customButtonClicked,
                this, &wizFileSelectionNewDb_IntroPage::browseButtonClicked);
    } else {
        disconnect(wizard(), &QWizard::customButtonClicked,
                this, &wizFileSelectionNewDb_IntroPage::browseButtonClicked);
    }
}

/*
 * page to enter GmbH address data
*/
wizProjectAddress_Page::wizProjectAddress_Page(QWidget* p) : QWizardPage(p)
{
    setTitle("Adresse der Projekt GmbH");
    //QLabel* lDisclaimer = new QLabel();
    setSubTitle("*<small>Diese Daten werden für Briefdruck benötigt und können auch später eingegeben und geändert werden.</small>");
    QLineEdit* leAddress1 = new QLineEdit;
    registerField(GMBH_ADDRESS1, leAddress1);
    QLineEdit* leAddress2 = new QLineEdit;
    registerField(GMBH_ADDRESS2, leAddress2);
    QLineEdit* leStrasse =  new QLineEdit;
    registerField(GMBH_STREET, leStrasse);
    QLineEdit* lePlz =      new QLineEdit;
    registerField(GMBH_PLZ, lePlz);
    QLineEdit* leStadt =    new QLineEdit;
    registerField(GMBH_CITY, leStadt);
    QLineEdit* leEmail =    new QLineEdit;
    registerField(GMBH_EMAIL, leEmail);
    QLineEdit* leUrl   =    new QLineEdit;
    registerField(GMBH_URL, leUrl);

    QGridLayout* grid  = new QGridLayout;
    grid->addWidget(leAddress1, 0, 0, 1, 3);
    grid->addWidget(leAddress2, 1, 0, 1, 3);
    grid->addWidget(leStrasse,  2, 0, 1, 3);
    grid->addWidget(lePlz,      3, 0);
    grid->addWidget(leStadt,    3, 1, 1, 2);
    grid->addWidget(leEmail,    4, 0, 1, 3);
    grid->addWidget(leUrl,      5, 0, 1, 3);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 4);
    // grid->setHorizontalSpacing(0);
    setLayout(grid);
}

void wizProjectAddress_Page::initializePage()
{
    dbConfig c(dbConfig::FROM_RTD);
    setField(GMBH_ADDRESS1,  c.address1);
    setField(GMBH_ADDRESS2,  c.address2);
    setField(GMBH_STREET,    c.street);
    setField(GMBH_PLZ,       c.plz);
    setField(GMBH_CITY,      c.city);
    setField(GMBH_EMAIL,     c.email);
    setField(GMBH_URL,       c.url);
}

wizProjectDetails_Page::wizProjectDetails_Page(QWidget* p) : QWizardPage(p)
{
    setTitle("Weitere Daten der Projekt GmbH");
    setSubTitle("*<small>Diese Daten werden für Briefdruck benötigt und können auch später eingegeben und geändert werden.</small>");
    QLabel* lHre = new QLabel ("Eintrag im Handeslregister");
    QLineEdit* leHre = new QLineEdit;
    registerField(GMBH_HRE, leHre);
    QLabel* lGefue = new QLabel ("Geschäftsführer*innen");
    QLineEdit* leGefue1 = new QLineEdit;
    registerField(GMBH_GEFUE1, leGefue1);
    QLineEdit* leGefue2 = new QLineEdit;
    registerField(GMBH_GEFUE2, leGefue2);
    QLineEdit* leGefue3 = new QLineEdit;
    registerField(GMBH_GEFUE3, leGefue3);
    QLabel* lDkv =new QLabel("DK Verwaltung");
    QLineEdit* leDkv = new QLineEdit;
    registerField(GMBH_DKV, leDkv);

    QGridLayout* grid  = new QGridLayout;
    grid->addWidget(lHre,     0, 0, 1, 2);
    grid->addWidget(leHre,    0, 2, 1, 3);
    grid->addWidget(lGefue,   1, 0, 1, 2);
    grid->addWidget(leGefue1, 1, 2, 1, 3);
    grid->addWidget(leGefue2, 2, 2, 1, 3);
    grid->addWidget(leGefue3, 3, 2, 1, 3);
    grid->addWidget(lDkv,     4, 0, 1, 2);
    grid->addWidget(leDkv,    4, 2, 1, 3);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(3, 3);
    setLayout(grid);
}

void wizProjectDetails_Page::initializePage()
{
    dbConfig c(dbConfig::FROM_RTD);
    setField(GMBH_HRE,    c.hre);
    setField(GMBH_GEFUE1, c.gefue1);
    setField(GMBH_GEFUE2, c.gefue2);
    setField(GMBH_GEFUE3, c.gefue3);
    setField(GMBH_DKV,    c.dkv);
}

wizContractLableInfo_Page::wizContractLableInfo_Page(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertragskennung");
    setSubTitle("Diese Informationen werden verwendet um eindeutige Kennzeichen für die einzelnen Verträge zu erzeugen");

    QLabel* lProject= new QLabel("Projekt Kürzel (2-5 Zeichen):");
    QLineEdit* leProject = new QLineEdit;
    lProject->setBuddy(leProject);
    registerField(GMBH_PI, leProject);

    QLabel* lIndex = new QLabel("Start Index:");
    leStartIndex = new QLineEdit;
    lIndex->setBuddy(leStartIndex);
    registerField(STARTINDEX, leStartIndex);

    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(lProject);
    layout->addWidget(leProject);
    layout->addWidget(lIndex);
    layout->addWidget(leStartIndex);
    setLayout(layout);
}

void wizContractLableInfo_Page::initializePage()
{
    QRandomGenerator rand(::GetTickCount());
    dbConfig c(dbConfig::FROM_RTD);
    setField(GMBH_PI, c.pi);
    int startindex = rand.bounded(1000, 9999);
    setField(STARTINDEX, QString::number(startindex));
}

bool wizContractLableInfo_Page::validatePage()
{
    int startindex = field(STARTINDEX).toInt();
    setField(STARTINDEX, QString::number(startindex));
    QString project = field(GMBH_PI).toString();
    if(project.length()> 5)
        setField(GMBH_PI, project.left(5));
    return true;
}

wizContractMinValues_Page:: wizContractMinValues_Page(QWidget* p) : QWizardPage(p)
{
    setTitle("Weitere Configuration");
    setSubTitle("Hier kannst Du den minimalen Auszahlungsbetrag und Vertragswert festlegen, für den das Programm eine Buchung erlaubt. "
                "Dieser Werte werden bei Auszahlungen und beim Anlegen von Verträgen berücksichtigt.<p>"
                "<small>Da Auszahlungen z.T. mit Überweisungskosten einhergehen und kleine Verträge unrentabel sind sollte man kleine Werte vermeiden.</small>");
    QLineEdit* leMa =new QLineEdit;
    registerField(MIN_PAYOUT, leMa);
    QLabel* lma     =new QLabel("Kleinster Auszahlungsbetrag in Euro:");
    lma->setBuddy(leMa);

    QLineEdit* leMc =new QLineEdit;
    registerField(MIN_AMOUNT, leMc);
    QLabel* lmc     =new QLabel("Kleinster Vertragswert in Euro:");
    lmc->setBuddy(leMc);

    QGridLayout* grid =new QGridLayout;
    grid->addWidget(lma,  0, 0);
    grid->addWidget(leMa, 0, 1);
    grid->addWidget(lmc,  1, 0);
    grid->addWidget(leMc, 1, 1);
    setLayout(grid);
}

void wizContractMinValues_Page::initializePage()
{
    dbConfig c(dbConfig::FROM_RTD);
    setField(MIN_PAYOUT, c.minPayout);
    setField(MIN_AMOUNT, c.minContract);
}

wizNewDatabase_SummaryPage::wizNewDatabase_SummaryPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Zusammenfassung");
    QCheckBox* cb = new QCheckBox("Die Eingaben sind korrekt!");
    registerField("confirmed", cb);
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
}
void wizNewDatabase_SummaryPage::initializePage()
{
    QString subt = "<b>Projekt Daten:</b>"
                   "<table><tr><td>Adresse:</td><td>%1</td></tr>"
                   "<tr><td></td><td>%2</td></tr>"
                   "<tr><td></td><td>%3</td></tr>"
                   "<tr><td></td><td>%4 %5<br></td></tr>";
    subt = subt.arg(field(GMBH_ADDRESS1).toString());
    subt = subt.arg(field(GMBH_ADDRESS2).toString());
    subt = subt.arg(field(GMBH_STREET).toString());
    subt = subt.arg(field(GMBH_PLZ).toString());
    subt = subt.arg(field(GMBH_CITY).toString());

    subt += "<tr><td>E-Mail:</td><td>%1</td></tr>"
            "<tr><td>Web:</td><td>%2</td></tr>"
            "<tr><td>Kürzel:    </td><td>%3</td></tr>"
            "<tr><td>Start Index:&nbsp;&nbsp;</td><td>%4</td></tr></table>";
    subt = subt.arg(field(GMBH_EMAIL).toString()).arg(field(GMBH_URL).toString());
    subt = subt.arg(field(GMBH_PI).toString());
    subt = subt.arg(field(STARTINDEX).toString());
    setSubTitle(subt);
}
bool wizNewDatabase_SummaryPage::validatePage()
{
    return field("confirmed").toBool();
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
wizNewDatabaseWiz::wizNewDatabaseWiz(QWidget* p) : QWizard(p) {
    addPage(new wizFileSelectionNewDb_IntroPage);
    addPage(new wizProjectAddress_Page);
    addPage(new wizProjectDetails_Page);
    addPage(new wizContractMinValues_Page);
    addPage(new wizContractLableInfo_Page);
    addPage(new wizNewDatabase_SummaryPage);
}
void wizNewDatabaseWiz::updateDbConfig()
{   LOG_CALL;
    dbConfig c;
    c.address1 =field(GMBH_ADDRESS1).toString();
    c.address2 =field(GMBH_ADDRESS2).toString();
    c.street   =field(GMBH_STREET).toString();
    c.plz      =field(GMBH_PLZ).toString();
    c.city     =field(GMBH_CITY).toString();
    c.email    =field(GMBH_EMAIL).toString();
    c.url      =field(GMBH_URL).toString();
    c.pi       =field(GMBH_PI).toString();
    c.startindex=field(STARTINDEX).toDouble();
    c.dbId     =c.pi +QString::number(c.startindex);
    c.hre      =field(GMBH_HRE).toString();
    c.gefue1   =field(GMBH_GEFUE1).toString();
    c.gefue2   =field(GMBH_GEFUE2).toString();
    c.gefue3   =field(GMBH_GEFUE3).toString();
    c.dkv      =field(GMBH_DKV).toString();
    c.storeRuntimeData();
}

wizConfigure_IntroPage::wizConfigure_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle("Konfiguration");
    setSubTitle("Mit dieser Dialogfolge kannst du Konfigurationen "
                "für Briefdruck und die Datenbank vornehmen.");
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
wizConfigureProjectWiz::wizConfigureProjectWiz(QWidget* p) : QWizard(p) {
    addPage(new wizConfigure_IntroPage);
    addPage(new wizProjectAddress_Page);
    addPage(new wizProjectDetails_Page);
    addPage(new wizContractMinValues_Page);
}
void wizConfigureProjectWiz::updateDbConfig()
{   LOG_CALL;
    dbConfig c(dbConfig::FROM_RTD);
    c.address1=field(GMBH_ADDRESS1).toString();
    c.address2=field(GMBH_ADDRESS2).toString();
    c.street  =field(GMBH_STREET).toString();
    c.plz     =field(GMBH_PLZ).toString();
    c.city    =field(GMBH_CITY).toString();
    c.email   =field(GMBH_EMAIL).toString();
    c.url     =field(GMBH_URL).toString();

    c.hre     =field(GMBH_HRE).toString();
    c.gefue1  =field(GMBH_GEFUE1).toString();
    c.gefue2  =field(GMBH_GEFUE2).toString();
    c.gefue3  =field(GMBH_GEFUE3).toString();
    c.dkv     =field(GMBH_DKV).toString();

    c.minPayout=field(MIN_PAYOUT).toInt();
    c.minContract=field(MIN_AMOUNT).toInt();
    c.storeRuntimeData();
    c.writeDb();
}

