
#include <QStringLiteral>
#include <QLineEdit>
#include <QIntValidator>
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
wpFileSelection_IntroPage::wpFileSelection_IntroPage(QWidget* p) : QWizardPage(p) {
    QLineEdit* le = new QLineEdit;
    le->setToolTip(qsl("Hier kannst du einen vollständigen Pfad zu einer Datei angeben"));
    registerField(qsl("selectedFile"), le);
    QLabel* l =new QLabel(qsl("Klicke auf 'durchsuchen' um eine Dateiauswahlfenster zu öffnen."));
    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

void wpFileSelection_IntroPage::initializePage()
{
    wizFileSelectionWiz* wiz = qobject_cast<wizFileSelectionWiz*>(wizard());
    if( wiz) {
        setTitle(wiz->title);
        setSubTitle(wiz->subtitle);
    }
}

void wpFileSelection_IntroPage::browseButtonClicked() {
    wizFileSelectionWiz* wiz = qobject_cast<wizFileSelectionWiz*>(wizard());

    QString selectedFile =( wiz->existingFile) ? QFileDialog::getOpenFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr)
            : QFileDialog::getSaveFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr);
    if(selectedFile.isEmpty())
        return;
    setField(qsl("selectedFile"), selectedFile);
}

bool wpFileSelection_IntroPage::validatePage() {
    wizFileSelectionWiz* wiz = qobject_cast<wizFileSelectionWiz*>(wizard());
    if( wiz->existingFile)
        return QFile::exists(field(qsl("selectedFile")).toString());
    else
        return ! field(qsl("selectedFile")).toString().isEmpty();
}

void wpFileSelection_IntroPage::setVisible(bool v) {
    QWizardPage::setVisible(v);
    wizard()->setOption(QWizard::HaveCustomButton1, v);
    if( v) {
        wizard()->setButtonText(QWizard::CustomButton1, qsl("durch&suchen"));
        connect(wizard(), &QWizard::customButtonClicked,
                this, &wpFileSelection_IntroPage::browseButtonClicked);
    } else {
        disconnect(wizard(), &QWizard::customButtonClicked,
                this, &wpFileSelection_IntroPage::browseButtonClicked);
    }
}

/*
 * minimal wizard to select a file
 * for copyDb or openDb
*/
wizFileSelectionWiz::wizFileSelectionWiz(QWidget* p) : QWizard(p) {
    addPage(new wpFileSelection_IntroPage);
    QFont f = font(); f.setPointSize(10); setFont(f);
}

/*
 * page to select a file name used for newDB
*/
wpFileSelectionNewDb_IntroPage::wpFileSelectionNewDb_IntroPage(QWidget* p) : QWizardPage(p) {
}

void wpFileSelectionNewDb_IntroPage::initializePage()
{
    setTitle(qsl("DB Konfiguration"));
    setSubTitle(qsl("Mit den folgenden Dialogen kannst Du die Datenbank "
    "für Euer Projekt konfigurieren"));
}

/*
 * page to enter GmbH address data
*/
wpProjectAddress_Page::wpProjectAddress_Page(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Adresse der Projekt GmbH"));
    //QLabel* lDisclaimer = new QLabel();
    setSubTitle(qsl("*<small>Diese Daten werden für Briefdruck benötigt und können auch später eingegeben und geändert werden.</small>"));
    QLineEdit* leAddress1 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_ADDRESS1), leAddress1);
    leAddress1->setToolTip(qsl("Gib hier die erste Adresszeile der GmbH ein"));
    QLineEdit* leAddress2 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_ADDRESS2), leAddress2);
    leAddress2->setToolTip(qsl("Gib hier die zweite Adresszeile der GmbH ein"));
    QLineEdit* leStrasse =  new QLineEdit;
    registerField(dbConfig::paramName(GMBH_STREET), leStrasse);
    leStrasse->setToolTip(qsl("Gib hier die Straße der GmbH ein."));
    QLineEdit* lePlz =      new QLineEdit;
    registerField(dbConfig::paramName(GMBH_PLZ), lePlz);
    lePlz->setToolTip(qsl("Gib hier die Postleitzahl der GmbH ein"));
    QLineEdit* leStadt =    new QLineEdit;
    registerField(dbConfig::paramName(GMBH_CITY), leStadt);
    leStadt->setToolTip(qsl("Gib hier die Stadt ein, in der die GmbH ist."));
    QLineEdit* leEmail =    new QLineEdit;
    registerField(dbConfig::paramName(GMBH_EMAIL), leEmail);
    leEmail->setToolTip(qsl("Gib hier die E-Mailadresse an, unter der die GmbH erreichbar ist."));
    QLineEdit* leUrl   =    new QLineEdit;
    registerField(dbConfig::paramName(GMBH_URL), leUrl);

    QGridLayout* grid      =new QGridLayout;
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

void wpProjectAddress_Page::initializePage()
{
    setField(dbConfig::paramName(GMBH_ADDRESS1),  dbConfig::readValue(GMBH_ADDRESS1));
    setField(dbConfig::paramName(GMBH_ADDRESS2),  dbConfig::readValue(GMBH_ADDRESS2));
    setField(dbConfig::paramName(GMBH_STREET),    dbConfig::readValue(GMBH_STREET));
    setField(dbConfig::paramName(GMBH_PLZ),       dbConfig::readValue(GMBH_PLZ));
    setField(dbConfig::paramName(GMBH_CITY),      dbConfig::readValue(GMBH_CITY));
    setField(dbConfig::paramName(GMBH_EMAIL),     dbConfig::readValue(GMBH_EMAIL));
    setField(dbConfig::paramName(GMBH_URL),       dbConfig::readValue(GMBH_URL));
}

wpProjectDetails_Page::wpProjectDetails_Page(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Weitere Daten der Projekt GmbH"));
    setSubTitle(qsl("*<small>Diese Daten werden für Briefdruck benötigt und können auch später eingegeben und geändert werden.</small>"));
    QLabel* lHre = new QLabel (qsl("Eintrag im Handeslregister"));
    QLineEdit* leHre = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_HRE), leHre);
    leHre->setToolTip(qsl("Gib hier an, wie der Handelsregistereintrag der GmbH lautet."));
    QLabel* lGefue = new QLabel (qsl("Geschäftsführer*innen"));
    QLineEdit* leGefue1 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_GEFUE1), leGefue1);
    leGefue1->setToolTip(qsl("Gib hier den Namen eines Geschäftsführers oder einer Geschäftsführerin ein."));
    QLineEdit* leGefue2 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_GEFUE2), leGefue2);
    leGefue2->setToolTip(qsl("Gib hier den Namen eines Geschäftsführers oder einer Geschäftsführerin ein."));
    QLineEdit* leGefue3 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_GEFUE3), leGefue3);
    leGefue3->setToolTip(qsl("Gib hier den Namen eines Geschäftsführers oder einer Geschäftsführerin ein."));
    QLabel* lDkv =new QLabel(qsl("DK Verwaltung"));
    QLineEdit* leDkv = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_DKV), leDkv);
    leDkv->setToolTip(qsl("Gib hier ein, wer bei Euch die Briefe der DK Verwaltung erstellt."));

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

void wpProjectDetails_Page::initializePage()
{
    setField(dbConfig::paramName(GMBH_HRE),    dbConfig::readValue(GMBH_HRE));
    setField(dbConfig::paramName(GMBH_GEFUE1), dbConfig::readValue(GMBH_GEFUE1));
    setField(dbConfig::paramName(GMBH_GEFUE2), dbConfig::readValue(GMBH_GEFUE2));
    setField(dbConfig::paramName(GMBH_GEFUE3), dbConfig::readValue(GMBH_GEFUE3));
    setField(dbConfig::paramName(GMBH_DKV),    dbConfig::readValue(GMBH_DKV));
}

wpContractLableInfo_Page::wpContractLableInfo_Page(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertragskennung"));
    setSubTitle(qsl("Diese Informationen werden verwendet um eindeutige Kennzeichen für die einzelnen Verträge zu erzeugen"));

    QLabel* lProject= new QLabel(qsl("Projekt Kürzel (2-5 Zeichen):"));
    QLineEdit* leProject = new QLineEdit;
    lProject->setBuddy(leProject);
    registerField(dbConfig::paramName(GMBH_INITIALS), leProject);
    leProject->setToolTip(qsl("Das Kürzel wird bei der Erstellung von Vertragskennzeichen verwendet"));

    QLabel* lIndex = new QLabel(qsl("Start Index:"));
    leStartIndex = new QLineEdit;
    lIndex->setBuddy(leStartIndex);
    registerField(dbConfig::paramName(STARTINDEX), leStartIndex);
    lIndex->setToolTip(qsl("Mit diesem Index beginnt die laufende Nummer in den Vertragskennzeichen"));

    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(lProject);
    layout->addWidget(leProject);
    layout->addWidget(lIndex);
    layout->addWidget(leStartIndex);
    setLayout(layout);
}

void wpContractLableInfo_Page::initializePage()
{
    QRandomGenerator rand = *QRandomGenerator::system();
    setField(dbConfig::paramName(GMBH_INITIALS), dbConfig::readValue(GMBH_INITIALS));
    int startindex = rand.bounded(1000, 9999);
    setField(dbConfig::paramName(STARTINDEX), QString::number(startindex));
}

bool wpContractLableInfo_Page::validatePage()
{
    int startindex = field(dbConfig::paramName(STARTINDEX)).toInt();
    setField(dbConfig::paramName(STARTINDEX), QString::number(startindex));
    QString project = field(dbConfig::paramName(GMBH_INITIALS)).toString();
    if(project.length()> 5)
        setField(dbConfig::paramName(GMBH_INITIALS), project.left(5));
    return true;
}

wpContractMinValues_Page:: wpContractMinValues_Page(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Weitere Konfiguration"));
    setSubTitle(qsl("Hier kannst Du den minimalen Auszahlungsbetrag und Vertragswert festlegen, für den das Programm eine Buchung erlaubt. "
                "Dieser Werte werden bei Auszahlungen und beim Anlegen von Verträgen berücksichtigt.<p>"
                "<small>Da Auszahlungen z.T. mit Überweisungskosten einhergehen und kleine Verträge unrentabel sind sollte man kleine Werte vermeiden.</small>"));
    QLineEdit* leMa =new QLineEdit;
    registerField(dbConfig::paramName(MIN_PAYOUT), leMa);
    leMa->setValidator(new QIntValidator(this));
    QLabel* lma     =new QLabel(qsl("Kleinster Auszahlungsbetrag in Euro:"));
    lma->setBuddy(leMa);

    QLineEdit* leMc =new QLineEdit;
    registerField(dbConfig::paramName(MIN_AMOUNT), leMc);
    leMc->setValidator(new QIntValidator(this));
    QLabel* lmc     =new QLabel(qsl("Kleinster Vertragswert in Euro:"));
    lmc->setBuddy(leMc);

    QLineEdit* leMi =new QLineEdit;
    registerField(dbConfig::paramName(MAX_INTEREST), leMi);
    leMi->setValidator(new QIntValidator(this));
    QLabel* lmi     =new QLabel(qsl("Größter auswählbarer Zins in 100tel (100 entspricht 1%)"));
    lmi->setBuddy(leMi);

    QGridLayout* grid =new QGridLayout;
    grid->addWidget(lma,  0, 0);
    grid->addWidget(leMa, 0, 1);
    grid->addWidget(lmc,  1, 0);
    grid->addWidget(leMc, 1, 1);
    grid->addWidget(lmi,  2, 0);
    grid->addWidget(leMi, 2, 1);
    setLayout(grid);
}

void wpContractMinValues_Page::initializePage()
{
    setField(dbConfig::paramName(MIN_PAYOUT), dbConfig::readValue(MIN_PAYOUT));
    setField(dbConfig::paramName(MIN_AMOUNT), dbConfig::readValue(MIN_AMOUNT));
    setField(dbConfig::paramName(MAX_INTEREST), dbConfig::readValue(MAX_INTEREST));
}

wpNewDatabase_SummaryPage::wpNewDatabase_SummaryPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    QCheckBox* cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    cb->setToolTip(qsl("Mit diesem Ankreuzfeld bestätigst Du, dass die Angaben richtig sind."));
    QVBoxLayout* layout = new QVBoxLayout;
    layout-> addWidget(cb);
    setLayout(layout);
    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(onConfirmData_toggled(int)));
}
void wpNewDatabase_SummaryPage::initializePage()
{
    QString subt = qsl("<b>Projekt Daten:</b>"
                   "<table><tr><td>Adresse:</td><td>%1</td></tr>"
                   "<tr><td></td><td>%2</td></tr>"
                   "<tr><td></td><td>%3</td></tr>"
                   "<tr><td></td><td>%4 %5<br></td></tr>");
    subt = subt.arg(field(dbConfig::paramName(GMBH_ADDRESS1)).toString());
    subt = subt.arg(field(dbConfig::paramName(GMBH_ADDRESS2)).toString());
    subt = subt.arg(field(dbConfig::paramName(GMBH_STREET)).toString());
    subt = subt.arg(field(dbConfig::paramName(GMBH_PLZ)).toString());
    subt = subt.arg(field(dbConfig::paramName(GMBH_CITY)).toString());

    subt += qsl("<tr><td>E-Mail:</td><td>%1</td></tr>"
            "<tr><td>Web:</td><td>%2</td></tr>"
            "<tr><td>Kürzel:    </td><td>%3</td></tr>"
            "<tr><td>Start Index:&nbsp;&nbsp;</td><td>%4</td></tr></table>");
    subt = subt.arg(field(dbConfig::paramName(GMBH_EMAIL)).toString(), field(dbConfig::paramName(GMBH_URL)).toString());
    subt = subt.arg(field(dbConfig::paramName(GMBH_INITIALS)).toString());
    subt = subt.arg(field(dbConfig::paramName(STARTINDEX)).toString());
    setSubTitle(subt);
}
void wpNewDatabase_SummaryPage::onConfirmData_toggled(int)
{
    completeChanged();
}
bool wpNewDatabase_SummaryPage::isComplete() const
{
    return field("confirmed").toBool();
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
wizConfigureNewDatabaseWiz::wizConfigureNewDatabaseWiz(QWidget* p) : QWizard(p) {
    addPage(new wpFileSelectionNewDb_IntroPage);
    addPage(new wpProjectAddress_Page);
    addPage(new wpProjectDetails_Page);
    addPage(new wpContractMinValues_Page);
    addPage(new wpContractLableInfo_Page);
    addPage(new wpNewDatabase_SummaryPage);
    QFont f = font(); f.setPointSize(10); setFont(f);
}
void wizConfigureNewDatabaseWiz::updateDbConfig(QSqlDatabase db)
{   LOG_CALL;
    dbConfig::writeValue(GMBH_ADDRESS1,  field(dbConfig::paramName(GMBH_ADDRESS1)), db);
    dbConfig::writeValue(GMBH_ADDRESS2,  field(dbConfig::paramName(GMBH_ADDRESS2)), db);
    dbConfig::writeValue(GMBH_STREET,    field(dbConfig::paramName(GMBH_STREET  )), db);
    dbConfig::writeValue(GMBH_PLZ ,      field(dbConfig::paramName(GMBH_PLZ)), db);
    dbConfig::writeValue(GMBH_CITY ,     field(dbConfig::paramName(GMBH_CITY)), db);
    dbConfig::writeValue(GMBH_EMAIL ,    field(dbConfig::paramName(GMBH_EMAIL)), db);
    dbConfig::writeValue(GMBH_URL ,      field(dbConfig::paramName(GMBH_URL)), db);
    dbConfig::writeValue(GMBH_INITIALS , field(dbConfig::paramName(GMBH_INITIALS)), db);
    dbConfig::writeValue(STARTINDEX ,    field(dbConfig::paramName(STARTINDEX)), db);
    dbConfig::writeValue(DBID,  QVariant(dbConfig::readValue(GMBH_INITIALS).toString() +dbConfig::readValue(STARTINDEX).toString()), db);
    dbConfig::writeValue(GMBH_HRE,     field(dbConfig::paramName(GMBH_HRE)), db);
    dbConfig::writeValue(GMBH_GEFUE1,  field(dbConfig::paramName(GMBH_GEFUE1)), db);
    dbConfig::writeValue(GMBH_GEFUE2,  field(dbConfig::paramName(GMBH_GEFUE2)), db);
    dbConfig::writeValue(GMBH_GEFUE3,  field(dbConfig::paramName(GMBH_GEFUE3)), db);
    dbConfig::writeValue(GMBH_DKV,     field(dbConfig::paramName(GMBH_DKV)), db);
    dbConfig::writeValue(MIN_PAYOUT,   field(dbConfig::paramName(MIN_PAYOUT)), db);
    dbConfig::writeValue(MIN_AMOUNT,   field(dbConfig::paramName(MIN_AMOUNT)), db);
    dbConfig::writeValue(MAX_INTEREST, field(dbConfig::paramName(MAX_INTEREST)), db);
}

wpConfigure_IntroPage::wpConfigure_IntroPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Konfiguration"));
    setSubTitle(qsl("Mit dieser Dialogfolge kannst du Konfigurationen "
                "für Briefdruck und die Datenbank vornehmen."));
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
wizConfigureProjectWiz::wizConfigureProjectWiz(QWidget* p) : QWizard(p) {
    addPage(new wpConfigure_IntroPage);
    addPage(new wpProjectAddress_Page);
    addPage(new wpProjectDetails_Page);
    addPage(new wpContractMinValues_Page);
    QFont f =font(); f.setPointSize(10); setFont(f);
}
void wizConfigureProjectWiz::updateDbConfig()
{   LOG_CALL;
    dbConfig::writeValue(GMBH_ADDRESS1,  field(dbConfig::paramName(GMBH_ADDRESS1)));
    dbConfig::writeValue(GMBH_ADDRESS2,  field(dbConfig::paramName(GMBH_ADDRESS2)));
    dbConfig::writeValue(GMBH_STREET,    field(dbConfig::paramName(GMBH_STREET  )));
    dbConfig::writeValue(GMBH_PLZ ,      field(dbConfig::paramName(GMBH_PLZ)));
    dbConfig::writeValue(GMBH_CITY ,     field(dbConfig::paramName(GMBH_CITY)));
    dbConfig::writeValue(GMBH_EMAIL ,    field(dbConfig::paramName(GMBH_EMAIL)));
    dbConfig::writeValue(GMBH_URL ,      field(dbConfig::paramName(GMBH_URL)));

    dbConfig::writeValue(GMBH_HRE,     field(dbConfig::paramName(GMBH_HRE)));
    dbConfig::writeValue(GMBH_GEFUE1,  field(dbConfig::paramName(GMBH_GEFUE1)));
    dbConfig::writeValue(GMBH_GEFUE2,  field(dbConfig::paramName(GMBH_GEFUE2)));
    dbConfig::writeValue(GMBH_GEFUE3,  field(dbConfig::paramName(GMBH_GEFUE3)));
    dbConfig::writeValue(GMBH_DKV,     field(dbConfig::paramName(GMBH_DKV)));
    dbConfig::writeValue(MIN_PAYOUT,   field(dbConfig::paramName(MIN_PAYOUT)));
    dbConfig::writeValue(MIN_AMOUNT,   field(dbConfig::paramName(MIN_AMOUNT)));
    dbConfig::writeValue(MAX_INTEREST, field(dbConfig::paramName(MAX_INTEREST)));
}

