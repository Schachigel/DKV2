#include <windows.h>

#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QFileDialog>
#include <QRandomGenerator>


#include "dkdbhelper.h"
#include "wizfileselection.h"

/*
 * general page to select a file name
 * NOT used for newDB
*/
wizFileSelection_IntroPage::wizFileSelection_IntroPage(QWidget* p) : QWizardPage(p) {
    QLineEdit* le = new QLineEdit;
    registerField("selectedFile", le);
    QLabel* l =new QLabel("Klicke auf 'durchsuchen' um eine Dateiauswahl Fenster zu öffnen");
    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

void wizFileSelection_IntroPage::initializePage()
{
    fileSelectionWiz* wiz = dynamic_cast<fileSelectionWiz*>(wizard());
    if( wiz) {
        setTitle(wiz->title);
        setSubTitle(wiz->subtitle);
    } else {
        newDatabaseWiz* nDbWiz = dynamic_cast<newDatabaseWiz*>(wizard());
        setTitle(nDbWiz->title);
        setSubTitle(nDbWiz->subtitle);
    }
}

void wizFileSelection_IntroPage::browseButtonClicked() {
    fileSelectionWiz* wiz = dynamic_cast<fileSelectionWiz*>(wizard());

    QString selectedFile =( wiz->existingFile) ? QFileDialog::getOpenFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr)
            : QFileDialog::getSaveFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr);
    if(selectedFile.isEmpty())
        return;
    setField( "selectedFile", selectedFile);
}

bool wizFileSelection_IntroPage::validatePage() {
    fileSelectionWiz* wiz = dynamic_cast<fileSelectionWiz*>(wizard());
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
fileSelectionWiz::fileSelectionWiz(QWidget* p) : QWizard(p) {
    addPage(new wizFileSelection_IntroPage);
}

/*
 * page to select a file name used for newDB
*/
wizFileSelectionNewDb_IntroPage::wizFileSelectionNewDb_IntroPage(QWidget* p) : QWizardPage(p) {
    QLineEdit* le = new QLineEdit;
    registerField("selectedFile", le);
    QLabel* l =new QLabel("Klicke auf 'durchsuchen' um eine Dateiauswahl Fenster zu öffnen");
    QVBoxLayout* layout =new QVBoxLayout;
    layout->addWidget(l);
    layout->addWidget(le);
    setLayout(layout);
}

void wizFileSelectionNewDb_IntroPage::initializePage()
{
    newDatabaseWiz* wiz = dynamic_cast<newDatabaseWiz*>(wizard());
    setTitle(wiz->title);
    setSubTitle(wiz->subtitle);
}

void wizFileSelectionNewDb_IntroPage::browseButtonClicked() {
    newDatabaseWiz* wiz = dynamic_cast<newDatabaseWiz*>(wizard());

    QString selectedFile =( wiz->existingFile) ? QFileDialog::getOpenFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr)
            : QFileDialog::getSaveFileName(nullptr, wiz->bffTitle, wiz->openInFolder, wiz->fileTypeDescription, nullptr);
    if(selectedFile.isEmpty())
        return;
    setField( "selectedFile", selectedFile);
}

bool wizFileSelectionNewDb_IntroPage::validatePage() {
    newDatabaseWiz* wiz = dynamic_cast<newDatabaseWiz*>(wizard());
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
wizProjectDetails_Page::wizProjectDetails_Page(QWidget* p) : QWizardPage(p)
{
    setTitle("Daten Deiner Projekt GmbH");
    QLineEdit* leAddress1 = new QLineEdit;
    registerField("address1", leAddress1);
    QLineEdit* leAddress2 = new QLineEdit;
    registerField("address2", leAddress2);
    QLineEdit* lePlz =      new QLineEdit;
    registerField("plz", lePlz);
    QLineEdit* leStadt =    new QLineEdit;
    registerField("stadt", leStadt);
    QLineEdit* leStrasse =  new QLineEdit;
    registerField("strasse", leStrasse);
    QLineEdit* leEmail =    new QLineEdit;
    registerField("email", leEmail);
    QLineEdit* leUrl   =    new QLineEdit;
    registerField("url", leUrl);

    QGridLayout* grid  = new QGridLayout;
    grid->addWidget(leAddress1, 0, 0, 1, 5);
    grid->addWidget(leAddress2, 1, 0, 1, 5);
    grid->addWidget(leStrasse,  2, 0, 1, 5);
    lePlz->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    leStadt->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    grid->addWidget(lePlz,      3, 0, 1, 1);
    grid->addWidget(leStadt,    3, 1, 1, 4);
    grid->addWidget(leEmail,    4, 0, 1, 5);
    grid->addWidget(leUrl,      5, 0, 1, 5);
    setLayout(grid);
}

void wizProjectDetails_Page::initializePage()
{
    setField("address1", getMetaInfo("gmbh.address1", "Esperanza Franklin GmbH"));
    setField("address2", getMetaInfo("gmbh.address2", ""));
    setField("strasse",  getMetaInfo("gmbh.strasse",  "Turley-Platz 9"));
    setField("plz",      getMetaInfo("gmbh.plz",      "68167"));
    setField("stadt",    getMetaInfo("gmbh.stadt",    "Mannheim"));
    setField("email",    getMetaInfo("gmbh.email",    "info@esperanza-mannheim.de"));
    setField("url",      getMetaInfo("gmbh.url",      "www.esperanza-mannheim.de"));
}

wizContractLableInfo_Page::wizContractLableInfo_Page(QWidget* p) : QWizardPage(p)
{
    setTitle("Vertragskennung");
    setSubTitle("Diese Informationen werden verwendet um eindeutige Kennzeichen für die einzelnen Verträge zu erzeugen");

    QLabel* lProject= new QLabel;
    lProject->setText("Projekt Kürzel (2-5 Zeichen):");
    QLineEdit* leProject = new QLineEdit;
    lProject->setBuddy(leProject);
    registerField("projekt", leProject);

    QLabel* lIndex = new QLabel;
    lIndex->setText("Start Index:");
    QLineEdit* leStartIndex = new QLineEdit;
    lIndex->setBuddy(leStartIndex);
    registerField("startindex", leStartIndex);

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
    setField("projekt", getMetaInfo("ProjektInitialen", "ESP"));
    int startindex = rand.bounded(1000, 9999);
    setField("startindex", QString::number(startindex));
}

bool wizContractLableInfo_Page::validatePage()
{
    int startindex = field("startindex").toInt();
    setField("startindex", QString::number(startindex));
    QString project = field("project").toString();
    if(project.length()> 5)
        setField("projekt", project.left(5));
    return true;
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
    subt = subt.arg(field("address1").toString());
    subt = subt.arg(field("address2").toString());
    subt = subt.arg(field("strasse").toString());
    subt = subt.arg(field("plz").toString());
    subt = subt.arg(field("stadt").toString());

    subt += "<tr><td>E-Mail:</td><td>%1</td></tr>"
            "<tr><td>Web:</td><td>%2</td></tr>"
            "<tr><td>Kürzel:    </td><td>%3</td></tr>"
            "<tr><td>Start Index:&nbsp;&nbsp;</td><td>%4</td></tr></table>";
    subt = subt.arg(field("email").toString()).arg(field("url").toString());
    subt = subt.arg(field("projekt").toString());
    subt = subt.arg(field("startindex").toString());
    setSubTitle(subt);
}
bool wizNewDatabase_SummaryPage::validatePage()
{
    return field("confirmed").toBool();
}

/*
 * newDb wizard: filename, GmbH data, db data
*/
newDatabaseWiz::newDatabaseWiz(QWidget* p) : QWizard(p) {
    addPage(new wizFileSelectionNewDb_IntroPage);
    addPage(new wizProjectDetails_Page);
    addPage(new wizContractLableInfo_Page);
    addPage(new wizNewDatabase_SummaryPage);
}
