#include "wizopenornewdatabase.h"

#include "helper.h"
#include "appconfig.h"

/*
 * wizard Pages: wpOpenOrNew
 */
wpOpenOrNew::wpOpenOrNew(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Datenbank Auswahl"));
    setSubTitle(qsl("Mit dieser Dialogfolge kann die Datenbank zum Speichern der Kreditdaten gewählt werden."));
// subTitleLabel->setWordWrap(true);
// subTitleLabel->setText(qsl("Mit dieser Dialogfolge kann die Datenbank zum Speichern der Kreditdaten gewählt werden."));

    QVBoxLayout* lv =new QVBoxLayout();
//    lv->addWidget(subTitleLabel);
    QRadioButton* rbNew =new QRadioButton(qsl("Neue Datenbank anlegen"), this);
    QRadioButton* rbOpen =new QRadioButton(qsl("Eine existierende Datenbank öffnen"), this);
    rbOpen->setChecked(true);
    registerField(fnCreateNew, rbNew);

    lv->addWidget(rbNew);
    lv->addWidget(rbOpen);

    setLayout(lv);
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
    if(field(fnCreateNew).toBool())
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
    subTitleLabel->setWordWrap(true);
    subTitleLabel->setText(qsl("Wähle ein Verzeichnis aus und gib den Namen für die neue Datenbank an"));

    QLabel* lVerzeichnis =new QLabel(qsl("&Verzeichnis"));
    QLineEdit* verzeichnis =new QLineEdit();
    registerField(fnDbFolder, verzeichnis);
    lVerzeichnis->setBuddy(verzeichnis);
    QHBoxLayout* layoutFolder =new QHBoxLayout();
    layoutFolder->addWidget(lVerzeichnis);
    layoutFolder->addWidget(verzeichnis);

    QLabel* lDateiname =new QLabel(qsl("&Dateiname"));
    QLineEdit* dateiname =new QLineEdit();
    registerField(fnDbFilename, dateiname);
    lDateiname->setBuddy(dateiname);
    QHBoxLayout* layoutFilename =new QHBoxLayout();
    layoutFilename->addWidget(lDateiname);
    layoutFilename->addWidget(dateiname);

    QVBoxLayout* l =new QVBoxLayout();
    l->addWidget(subTitleLabel);
    l->addLayout(layoutFolder);
    l->addLayout(layoutFilename);
    setLayout(l);
}

void wpNewDb::initializePage()
{
    setField(fnDbFolder, defaultFolder());
    setField(fnDbFilename, defaultDKDB_Filename());
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
    QString folder =QFileDialog::getExistingDirectory(this, qsl("Datenbank Verzeichnis"),
                          field(fnDbFolder).toString(), QFileDialog::ShowDirsOnly );

    if( not folder.isEmpty ())
        setField( fnDbFolder, folder);

}

bool wpNewDb::validatePage()
{   LOG_CALL;
    QString folder =field(fnDbFolder).toString();
    QFileInfo fiFolder (folder);
    QString file =field(fnDbFilename).toString().trimmed();
    if( not file.contains('.')) file += qsl(".dkdb");
    QString path =folder +qsl("/") +file;
    bool allGood =false;
    do {
        if( not fiFolder.isDir()) break;
        if( not fiFolder.isWritable()) break;
        if( QFile(path).exists()) break;
        allGood =true;
    }
    while(false);
    if( allGood) {
        qobject_cast<wizOpenOrNewDb*>( wizard())->selectedFile = path;
        return true;
    }
    qCritical() << "new DB file / folder validation failed";
    QMessageBox::information(this, qsl("Ungültige Eingabedaten"),
                             qsl("Die Datei kann nicht angelegt werden. Bitte stelle sicher, dass in dem "
                             "Verzeichnis geschrieben werden darf und dass es keine gleichnamige Datei in dem Verzeichnis gibt."));
    return false;
}

int wpNewDb::nextId() const
{
    return Zinssusance;
}

/*
 * wizard Pages: wpICalcMode
 */
wpICalcMode::wpICalcMode(QWidget* p) : QWizardPage (p)
{
    setTitle(qsl("Angaben zu Zinsmethode (Zinssusance)"));
    setSubTitle (qsl("DKV2 unterstütz die Methoden <i>(30/360)</i> und <i>(act/act)</i> der Zinsberechnung"));
    QLabel* lbl =new QLabel(qsl("Die Unterschiede sind geringfügig und treten nur in Jahren auf, "
                    "die nicht vollständig im Zinszeitraum liegen. <br>Sie stellen "
                   "keinen besonderen Vor- oder Nachteil für Projekt oder DK Geber*innen dar."));

    QRadioButton* rb30360 =new QRadioButton(qsl("30 / 360"));
    rb30360->setToolTip (qsl("Für jeden Tag gibt es 1/360-tel des Jahreszins. Es werden 30 Tage pro Monat berechnet.<br>"
                             "Schaltjahre und normale Jahre werden gleich bewertet."));
    QLabel* lbl360 =new QLabel(qsl("Für jeden Tag gibt es 1/360-tel des Jahreszins. Es werden 30 Tage pro Monat berechnet.<br>"
                                   "Schaltjahre und normale Jahre werden gleich bewertet."));

    QRadioButton* rbactact=new QRadioButton(qsl("act / act"));
    rbactact->setToolTip (qsl("In Schaltjahren wird ein Tag mit 1/366-tel des Jahreszins berechnet, <br>in anderen Jahren mit einem 1/365-tel."));
    registerField (fnZinssusance, rb30360);
    QLabel* lblact =new QLabel(qsl("In Schaltjahren wird ein Tag mit 1/366-tel des Jahreszins berechnet, <br>in anderen Jahren mit einem 1/365-tel."));

    QLabel *lblInfo =new QLabel(qsl("<br>Wichtig! Der Zinsmodus gilt für alle Verträge der Datenbank <br>und kann nachträglich nicht mehr geändert werden!"));
    lblInfo->setTextFormat (Qt::RichText);
    QVBoxLayout* l =new QVBoxLayout();
    l->addWidget (lbl);
    l->addWidget (rb30360);
    l->addWidget (lbl360);
    l->addWidget (rbactact);
    l->addWidget (lblact);
    l->addWidget (lblInfo);
    setLayout (l);
}

void wpICalcMode::initializePage()
{
    setField (fnZinssusance, true);
}

int wpICalcMode::nextId () const
{
    return -1;
}

/*
 * wizard Pages: wpExistingDb
 */
wpExistingDb::wpExistingDb(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("DK Datenbank auswählen"));
    subTitleLabel->setWordWrap(true);
    subTitleLabel->setText(qsl("Wähle eine existierende DK Datenbank (*.dkdb) aus"));
    QLabel* lAuswahl =new QLabel(qsl("&Ausgewählte Datei"));
    QLineEdit* leFullfile =new QLineEdit();
    leFullfile->setReadOnly(true);
    registerField(fnExistingFile, leFullfile);
    lAuswahl->setBuddy(leFullfile);
    QHBoxLayout* hbl =new QHBoxLayout();
    hbl->addWidget(lAuswahl);
    hbl->addWidget(leFullfile);
    QVBoxLayout *lv = new QVBoxLayout();
    lv->addWidget(subTitleLabel);
    lv->addLayout(hbl);
    setLayout(lv);
}

void wpExistingDb::initializePage()
{

}

bool wpExistingDb::validatePage()
{
    QString db =field(fnExistingFile).toString();
    if( QFileInfo::exists(db)) {
        qobject_cast<wizOpenOrNewDb*>( wizard())->selectedFile = db;
        return true;
    }
    QMessageBox::information(this, qsl("Ungültige Eingabedaten"),
                             qsl("Die Datei kann nicht geöffnet werden. Bitte stelle sicher, dass "
                             "eine existierende Datenbankdatei ausgewählt wird."));
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

    QString file =QFileDialog::getOpenFileName(this, qsl("Datenbankdatei auswählen"),
                                  defaultFolder(), qsl("DK Datenbank *.dkdb *.db"));
    if(not file.isEmpty())
        setField( fnExistingFile,file);
}

/*
 * Wizard: wizOpenOrNewDb
 */
wizOpenOrNewDb::wizOpenOrNewDb(QWidget* p) : QWizard(p)
{   LOG_CALL;
    setWizardStyle(QWizard::ModernStyle);
    setPage(NewOrOpen, new wpOpenOrNew(this));
    setPage(selectNewFile, new wpNewDb(this));
    setPage (Zinssusance, new wpICalcMode(this));
    setPage(selectExistingFile, new wpExistingDb(this));
}
