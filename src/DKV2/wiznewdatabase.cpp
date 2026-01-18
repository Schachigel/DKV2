#include "wiznewdatabase.h"



#include "helper.h"
#include "helperfin.h"
#include "helpersql.h"
#include "appconfig.h"

/*
 * general page to select a file name
 * NOT used for newDB
*/
wpFileSelection_IntroPage::wpFileSelection_IntroPage(QWidget *p) : QWizardPage(p)
{
    QLineEdit *le = new QLineEdit;
    le->setToolTip(qsl("Hier kannst du einen vollständigen Pfad zu einer Datei angeben"));
    registerField(qsl("selectedFile"), le);
    QLabel *l = new QLabel(qsl("Klicke auf 'durchsuchen' um ein Dateiauswahlfenster zu öffnen."));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

void wpFileSelection_IntroPage::initializePage()
{
    wizFileSelectionWiz *wiz = qobject_cast<wizFileSelectionWiz *>(wizard());
    if (wiz)
    {
        setTitle(wiz->title);
        subTitleLabel->setText(wiz->subtitle);
    }
}

void wpFileSelection_IntroPage::browseButtonClicked()
{
    wizFileSelectionWiz *wiz = qobject_cast<wizFileSelectionWiz *>(wizard());

    QString selectedFile = (wiz->existingFile) ? QFileDialog::getOpenFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr)
                                               : QFileDialog::getSaveFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr);
    if (selectedFile.isEmpty())
        return;
    setField(qsl("selectedFile"), selectedFile);
}

bool wpFileSelection_IntroPage::validatePage()
{
    wizFileSelectionWiz *wiz = qobject_cast<wizFileSelectionWiz *>(wizard());
    QString file = field(qsl("selectedFile")).toString();
    QFileInfo fi(file);
    if (fi.suffix().isEmpty())
        file += qsl(".dkdb");
    if (fi.path() == qsl("."))
        file = wiz->openInFolder + qsl("/") + file;
    setField(qsl("selectedFile"), file);
    if (wiz->existingFile)
    {

        return QFile::exists(file);
    }
    else
        return not field(qsl("selectedFile")).toString().isEmpty();
}

void wpFileSelection_IntroPage::setVisible(bool v)
{
    QWizardPage::setVisible(v);
    wizard()->setOption(QWizard::HaveCustomButton1, v);
    if (v)
    {
        wizard()->setButtonText(QWizard::CustomButton1, qsl("durch&suchen"));
        connect(wizard(), &QWizard::customButtonClicked,
                this, &wpFileSelection_IntroPage::browseButtonClicked);
    }
    else
    {
        disconnect(wizard(), &QWizard::customButtonClicked,
                   this, &wpFileSelection_IntroPage::browseButtonClicked);
    }
}

/*
 * minimal wizard to select a file
 * for copyDb or openDb
*/
wizFileSelectionWiz::wizFileSelectionWiz(QWidget *p) : QWizard(p)
{   LOG_CALL;
    setWizardStyle(QWizard::ModernStyle);
    addPage(new wpFileSelection_IntroPage);
}

/*
 * page to select a file name used for newDB
*/
wpFileSelectionNewDb_IntroPage::wpFileSelectionNewDb_IntroPage(QWidget *p) : QWizardPage(p)
{
    subTitleLabel->setWordWrap(true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    setLayout(layout);
}

void wpFileSelectionNewDb_IntroPage::initializePage()
{
    setTitle(qsl("DB Konfiguration"));
    subTitleLabel->setText(qsl("Mit den folgenden Dialogen kannst Du die Datenbank "
                    "für Euer Projekt konfigurieren"));
}

/*
 * page to enter GmbH address data
*/
wpProjectAddress_Page::wpProjectAddress_Page(QWidget *p) : QWizardPage(p)
{
    setTitle(qsl("Adresse der Projekt GmbH"));

    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
    subTitleLabel->setWordWrap(true);
    subTitleLabel->setText(qsl("*<small>Diese Daten werden für Briefdruck benötigt und können auch später eingegeben und geändert werden.</small>"));

    QLineEdit *leProjekt =new QLineEdit();
    registerField (dbConfig::paramName(GMBH_PROJECT), leProjekt);
    leProjekt->setToolTip (qsl("Name des Wohnprojektes"));
    QLineEdit *leAddress1 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_ADDRESS1), leAddress1);
    leAddress1->setToolTip(qsl("Gib hier die erste Adresszeile der GmbH ein"));
    QLineEdit *leAddress2 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_ADDRESS2), leAddress2);
    leAddress2->setToolTip(qsl("Gib hier die zweite Adresszeile der GmbH ein"));
    QLineEdit *leStrasse = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_STREET), leStrasse);
    leStrasse->setToolTip(qsl("Gib hier die Straße der GmbH ein."));
    QLineEdit *lePlz = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_PLZ), lePlz);
    lePlz->setToolTip(qsl("Gib hier die Postleitzahl der GmbH ein"));
    QLineEdit *leStadt = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_CITY), leStadt);
    leStadt->setToolTip(qsl("Gib hier die Stadt ein, in der die GmbH ist."));
    QLineEdit *leEmail = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_EMAIL), leEmail);
    leEmail->setToolTip(qsl("Gib hier die E-Mailadresse an, unter der die GmbH erreichbar ist."));
    QLineEdit *leUrl = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_URL), leUrl);

    QVBoxLayout* vb =new QVBoxLayout();
    vb->addWidget(subTitleLabel);

    QGridLayout *grid = new QGridLayout;
    int row = 0;
    grid->addWidget (leProjekt, row, 0, 1, 3);
    row++;
    grid->addWidget(leAddress1, row, 0, 1, 3);
    row++;
    grid->addWidget(leAddress2, row, 0, 1, 3);
    row++;
    grid->addWidget(leStrasse, row, 0, 1, 3);
    row++;
    grid->addWidget(lePlz, row, 0);
    grid->addWidget(leStadt, row, 1, 1, 2);
    row++;
    grid->addWidget(leEmail, row, 0, 1, 3);
    row++;
    grid->addWidget(leUrl, row, 0, 1, 3);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 4);

    vb->addLayout (grid);
    setLayout(vb);
}

void wpProjectAddress_Page::initializePage()
{
    setField(dbConfig::paramName(GMBH_PROJECT), dbConfig::readValue (GMBH_PROJECT));
    setField(dbConfig::paramName(GMBH_ADDRESS1), dbConfig::readValue(GMBH_ADDRESS1));
    setField(dbConfig::paramName(GMBH_ADDRESS2), dbConfig::readValue(GMBH_ADDRESS2));
    setField(dbConfig::paramName(GMBH_STREET), dbConfig::readValue(GMBH_STREET));
    setField(dbConfig::paramName(GMBH_PLZ), dbConfig::readValue(GMBH_PLZ));
    setField(dbConfig::paramName(GMBH_CITY), dbConfig::readValue(GMBH_CITY));
    setField(dbConfig::paramName(GMBH_EMAIL), dbConfig::readValue(GMBH_EMAIL));
    setField(dbConfig::paramName(GMBH_URL), dbConfig::readValue(GMBH_URL));
}

wpProjectDetails_Page::wpProjectDetails_Page(QWidget *p) : QWizardPage(p)
{
    setTitle(qsl("Weitere Daten der Projekt GmbH"));
    subTitleLabel->setWordWrap(true);
    subTitleLabel->setText(qsl("*<small>Diese Daten werden für Briefdruck benötigt und können auch später eingegeben und geändert werden.</small>"));
    QLabel *lHre = new QLabel(qsl("Eintrag im Handeslregister"));
    QLineEdit *leHre = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_HRE), leHre);
    leHre->setToolTip(qsl("Gib hier an, wie der Handelsregistereintrag der GmbH lautet."));
    QLabel *lGefue = new QLabel(qsl("Geschäftsführer*innen"));
    QLineEdit *leGefue1 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_GEFUE1), leGefue1);
    leGefue1->setToolTip(qsl("Gib hier den Namen eines Geschäftsführers oder einer Geschäftsführerin ein."));
    QLineEdit *leGefue2 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_GEFUE2), leGefue2);
    leGefue2->setToolTip(qsl("Gib hier den Namen eines Geschäftsführers oder einer Geschäftsführerin ein."));
    QLineEdit *leGefue3 = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_GEFUE3), leGefue3);
    leGefue3->setToolTip(qsl("Gib hier den Namen eines Geschäftsführers oder einer Geschäftsführerin ein."));
    QLabel *lDkv = new QLabel(qsl("DK Verwaltung"));
    QLineEdit *leDkv = new QLineEdit;
    registerField(dbConfig::paramName(GMBH_DKV), leDkv);
    leDkv->setToolTip(qsl("Gib hier ein, wer bei Euch die Briefe der DK Verwaltung erstellt."));

    QGridLayout *grid = new QGridLayout;
    int row = 0;
    grid->addWidget(subTitleLabel, row, 0, 1, 4);
    row++;
    grid->addWidget(lHre, row, 0, 1, 2);
    grid->addWidget(leHre, row, 2, 1, 3);
    row++;
    grid->addWidget(lGefue, row, 0, 1, 2);
    grid->addWidget(leGefue1, row, 2, 1, 3);
    row++;
    grid->addWidget(leGefue2, row, 2, 1, 3);
    row++;
    grid->addWidget(leGefue3, row, 2, 1, 3);
    row++;
    grid->addWidget(lDkv, row, 0, 1, 2);
    grid->addWidget(leDkv, row, 2, 1, 3);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(3, 3);
    setLayout(grid);
}

void wpProjectDetails_Page::initializePage()
{
    setField(dbConfig::paramName(GMBH_HRE), dbConfig::readValue(GMBH_HRE));
    setField(dbConfig::paramName(GMBH_GEFUE1), dbConfig::readValue(GMBH_GEFUE1));
    setField(dbConfig::paramName(GMBH_GEFUE2), dbConfig::readValue(GMBH_GEFUE2));
    setField(dbConfig::paramName(GMBH_GEFUE3), dbConfig::readValue(GMBH_GEFUE3));
    setField(dbConfig::paramName(GMBH_DKV), dbConfig::readValue(GMBH_DKV));
}

wpContractLableInfo_Page::wpContractLableInfo_Page(QWidget *p) : QWizardPage(p)
{
    setTitle(qsl("Vertragskennung"));
    QLabel *subTitleLabel = new QLabel(qsl("Keine Daten!"));
    subTitleLabel->setWordWrap(true);
    subTitleLabel->setText(qsl("Diese Informationen werden verwendet um eindeutige Kennzeichen für die einzelnen Verträge zu erzeugen"));

    QLabel *lProject = new QLabel(qsl("Projekt Kürzel (2-5 Zeichen):"));
    QLineEdit *leProject = new QLineEdit;
    lProject->setBuddy(leProject);
    registerField(dbConfig::paramName(GMBH_INITIALS), leProject);
    leProject->setToolTip(qsl("Das Kürzel wird bei der Erstellung von Vertragskennzeichen verwendet"));

    QLabel *lIndex = new QLabel(qsl("Start Index:"));
    QLineEdit* leStartIndex;
    leStartIndex = new QLineEdit;
    lIndex->setBuddy(leStartIndex);
    registerField(dbConfig::paramName(STARTINDEX), leStartIndex);
    lIndex->setToolTip(qsl("Mit diesem Index beginnt die laufende Nummer in den Vertragskennzeichen"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
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
    setField(dbConfig::paramName(STARTINDEX), i2s(startindex));
}

bool wpContractLableInfo_Page::validatePage()
{
    int startindex = field(dbConfig::paramName(STARTINDEX)).toInt();
    setField(dbConfig::paramName(STARTINDEX), i2s(startindex));
    QString project = field(dbConfig::paramName(GMBH_INITIALS)).toString();
    if (project.length() > 5)
        setField(dbConfig::paramName(GMBH_INITIALS), project.left(5));
    return true;
}

wpContractMinValues_Page::wpContractMinValues_Page(QWidget *p) : QWizardPage(p)
{
    setTitle(qsl("Weitere Konfiguration"));

    QLabel* subtitle =new QLabel(qsl("Halte die Maus über die Beschriftungen für weitere Information!"));
    subtitle->setWordWrap (true);

    leMa =new QLineEdit;
    registerField(dbConfig::paramName(MIN_PAYOUT), leMa);
    leMa->setValidator(new QIntValidator(1,1000,this));
    QLabel* lma     =new QLabel(qsl("Kleinster Auszahlungsbetrag in Euro:"));
    lma->setToolTip (qsl("DKV2 wird keine Auszahlungen erlauben, deren Wert geringer ist als der hier angegebene Wert"));
    lma->setBuddy(leMa);

    leMc =new QLineEdit;
    registerField(dbConfig::paramName(MIN_AMOUNT), leMc);
    leMc->setValidator(new QIntValidator(1,1000, this));
    QLabel* lmc     =new QLabel(qsl("Kleinster Vertragswert in Euro:"));
    lmc->setToolTip (qsl("DKV2 wird nicht zulassen, dass der Wert eines Vertrages kleiner wird, als der hier angegebene Wert"));
    lmc->setBuddy(leMc);

    leMi =new QLineEdit;
    registerField(dbConfig::paramName(MAX_INTEREST), leMi);
    leMi->setValidator(new QIntValidator(100, 1000, this));
    QLabel* lmi     =new QLabel(qsl("Größter auswählbarer Zins in 100tel Prozent"));
    lmi->setToolTip (qsl("Diese Angabe beschränkt die Werte, die für den Zins angeboten werden. 100 entspricht 1%"));
    lmi->setBuddy(leMi);

    leMaxINbr =new QLineEdit;
    registerField(dbConfig::paramName(MAX_INVESTMENT_NBR), leMaxINbr);
    leMaxINbr->setValidator(new QIntValidator(1, 100, this));
    QLabel* lmaxINbr =new QLabel(qsl("Grenzwert für die Ansahl von Verträgen pro Investment"));
    lmaxINbr->setToolTip (qsl("Ab der hier angegebenen Anzahl wird im Programm die Anzahl<br> von Verträgen pro Investment rot eingefärbt"));
    lmaxINbr->setBuddy(leMaxINbr);

    leMaxISum =new QLineEdit;
    registerField(dbConfig::paramName(MAX_INVESTMENT_SUM), leMaxISum);
    leMaxISum->setValidator(new QIntValidator(1, 1000000, this));
    QLabel* lmaxISum =new QLabel(qsl("Grenzwert für die Summe von Verträgen pro Investment"));
    lmaxISum->setToolTip (qsl("Ab der hier angegebenen Summe wird im Programm der Wert<br> der VertrÃ¤ge pro Investment rot eingefärbt"));
    lmaxISum->setBuddy(leMaxISum);

    QGridLayout* grid =new QGridLayout;
    int row =0, col =0;
    grid->addWidget (subtitle, row++, col++);
    grid->addWidget(lma,  row, 0);
    grid->addWidget(leMa, row++, 1);
    grid->addWidget(lmc,  row, 0);
    grid->addWidget(leMc, row++, 1);
    grid->addWidget(lmi,  row, 0);
    grid->addWidget(leMi, row++, 1);
    grid->addWidget(lmaxINbr, row, 0);
    grid->addWidget(leMaxINbr, row++, 1);
    grid->addWidget(lmaxISum, row, 0);
    grid->addWidget(leMaxISum, row++, 1);
    setLayout(grid);
}

void wpContractMinValues_Page::initializePage()
{
    setField(dbConfig::paramName(MIN_PAYOUT), dbConfig::readValue(MIN_PAYOUT).toInt());
    setField(dbConfig::paramName(MIN_AMOUNT), dbConfig::readValue(MIN_AMOUNT).toInt());
    setField(dbConfig::paramName(MAX_INTEREST), dbConfig::readValue(MAX_INTEREST).toInt());
    setField(dbConfig::paramName(MAX_INVESTMENT_NBR), dbConfig::readValue(MAX_INVESTMENT_NBR).toInt());
    setField(dbConfig::paramName(MAX_INVESTMENT_SUM), dbConfig::readValue(MAX_INVESTMENT_SUM).toInt ());
}

bool wpContractMinValues_Page::validatePage()
{
    return leMa->hasAcceptableInput() and leMc->hasAcceptableInput() and leMi->hasAcceptableInput() and leMaxINbr->hasAcceptableInput() and leMaxISum->hasAcceptableInput();
}

wpNewDatabase_SummaryPage::wpNewDatabase_SummaryPage(QWidget *p) : QWizardPage(p)
{
    setTitle(qsl("Zusammenfassung"));
    QCheckBox *cb = new QCheckBox(qsl("Die Eingaben sind korrekt!"));
    registerField(qsl("confirmed"), cb);
    cb->setToolTip(qsl("Mit diesem Ankreuzfeld bestätigst Du, dass die Angaben richtig sind."));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    layout->addWidget(cb);
    setLayout(layout);
    connect(cb, &QCheckBox::checkStateChanged, this, &wpNewDatabase_SummaryPage::onConfirmData_toggled);
}
void wpNewDatabase_SummaryPage::initializePage()
{
    QString subt = qsl("<b>Projekt Daten:</b>"
                       "<table>"
                       "<tr><td>Projekt:</td><td>%1</td></tr>"
                       "<tr><td>Adresse:</td><td>%2</td></tr>"
                       "<tr><td></td><td>%3</td></tr>"
                       "<tr><td></td><td>%4</td></tr>"
                       "<tr><td></td><td>%5 %6<br></td></tr>");
    subt = subt.arg(field(dbConfig::paramName(GMBH_PROJECT)).toString());
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
    subTitleLabel->setText(subt);
}
void wpNewDatabase_SummaryPage::onConfirmData_toggled(Qt::CheckState)
{
    emit completeChanged();
}
bool wpNewDatabase_SummaryPage::isComplete() const
{
    return field(qsl("confirmed")).toBool();
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
wizConfigureNewDatabaseWiz::wizConfigureNewDatabaseWiz(QWidget *p) : QWizard(p)
{   LOG_CALL;
    setWizardStyle(QWizard::ModernStyle);
    addPage(new wpFileSelectionNewDb_IntroPage);
    addPage(new wpProjectAddress_Page);
    addPage(new wpProjectDetails_Page);
    addPage(new wpContractMinValues_Page);
    addPage(new wpContractLableInfo_Page);
    addPage(new wpNewDatabase_SummaryPage);
}

void wizConfigureNewDatabaseWiz::updateDbConfig(const QString &dbFile)
{
    LOG_CALL;
    dbCloser closer{qsl("updateDbConfig")};
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    db.setDatabaseName(dbFile);
    if (not db.open())
    {
        qCritical() << "failed to open db" << db.lastError();
        return;
    }
    updateDbConfig(db);
    return;
}

void wizConfigureNewDatabaseWiz::updateDbConfig(const QSqlDatabase &db)
{
    LOG_CALL;
    dbConfig::writeValue(GMBH_PROJECT, field(dbConfig::paramName(GMBH_PROJECT)), db);
    dbConfig::writeValue(GMBH_ADDRESS1, field(dbConfig::paramName(GMBH_ADDRESS1)), db);
    dbConfig::writeValue(GMBH_ADDRESS2, field(dbConfig::paramName(GMBH_ADDRESS2)), db);
    dbConfig::writeValue(GMBH_STREET, field(dbConfig::paramName(GMBH_STREET)), db);
    dbConfig::writeValue(GMBH_PLZ, field(dbConfig::paramName(GMBH_PLZ)), db);
    dbConfig::writeValue(GMBH_CITY, field(dbConfig::paramName(GMBH_CITY)), db);
    dbConfig::writeValue(GMBH_EMAIL, field(dbConfig::paramName(GMBH_EMAIL)), db);
    dbConfig::writeValue(GMBH_URL, field(dbConfig::paramName(GMBH_URL)), db);
    dbConfig::writeValue(GMBH_INITIALS, field(dbConfig::paramName(GMBH_INITIALS)), db);
    dbConfig::writeValue(STARTINDEX, field(dbConfig::paramName(STARTINDEX)), db);
    dbConfig::writeValue(DBID, QVariant(dbConfig::readValue(GMBH_INITIALS).toString() + dbConfig::readValue(STARTINDEX).toString()), db);
    dbConfig::writeValue(GMBH_HRE, field(dbConfig::paramName(GMBH_HRE)), db);
    dbConfig::writeValue(GMBH_GEFUE1, field(dbConfig::paramName(GMBH_GEFUE1)), db);
    dbConfig::writeValue(GMBH_GEFUE2, field(dbConfig::paramName(GMBH_GEFUE2)), db);
    dbConfig::writeValue(GMBH_GEFUE3, field(dbConfig::paramName(GMBH_GEFUE3)), db);
    dbConfig::writeValue(GMBH_DKV, field(dbConfig::paramName(GMBH_DKV)), db);
    dbConfig::writeValue(MIN_PAYOUT, field(dbConfig::paramName(MIN_PAYOUT)), db);
    dbConfig::writeValue(MIN_AMOUNT, field(dbConfig::paramName(MIN_AMOUNT)), db);
    dbConfig::writeValue(MAX_INTEREST, field(dbConfig::paramName(MAX_INTEREST)), db);
    dbConfig::writeValue(MAX_INVESTMENT_NBR, field(dbConfig::paramName(MAX_INVESTMENT_NBR)), db);
    dbConfig::writeValue(MAX_INVESTMENT_SUM, field(dbConfig::paramName(MAX_INVESTMENT_SUM)), db);
}

wpConfigure_IntroPage::wpConfigure_IntroPage(QWidget *p) : QWizardPage(p)
{
    setTitle(qsl("Konfiguration"));
    subTitleLabel->setWordWrap(true);
    subTitleLabel->setText(qsl("Mit dieser Dialogfolge kannst du Konfigurationen "
                    "für Briefdruck und die Datenbank vornehmen."));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(subTitleLabel);
    setLayout(layout);
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
wizConfigureProjectWiz::wizConfigureProjectWiz(QWidget *p) : QWizard(p)
{   LOG_CALL;
    setWizardStyle(QWizard::ModernStyle);
    addPage(new wpConfigure_IntroPage);
    addPage(new wpProjectAddress_Page);
    addPage(new wpProjectDetails_Page);
    addPage(new wpContractMinValues_Page);
}
void wizConfigureProjectWiz::updateDbConfig()
{
    LOG_CALL;
    dbConfig::writeValue(GMBH_PROJECT, field(dbConfig::paramName(GMBH_PROJECT)));
    dbConfig::writeValue(GMBH_ADDRESS1, field(dbConfig::paramName(GMBH_ADDRESS1)));
    dbConfig::writeValue(GMBH_ADDRESS2, field(dbConfig::paramName(GMBH_ADDRESS2)));
    dbConfig::writeValue(GMBH_STREET, field(dbConfig::paramName(GMBH_STREET)));
    dbConfig::writeValue(GMBH_PLZ, field(dbConfig::paramName(GMBH_PLZ)));
    dbConfig::writeValue(GMBH_CITY, field(dbConfig::paramName(GMBH_CITY)));
    dbConfig::writeValue(GMBH_EMAIL, field(dbConfig::paramName(GMBH_EMAIL)));
    dbConfig::writeValue(GMBH_URL, field(dbConfig::paramName(GMBH_URL)));

    dbConfig::writeValue(GMBH_HRE, field(dbConfig::paramName(GMBH_HRE)));
    dbConfig::writeValue(GMBH_GEFUE1, field(dbConfig::paramName(GMBH_GEFUE1)));
    dbConfig::writeValue(GMBH_GEFUE2, field(dbConfig::paramName(GMBH_GEFUE2)));
    dbConfig::writeValue(GMBH_GEFUE3, field(dbConfig::paramName(GMBH_GEFUE3)));
    dbConfig::writeValue(GMBH_DKV, field(dbConfig::paramName(GMBH_DKV)));
    dbConfig::writeValue(MIN_PAYOUT, field(dbConfig::paramName(MIN_PAYOUT)));
    dbConfig::writeValue(MIN_AMOUNT, field(dbConfig::paramName(MIN_AMOUNT)));
    dbConfig::writeValue(MAX_INTEREST, field(dbConfig::paramName(MAX_INTEREST)));
    dbConfig::writeValue(MAX_INVESTMENT_NBR, field(dbConfig::paramName(MAX_INVESTMENT_NBR)));
    dbConfig::writeValue(MAX_INVESTMENT_SUM, field(dbConfig::paramName(MAX_INVESTMENT_SUM)));
}
