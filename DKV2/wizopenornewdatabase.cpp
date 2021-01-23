#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include "appconfig.h"
#include "helper.h"
#include "wizopenornewdatabase.h"

/*
 * wizard Pages: wpOpenOrNew
 */
wpOpenOrNew::wpOpenOrNew(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Datenbank Auswahl"));
    setSubTitle(qsl("Mit dieser Dialogfolge kann die Datenbank zum Speichern der Kreditdaten gewählt werden."));
    QRadioButton* rbNew  = new QRadioButton(qsl("Neue Datenbank anlegen"));
    registerField(qsl("createNewDb"), rbNew);
    QRadioButton* rbOpen = new QRadioButton(qsl("Eine existierende Datenbank öffnen"));
    QVBoxLayout* l =new QVBoxLayout();
    l->addWidget(rbNew);
    l->addWidget(rbOpen);
    setLayout(l);
}

//void wizOpenOrNewDatabase::initializePage()
//{   //LOG_CALL;
//}

//bool wizOpenOrNewDatabase::validatePage()
//{   //LOG_CALL;
//    return true;
//}

int wpOpenOrNew::nextId() const
{
    if(field(qsl("createNewDb")).toBool())
        return selectNewFile;
    else
        return selectExistingFile;
}
/*
 * wizard Pages: wpNewDb
 */
QString defaultFolder() {
    QString selectedFolder =appConfig::LastDb();
    if( selectedFolder.isEmpty()) {
        selectedFolder =QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    } else {
        selectedFolder = QFileInfo(selectedFolder).absolutePath();
    }
    return selectedFolder;
}

QString defaultDKDB_Filename() {
    return qsl("DK-Datenbank.dkdb");
}

wpNewDb::wpNewDb(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Angaben zur neuen Datenbank"));
    setSubTitle(qsl("Wähle ein Verzeichnis aus und gib den Namen für die neue Datenbank an"));

    QLabel* lVerzeichnis =new QLabel(qsl("&Verzeichnis"));
    QLineEdit* verzeichnis =new QLineEdit();
    registerField(qsl("dbFolder"), verzeichnis);
    lVerzeichnis->setBuddy(verzeichnis);
    QHBoxLayout* layoutFolder =new QHBoxLayout();
    layoutFolder->addWidget(lVerzeichnis);
    layoutFolder->addWidget(verzeichnis);

    QLabel* lDateiname =new QLabel(qsl("&Dateiname"));
    QLineEdit* dateiname =new QLineEdit();
    registerField(qsl("dbFilename"), dateiname);
    lDateiname->setBuddy(dateiname);
    QHBoxLayout* layoutFilename =new QHBoxLayout();
    layoutFilename->addWidget(lDateiname);
    layoutFilename->addWidget(dateiname);

    QVBoxLayout* l =new QVBoxLayout();
    l->addLayout(layoutFolder);
    l->addLayout(layoutFilename);
    setLayout(l);
}

void wpNewDb::initializePage()
{
    setField(qsl("dbFolder"), defaultFolder());
    setField(qsl("dbFilename"), defaultDKDB_Filename());
}

void wpNewDb::setVisible(bool v)
{   LOG_CALL;
    QWizardPage::setVisible(v);
    wizard()->setOption(QWizard::HaveCustomButton1, v);
    if(v) {
        wizard()->setButtonText(QWizard::CustomButton1, qsl("Verz. auswählen"));
        connect(wizard(), &QWizard::customButtonClicked, this, &wpNewDb::browseButtonClicked);
    } else {
        disconnect(wizard(), &QWizard::customButtonClicked, this, &wpNewDb::browseButtonClicked);
    }
}

void wpNewDb::browseButtonClicked()
{   LOG_CALL;
     setField( qsl("dbFolder"),
               QFileDialog::getExistingDirectory(this, qsl("Datenbank Verzeichnis"),
                                                 field(qsl("dbFolder")).toString(),
                                                 QFileDialog::ShowDirsOnly ));
}

bool wpNewDb::validatePage()
{   LOG_CALL;
    QString folder =field(qsl("dbFolder")).toString();
    QFileInfo fiFolder (folder);
    QString file =field("dbFilename").toString().trimmed();
    if( !file.contains('.')) file += ".dkdb";
    QString path =folder +qsl("/") +file;
    bool allGood =false;
    do {
        if( ! fiFolder.isDir()) break;
        if( ! fiFolder.isWritable()) break;
        if( QFile(path).exists()) break;
        allGood =true;
    }
    while(false);
    if( allGood) {
        qobject_cast<wizOpenOrNewDb*>( wizard())->selectedFile = path;
        return true;
    }
    qDebug() << "new DB file / folder validation failed";
    QMessageBox::information(this, qsl("Ungültige Eingabedaten"),
                             qsl("Die Datei kann nicht angelegt werden. Bitte stelle sicher, dass in dem "
                             "Verzeichnis geschrieben werden darf und dass es keine gleichnamige Datei in dem Verzeichnis gibt."));
    return false;
}

int wpNewDb::nextId() const
{
    return -1;
}

/*
 * wizard Pages: wpExistingDb
 */
wpExistingDb::wpExistingDb(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("DK Datenbank auswählen"));
    setSubTitle(qsl("Wähle eine existierende DK Datenbank (*.dkdb) aus"));
    QLabel* lAuswahl =new QLabel(qsl("&Ausgewählte Datei"));
    QLineEdit* leFullfile =new QLineEdit();
    leFullfile->setReadOnly(true);
    registerField(qsl("existingFile"), leFullfile);
    lAuswahl->setBuddy(leFullfile);
    QHBoxLayout* hbl =new QHBoxLayout();
    hbl->addWidget(lAuswahl);
    hbl->addWidget(leFullfile);
    setLayout(hbl);
}

void wpExistingDb::initializePage()
{

}

bool wpExistingDb::validatePage()
{
    QString db =field(qsl("existingFile")).toString();
    if( QFileInfo::exists(db)) {
        qobject_cast<wizOpenOrNewDb*>( wizard())->selectedFile = db;
        return true;
    }
    return false;
}

int wpExistingDb::nextId() const
{
    return -1;
}

void wpExistingDb::setVisible(bool v)
{   LOG_CALL;
    QWizardPage::setVisible(v);
    wizard()->setOption(QWizard::HaveCustomButton1, v);
    if(v) {
        wizard()->setButtonText(QWizard::CustomButton1, qsl("Datenbank auswählen"));
        connect(wizard(), &QWizard::customButtonClicked, this, &wpExistingDb::browseButtonClicked);
    } else {
        disconnect(wizard(), &QWizard::customButtonClicked, this, &wpExistingDb::browseButtonClicked);
    }
}

void wpExistingDb::browseButtonClicked()
{   LOG_CALL;
     setField( qsl("existingFile"),
               QFileDialog::getOpenFileName(this, qsl("Datenbankdatei auswählen"),
                                            defaultFolder(), qsl("DK Datenbank *.dkdb *.db")));
}


/*
 * Wizard: wizOpenOrNewDb
 */
wizOpenOrNewDb::wizOpenOrNewDb(QWidget* p) : QWizard(p)
{   LOG_CALL;
    setPage(NewOrOpen, new wpOpenOrNew(this));
    setPage(selectNewFile, new wpNewDb(this));
    setPage(selectExistingFile, new wpExistingDb(this));
}
