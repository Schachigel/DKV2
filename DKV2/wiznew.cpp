#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "helperfin.h"
#include "appconfig.h"
#include "creditor.h"

#include "investment.h"
#include "wiznew.h"

/*
 * wizNewOrExistingPage - ask user if a new user should be created or an existing
 * one should be used
*/
const QString pnNew {qsl("create_new")};
const QString pnCreditor {qsl("creditor")};

wpNewOrExisting::wpNewOrExisting(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Vertrag anlegen"));
    setSubTitle(qsl("Mit dieser Dialogfolge kannst Du einen Kredigeber*in und Vertrag anlegen."));
    rbNew =new QRadioButton(qsl("Neuen Kreditgeber*in anlegen"));
    rbNew->setToolTip(qsl("Wähle diese Option um einen Kreditgeber oder eine Kreditgeberin anzulegen. "
                          "Danach kannst Du die Eingabe beenden oder einen Vertrag anlegen."));
    rbExisting =new QRadioButton(qsl("Existierenden Kreditgeber*in verwenden"));
    rbExisting->setToolTip(qsl("Wähle diese Option um einen Vertrag zu einem bereits erfassten "
                               "Kreditgeber*in einzugeben."));
    registerField(pnNew, rbNew);
    cbCreditors = new QComboBox();
    cbCreditors->setToolTip(qsl("Wähle hier den Kreditgeber*in aus, für den der Vertrag angelegt werden soll."));
    registerField(pnCreditor, cbCreditors);
    QVBoxLayout* l =new QVBoxLayout();
    l->addWidget(rbNew);
    l->addWidget(rbExisting);
    l->addWidget(cbCreditors);
    setLayout(l);
    connect(rbExisting, SIGNAL(toggled(bool)), this, SLOT(onExistingCreditor_toggled(bool)));
}
void wpNewOrExisting::initializePage()
{   LOG_CALL;
    if( init) {
        setField(pnNew, true);
        QList<QPair<int, QString>> Personen;
        cbCreditors->clear();
        KreditorenListeMitId(Personen);
        if( Personen.count()) {
            for(auto& Entry :qAsConst(Personen)) {
                cbCreditors->addItem( Entry.second, QVariant((Entry.first)));
            }
            rbExisting->setChecked(false);
            rbNew->setChecked(true);
            cbCreditors->setEnabled(false);
        } else {
            rbNew->setChecked(true);
            cbCreditors->setEnabled(false);
            rbExisting->setEnabled(false);
        }
    }
    init =false; // do not repeat after "back"
}
void wpNewOrExisting::onExistingCreditor_toggled(bool b)
{   LOG_CALL;
    cbCreditors->setEnabled(b);
}
bool wpNewOrExisting::validatePage()
{   LOG_CALL;

    if( field(pnNew).toBool())
        qInfo() << "User chose to create a new creditor";
    else {
        wizNew* wiz =qobject_cast<wizNew*> (wizard());
        wiz->creditorId = cbCreditors->itemData(field(pnCreditor).toInt()).toLongLong();
        creditor c(wiz->creditorId);
        setField(pnFName, c.firstname());
        setField(pnLName, c.lastname());
        setField(pnStreet, c.street());
        setField(pnPcode, c.postalCode());
        setField(pnCity, c.city());
        setField(pnCountry, c.country());
        setField(pnEMail, c.email());
        setField(pnComment, c.comment());
        setField(pnIban, c.iban());
        setField(pnBic, c.bic());
        qInfo() << "User selected user Id " << wiz->creditorId;
    }
    return true;
}
int wpNewOrExisting::nextId() const
{   LOG_CALL;
    if( field(pnNew).toBool())
        return page_address;
    else
        return page_label_and_amount;
}

const QString pnFName{qsl("firstname")};
const QString pnLName{qsl("lastname")};
const QString pnStreet{qsl("street")};
const QString pnCity{qsl("city")};
const QString pnPcode{qsl("pcode")};
const QString pnCountry{qsl("country")};
/*
 * wizNewCreditorAddressPage - ask address data for the new creditor
*/
wpNewCreditorAddress::wpNewCreditorAddress(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Adresse"));
    setSubTitle(qsl("Gib hier Namen und die Adressdaten ein."));
    QLineEdit* leFirstName =new QLineEdit;
    leFirstName->setToolTip(qsl("Die Felder für Vor- und Nachname dürfen nicht beide leer sein"));
    registerField(pnFName, leFirstName);
    QLineEdit* leLastName =new QLineEdit;
    registerField(pnLName, leLastName);
    leLastName->setToolTip(qsl("Die Felder für Vor- und Nachname dürfen nicht beide leer sein"));
    QLineEdit* leStreet =new QLineEdit;
    leStreet->setToolTip(qsl("Die Felder Straße, Postleitzahl und Stadt dürfen nicht leer sein"));
    registerField(pnStreet, leStreet);
    QLineEdit* lePlz =new QLineEdit;
    lePlz->setToolTip(qsl("Die Felder Straße, Postleitzahl und Stadt dürfen nicht leer sein"));
    registerField(pnPcode, lePlz);
    QLineEdit* leCity =new QLineEdit;
    leCity->setToolTip(qsl("Die Felder Straße, Postleitzahl und Stadt dürfen nicht leer sein"));
    registerField(pnCity, leCity);
    QLineEdit* leCountry =new QLineEdit;
    leCountry->setToolTip(qsl("Trage hier ein Land ein, falls die Kreditor*in nicht in Deutschland wohnt"));
    registerField(pnCountry, leCountry);

    QLabel* l1 =new QLabel(qsl("Name"));
    QLabel* l2 =new QLabel(qsl("Straße"));
    QLabel* l3 =new QLabel(qsl("Plz/Ort"));
    QLabel* l4 =new QLabel(qsl("Land"));

    QGridLayout* grid =new QGridLayout;
    grid->addWidget(l1,          0, 0, 1, 1);
    grid->addWidget(leFirstName, 0, 1, 1, 2);
    grid->addWidget(leLastName,  0, 3, 1, 2);

    grid->addWidget(l2,          1, 0, 1, 1);
    grid->addWidget(leStreet,    1, 1, 1, 4);

    grid->addWidget(l3,          2, 0, 1, 1);
    grid->addWidget(lePlz,       2, 1, 1, 1);
    grid->addWidget(leCity,      2, 2, 1, 3);

    grid->addWidget(l4,          3, 0, 1, 1);
    grid->addWidget(leCountry,   3, 1, 2, 1);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 2);
    grid->setColumnStretch(2, 2);
    grid->setColumnStretch(3, 2);
    grid->setColumnStretch(4, 2);

    setLayout(grid);
}
void wpNewCreditorAddress::initializePage()
{
    //    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    //    if( not wiz->inUpdateMode()) {
    //        setField(pnFName, QString());
    //        setField(pnLName, QString());
    //        setField(pnStreet, QString());
    //        setField(pnPcode, QString());
    //        setField(pnCity, QString());
    //        setField(pnCountry, QString());
    //    }
}
bool wpNewCreditorAddress::validatePage()
{   LOG_CALL;
    setField(pnFName,     field(pnFName).toString().trimmed());
    setField(pnLName,     field(pnLName).toString().trimmed());
    setField(pnStreet,    field(pnStreet)   .toString().trimmed());
    setField(pnPcode,     field(pnPcode)    .toString().trimmed());
    setField(pnCity,      field(pnCity)     .toString().trimmed());
    setField(pnCountry,   field(pnCountry)  .toString().trimmed());

    if( field(pnFName).toString().isEmpty()
            &&
            field(pnLName).toString().isEmpty())
        return false;
    if( field(pnStreet).toString().isEmpty() ||
            field(pnPcode).toString().isEmpty() ||
            field(pnCity).toString().isEmpty())
        return false;
    else
        return true;
}
int wpNewCreditorAddress::nextId() const
{   LOG_CALL;
    return page_email;
}

/*
 * wizEmailPage - ask e-mail and comment for the new creditor
*/
const QString pnEMail {qsl("e-mail")};
const QString pnComment {qsl("comment")};

wpEmail::wpEmail(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("E-mail"));
    setSubTitle(qsl("Gib hier die E-Mail Adresse und eine Anmerkung an."));

    QLabel* l1 =new QLabel(qsl("E-mail"));
    QLineEdit* leEmail =new QLineEdit;
    leEmail->setToolTip(qsl("Die E-Mail Adresse muss im gültigen Format vorliegen "
                            "(s. https://de.wikipedia.org/wiki/E-Mail-Adresse)."));
    registerField(pnEMail, leEmail);
    QLabel* l2 =new QLabel(qsl("Anmerkung"));
    QPlainTextEdit* eComment =new QPlainTextEdit;
    registerField(pnComment, eComment, "plainText", "textChanged");
    eComment->setToolTip(qsl("Es ist hilfreich in der Anmerkung den Kontakt "
                             "im Projekt oder sonstige Besonderheiten vermerken."));
    QGridLayout* g =new QGridLayout;
    g->addWidget(l1,      0, 0, 1, 1);
    g->addWidget(leEmail, 0, 1, 1, 4);
    g->addWidget(l2,      1, 0, 1, 1);
    g->addWidget(eComment, 1, 1, 2, 4);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 2);
    g->setColumnStretch(2, 2);
    g->setColumnStretch(3, 2);
    g->setColumnStretch(4, 2);
    setLayout(g);
}
bool wpEmail::validatePage()
{   LOG_CALL;
    QString email =field(pnEMail).toString().trimmed().toLower();
    setField(pnEMail, email);
    if( not email.isEmpty())
    {
        QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                              QRegularExpression::CaseInsensitiveOption);
        if( not rx.match(field(pnEMail).toString()).hasMatch()) {
            QMessageBox::information(this, qsl("Fehler"), qsl("Das Format der e-mail Adresse ist ungültig: ") + email);
            return false;
        }
    }
    setField(pnComment, field(pnComment).toString().trimmed());
    return true;
}
int wpEmail::nextId() const
{   LOG_CALL;
    return page_bankaccount;
}

/*
 * wpBankAccount - ask bank account data
*/
const QString pnIban {qsl("iban")};
const QString pnBic  {qsl("bic")};
wpBankAccount::wpBankAccount(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Bankdaten"));
    setSubTitle(qsl("Gib die Bankdaten der neuen Kreditor*in ein."));
    QLabel* l1 =new QLabel(qsl("IBAN"));
    QLineEdit* leIban =new QLineEdit;
    leIban->setToolTip(qsl("Es muss eine gültige IBAN eingegeben werden "
                           "(s. https://de.wikipedia.org/wiki/Internationale_Bankkontonummer)."));
    l1->setBuddy(leIban);
    registerField(pnIban, leIban);
    QLabel* l2 =new QLabel(qsl("BIC <small>(optional)</small>"));
    QLineEdit* leBic =new QLineEdit;
    l2->setBuddy(leBic);
    registerField(pnBic, leBic);

    QGridLayout* g =new QGridLayout;
    g->addWidget(l1);
    g->addWidget(leIban);
    g->addWidget(l2);
    g->addWidget(leBic);
    setLayout(g);
}
bool wpBankAccount::validatePage()
{   LOG_CALL;
    QString formatedIban =field(pnIban).toString().trimmed();
    QString iban =formatedIban.remove(' ');
    if( not iban.isEmpty()) {
        IbanValidator iv; int pos =0;
        if( iv.validate(iban, pos) not_eq IbanValidator::State::Acceptable) {
            QMessageBox::information(this, "Fehler", "Die eingegebene Zeichenfolge ist keine gültige IBAN");
            return false;
        }
    }
    setField(pnIban, iban);
    setField(pnBic, field(pnBic).toString().trimmed());
    return true;
}
int wpBankAccount::nextId() const
{   LOG_CALL;
    return page_confirm_creditor;
}

/*
 * wizConfirmCreditorData : ask if creditor data is OK and if a contract should be created
*/
const QString pnConfirmCreditor {qsl("confirmCreateContract")};
const QString pnCreateContract {qsl("createContract")};

wpConfirmCreditor::wpConfirmCreditor(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Daten bestätigen"));
    QCheckBox* cbConfirmCreditor =new QCheckBox(qsl("Die Angaben sind richtig!"));
    cbConfirmCreditor->setToolTip(qsl("Du musst die Daten prüfen und ihre Richtigkeit "
                                      "bestätigen um sie speichern zu können."));
    registerField(pnConfirmCreditor+qsl("*"), cbConfirmCreditor);
    cbConfirmCreditor->setChecked(false);
    cbCreateContract =new QCheckBox(qsl("Im Anschluß Vertragsdaten eingeben"));
    cbCreateContract->setToolTip(qsl("Selektiere dieses Feld, um gleich einen neuen Vertrag anzulegen"));
    registerField(pnCreateContract, cbCreateContract);
    cbCreateContract->setChecked(true);

    QVBoxLayout* l =new QVBoxLayout;
    l->addWidget(cbConfirmCreditor);
    l->addWidget(cbCreateContract);
    setLayout(l);
    connect(cbCreateContract, SIGNAL(stateChanged(int)), this, SLOT(onConfirmCreateContract_toggled(int)));
    setCommitPage(true);
}
void wpConfirmCreditor::initializePage()
{   LOG_CALL;
    QString creditorSummary {qsl("<table><tr><td>Name:</td><td><b>%1 %2</b><br></td></tr>"
                                 "<tr><td>Straße:</td><td>%3</td></tr>"
                                 "<tr><td>Plz/Ort:</td><td>%4</td></tr>"
                                 "<tr><td>E-Mail:</td><td>%5<br></td></tr>"
                                 "<tr><td>Kommentar:</td><td><small>%6<small></td></tr>"
                                 "<tr><td>Bankdaten:</td><td><table><tr><td>IBAN:</td><td>%7</td></tr><tr><td>BIC:</td><td>%8<td></tr></table></td></tr></table>")};
    QString firstn =field(pnFName).toString().isEmpty() ? qsl("(kein Vorname)")  : field(pnFName).toString();
    QString lastn  =field(pnLName).toString().isEmpty()  ? qsl("(kein Nachname)") : field(pnLName).toString();
    QString street =field(pnStreet).toString().isEmpty()    ? qsl("(keine Strasse)") : field(pnStreet).toString();

    QString pcode  =field(pnPcode).toString().isEmpty()     ? qsl("(keine PLZ)")     : field(pnPcode).toString();
    QString city   =field(pnCity).toString().isEmpty()      ? qsl("(keine Stadt)")   : field(pnCity).toString();
    QString country =field(pnCountry).toString();
    QString xxx = pcode +qsl(" ") +city;
    if( not country.isEmpty())
        xxx += qsl(" (") +country +qsl(")");
    QString email  =field(pnEMail).toString().isEmpty()     ? qsl("(keine E-Mail Adresse)") :field(pnEMail).toString();
    QString comment=field(pnComment).toString().isEmpty()   ? qsl("(keine Anmerkung)"): field(pnComment).toString();
    QString iban   =field(pnIban).toString().isEmpty()      ? qsl("(keine IBAN)")     : field(pnIban).toString();
    QString bic    =field(pnBic).toString().isEmpty()       ? qsl("(keine BIC)")      : field(pnBic).toString();

    setSubTitle(creditorSummary.arg(firstn, lastn, street, xxx,
                                    email, comment, iban, bic));
    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    if( wiz->selectCreateContract)
        cbCreateContract->setChecked(true);
    else
        cbCreateContract->setChecked(false);
}
bool wpConfirmCreditor::validatePage()
{   LOG_CALL;
    creditor c;
    if( field(pnConfirmCreditor).toBool()) {
        c.setFirstname( field(pnFName).toString());
        c.setLastname(  field(pnLName).toString());
        c.setStreet(    field(pnStreet).toString());
        c.setPostalCode(field(pnPcode).toString());
        c.setCity(      field(pnCity).toString());
        c.setCountry(   field(pnCountry).toString());
        c.setEmail(     field(pnEMail).toString());
        c.setComment(   field(pnComment).toString());
        c.setIban(      field(pnIban).toString());
        c.setBic(       field(pnBic).toString());
    } else {
        QMessageBox::information(this, qsl("Bestätigung fehlt"), qsl("Du kannst die Richtigkeit der Daten bestätigen oder die Dialogfolge abbrechen."));
        return false;
    }
    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    if( wiz->inUpdateMode()){
        if( wiz) {
            c.setId(wiz->creditorId);
            return (c.update() >0);
        }
    } else {
        QString errorText;
        if( c.isValid(errorText))
            wiz->creditorId =c.save();
        if( wiz->creditorId == -1) {
            QMessageBox::critical(this, qsl("Fehler"), qsl("Der Kreditor konnte nicht gespeichert werden. ") + errorText);
            return false;
        } else {
            qInfo() << "successfully created creditor " << c.id();
            return true;
        }
    }
    Q_ASSERT("one should never come here");
    return false;
}
int wpConfirmCreditor::nextId() const
{   LOG_CALL;
    if( field(pnCreateContract).toBool())
        return page_label_and_amount;
    else
        return -1;
}
void wpConfirmCreditor::onConfirmCreateContract_toggled(int state)
{   LOG_CALL;
    qInfo() << "onConfirmCreateContract..." << state;
    setFinalPage( not state);
}

/*
 * CONTRACT CREATION UI STARTS HERE
 *
 * wizNewContractData - ask basic contract data
*/
const QString pnLabel  {qsl("label")};
const QString pnAmount {qsl("amount")};
const QString pnContComment {qsl("contractComment")};

wpLableAndAmount::wpLableAndAmount(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Vertragsdaten"));

    QLabel* l1 =new QLabel(qsl("Kennung"));
    QLineEdit* leKennung =new QLineEdit;
    leKennung->setToolTip(qsl("Über die Kennung wird der Vertrag z.B. in einem Schriftwechsel identifiziert."));
    registerField(pnLabel, leKennung);
    l1->setBuddy(leKennung);

    QLabel* l2 =new QLabel(qsl("Betrag"));
    QLineEdit* leAmount =new QLineEdit;
    leAmount->setToolTip(qsl("Der Kreditbetrag muss größer als ") + dbConfig::readValue(MIN_AMOUNT).toString() + qsl("Euro sein"));
    registerField(pnAmount, leAmount);
    leAmount->setValidator(new QIntValidator(this));
    l2->setBuddy(leAmount);

    QLabel* l3 =new QLabel(qsl("Anmerkung"));
    QPlainTextEdit* eComment =new QPlainTextEdit;
    registerField(pnContComment, eComment, "plainText", "textChanged");
    eComment->setToolTip(qsl("Diese Anmerkung wird mit dem Vertrag gespeichert"));

    QGridLayout* g =new QGridLayout;
    g->addWidget(l1, 0, 0);
    g->addWidget(leKennung, 0, 1);
    g->addWidget(l2, 1, 0);
    g->addWidget(leAmount, 1, 1);
    g->addWidget(l3, 2, 0);
    g->addWidget(eComment, 2, 1);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 4);

    setLayout(g);
}
void wpLableAndAmount::initializePage()
{   LOG_CALL;
    QString creditorInfo { qsl("%1, %2<p><small>%3 %4 %5</small>")};
    setSubTitle(creditorInfo.arg(field(pnLName).toString(),
                                 field(pnFName).toString(), field(pnStreet).toString(),
                                 field(pnPcode).toString(), field(pnCity).toString()));
    setField(pnLabel, proposeContractLabel());
}
void wpLableAndAmount::cleanupPage()
{
    // a back from here can only happen, if a existing creditor was selected
    // we have to clean up all the creditor data in case the user wants to create a new one
    setField(pnFName, QString());
    setField(pnLName, QString());
    setField(pnStreet, QString());
    setField(pnPcode, QString());
    setField(pnCity, QString());
    setField(pnCountry, QString());
    setField(pnEMail, QString());
    setField(pnComment, QString());
    setField(pnIban, QString());
    setField(pnBic, QString());
}
bool wpLableAndAmount::validatePage()
{   LOG_CALL;
    QString msg;
    setField(pnLabel, field(pnLabel.trimmed()));
    int minContractValue = dbConfig::readValue(MIN_AMOUNT).toInt();
    const QString label =field(pnLabel).toString();
    if( label.isEmpty())
        msg =qsl("Die Vertragskennung darf nicht leer sein");
    else if( not isValidNewContractLabel(label))
        msg =qsl("Die Vertragskennung existiert bereits für einen laufenden oder beendeten Vertrag und darf nicht erneut vergeben werden");
    else if( field(pnAmount).toInt() < minContractValue)
        msg =qsl("Der Wert des Vertrags muss größer sein als der konfigurierte Minimalwert "
                 "eines Vertrages von ") +dbConfig::readValue(MIN_AMOUNT).toString() +qsl(" Euro");
    if( not msg.isEmpty()) {
        QMessageBox::critical(this, qsl("Fehler"), msg);
        return false;
    }
    return true;
}
int wpLableAndAmount::nextId() const
{   LOG_CALL;
    return page_contract_timeframe;
}

/*
 * wpContractTimeframe - contract date, notice period, termination date
*/
const QString pnCDate {qsl("startD")};
const QString pnEDate {qsl("endD")};
const QString pnPeriod{qsl("noticePeriod")};

wpContractTimeframe::wpContractTimeframe(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Vertragsbeginn und -Ende"));
    setSubTitle(qsl("Für das Vertragsende kann eine Kündigungsfrist <b>oder</b> ein festes Vertragsende vereinbart werden."));
    QDateEdit* deCDate =new QDateEdit(QDate::currentDate());
    registerField(pnCDate, deCDate);
    deCDate->setDisplayFormat("dd.MM.yyyy");
    QLabel* l1 =new QLabel(qsl("Vertragsdatum"));
    l1->setBuddy(deCDate);

    cbNoticePeriod =new QComboBox;
    registerField(pnPeriod, cbNoticePeriod);
    cbNoticePeriod->addItem(qsl("festes Vertragsende"), QVariant(-1));
    for (int i=3; i<12; i++)
        cbNoticePeriod->addItem(QString::number(i) + qsl(" Monate"), QVariant(i));
    cbNoticePeriod->addItem(qsl("1 Jahr"), QVariant(12));
    cbNoticePeriod->addItem(qsl("1 Jahr und 1 Monat"), QVariant(13));
    for (int i=14; i<24; i++)
        cbNoticePeriod->addItem(qsl("1 Jahr und ") + QString::number( i-12) + qsl(" Monate"), QVariant(i));
    cbNoticePeriod->addItem(qsl("2 Jahre"), QVariant(24));
    connect(cbNoticePeriod, SIGNAL(currentIndexChanged(int)), this, SLOT(onNoticePeriod_currentIndexChanged(int)));
    cbNoticePeriod->setToolTip(qsl("Wird eine Kündigungsfrist vereinbart, "
                                   "so gibt es kein festes Vertragsende. "
                                   "Um ein festes Vertragsende einzugeben wähle "
                                   "in dieser Liste \"festes Vertragsende\""));
    QLabel* l2 =new QLabel(qsl("Kündigungsfrist"));
    l2->setBuddy(cbNoticePeriod);

    deTerminationDate =new QDateEdit;
    registerField(pnEDate, deTerminationDate);
    deTerminationDate->setDisplayFormat("dd.MM.yyyy");
    QLabel* l3 =new QLabel(qsl("Vertragsende"));
    l3->setBuddy(deTerminationDate);

    QGridLayout* g =new QGridLayout;
    g->addWidget(l1, 0, 0);
    g->addWidget(deCDate, 0, 1);
    g->addWidget(l2, 1, 0);
    g->addWidget(cbNoticePeriod, 1, 1);
    g->addWidget(l3, 2, 0);
    g->addWidget(deTerminationDate, 2, 1);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 4);
    setLayout(g);
}
void wpContractTimeframe::initializePage()
{   LOG_CALL;
    setField(pnPeriod, 4);
    setField(pnEDate, EndOfTheFuckingWorld);
    deTerminationDate->setEnabled(false);
}
void wpContractTimeframe::onNoticePeriod_currentIndexChanged(int i)
{   LOG_CALL;
    if( i == 0) {
        // deTerminationDate->setDate(QDate::currentDate().addYears(5));
        setField(pnEDate, QDate::currentDate().addYears(5));
        deTerminationDate->setEnabled(true);
    } else {
        //deTerminationDate->setDate(EndOfTheFuckingWorld);
        setField(pnEDate, EndOfTheFuckingWorld);
        deTerminationDate->setEnabled(false);
    }
}
bool wpContractTimeframe::validatePage()
{   LOG_CALL;
    QDate start =field(pnCDate).toDate();
    QDate end   =field(pnEDate).toDate();
    if( start > end) {
        qInfo() << "Enddate is before Startdate: " << field(pnCDate).toDate() << "/" << field(pnEDate).toDate();
        return false;
    }
    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    if( wiz) {
        wiz->noticePeriod =cbNoticePeriod->itemData(field(pnPeriod).toInt()).toInt();
        //        wiz->date =start;
        //        wiz->termination =end;
        return true;
    }
    Q_ASSERT(true);
    return true;
}
int wpContractTimeframe::nextId() const
{   LOG_CALL;
    if( nbrActiveInvestments(field(pnCDate).toDate()))
        return page_interest_selection_Mode;
    else
        return page_interest_value_selection;
}

/*
 * wpInterestSelectionMode - ask: investment or interest?
*/
const auto pnISelMode {qsl("iSelMode")};

wpInterestSelectionMode::wpInterestSelectionMode(QWidget* w) : QWizardPage(w)
{
    setTitle(qsl("Zinssatz"));
    setSubTitle(qsl("Wähle, ob Du den Zins durch die Auswahl einer Geldanlage oder "
                    "durch die Eingabe des Zinssatzes auswählen möchtest"));
    QRadioButton* rbInvestment =new QRadioButton(qsl("Anlage auswählen"));
    rbInvestment->setToolTip(qsl("Bestimme den Zinssatz über die Auswahl einer Geldanlage"));
    QRadioButton* rbInterest   =new QRadioButton(qsl("Zinssatz auswählen"));
    rbInterest->setToolTip(qsl("Bestimme den Zinssatz direkt"));
    registerField(pnISelMode, rbInvestment);
    QVBoxLayout* vl =new QVBoxLayout();
    vl->addWidget(rbInvestment);
    vl->addWidget(rbInterest);
    setLayout(vl);
}
void wpInterestSelectionMode::initializePage()
{
    setField(pnISelMode, true);
}
int wpInterestSelectionMode::nextId() const
{
    if(field(pnISelMode).toBool())
        return page_interest_from_investment;
    else
        return page_interest_value_selection;
}

/*
 * wpInterestFromInvestment
*/
const QString pnI_CheckBoxIndex{qsl("iByInvestment")};

wpInterestFromInvestment::wpInterestFromInvestment(QWidget* w) : QWizardPage(w)
{
    setTitle(qsl("Wähle die gewünschte Geldanlage aus der Liste aus"));
    cbInvestments =new QComboBox();
    lblInvestmentInfo =new QLabel();
    QVBoxLayout* l =new QVBoxLayout();
    l->addWidget(cbInvestments);
    l->addWidget(lblInvestmentInfo);
    setLayout(l);
    // todo: on cbInvestments_change: show details in lable / table
    // interest/startd/endd
    // sum(active), count(active) / sum(inactive), count(inactive)
    // after booking:
    // sum(active), count(active) / sum(inactive), count(inactive)
    // RED if any number goes too high
    connect(cbInvestments, SIGNAL(currentIndexChanged(int)), this, SLOT(onInvestments_currentIndexChanged(int)));
}
void wpInterestFromInvestment::initializePage()
{
    QVector<QPair<qlonglong, QString>> investments =activeInvestments(field(pnCDate).toDate());
    for( auto invest : qAsConst(investments)) {
        cbInvestments->addItem(invest.second, invest.first);
    }
    registerField(pnI_CheckBoxIndex, cbInvestments);
}
void wpInterestFromInvestment::onInvestments_currentIndexChanged(int ) {
    qlonglong rowId =cbInvestments->currentData().toLongLong();
    double amount =field(pnAmount).toDouble();
    QString html =investmentInfoForContract(rowId, amount);
    lblInvestmentInfo->setText(html);
}
bool wpInterestFromInvestment::validatePage()
{
    rowid_investment = cbInvestments->currentData().toLongLong();
    wizNew* wiz =qobject_cast<wizNew*>(wizard());
    wiz->interest =interestOfInvestmentByRowId(rowid_investment);
    if( wiz->interest == 0){
        // without interest -> interest payout mode "zero"
        wiz->iPaymentMode =interestModel::zero;
    }

    return true;
}
int wpInterestFromInvestment::nextId() const
{
    wizNew* wiz =qobject_cast<wizNew*>(wizard());
    if( wiz->interest == 0)
        return page_confirm_contract;
    else
        return page_interest_payment_mode;
}

/*
 * wpNewContractInterest - interest value from Investment or comboBox, interest mode
*/
const QString pnInterestIndex {qsl("iIndex")};

wpInterestSelection::wpInterestSelection(QWidget* w) : QWizardPage(w)
{
    setTitle(qsl("Zinssatz"));
    setSubTitle(qsl("Wähle den Zinssatz aus"));

    QLabel* lZ =new QLabel(qsl("Zinssatz"));
    QComboBox* cbInterest =new QComboBox;
    lZ->setBuddy(cbInterest);
    cbInterest->addItem(qsl("Zinslos"), QVariant(0));
    for( int i =1; i <= dbConfig::readValue(MAX_INTEREST).toInt(); i++)
        cbInterest->addItem(QString::number(double(i)/100., 'f', 2), QVariant(i));
    cbInterest->setCurrentIndex(qMin(90, cbInterest->count()));
    registerField(pnInterestIndex, cbInterest);
    QHBoxLayout* hbl =new QHBoxLayout();
    hbl->addWidget(lZ);
    hbl->addWidget(cbInterest);
    setLayout(hbl);
}
bool wpInterestSelection::validatePage()
{
    // without interest -> interest payout mode "payout"
    wizNew* wiz =qobject_cast<wizNew*>(wizard());
    if( field(pnInterestIndex) == 0)
        wiz->iPaymentMode =interestModel::zero;
    else
        wiz->iPaymentMode =interestModel::payout; // default for the next wiz page
    return true;
}
int wpInterestSelection::nextId() const
{
    wizNew* wiz =qobject_cast<wizNew*>(wizard());
    wiz->interest =field(pnInterestIndex).toInt();
    if( wiz->interest == 0)
        return page_confirm_contract;
    else
        return page_interest_payment_mode;
}

/*
 * wpInterestMode - kein Zins? thesaurierend? Auszahlend? Fix?
*/
const QString pnIMode {qsl("imode")};
wpInterestPayoutMode::wpInterestPayoutMode(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Zinsmodus"));
    setSubTitle(qsl("Wähle den Zinsmodus für diesen Vertrag"));

    cbImode =new QComboBox();
    cbImode->addItem("Zinsen werden ausgezahlt");
    cbImode->setItemData(0, toInt(interestModel::payout));
    cbImode->addItem("Zinsen werden angespart und verzinst");
    cbImode->setItemData(1, toInt(interestModel::reinvest));
    cbImode->addItem("Zinsen werden angespart aber nicht verzinst");
    cbImode->setItemData(2, toInt(interestModel::fixed));

    cbImode->setToolTip("Bei thesaurierenden Verträgen erfolgt keine jährliche Auszahlung der Zinsen."
                            " Die Zinsen werden dem Kreditbetrag hinzugerechnet und in Folgejahren mit verzinst.");
    cbImode->setCurrentIndex(1);
    registerField(pnIMode, cbImode);
    QVBoxLayout* vbl =new QVBoxLayout();
    vbl->addWidget(cbImode);
    setLayout(vbl);
}
bool wpInterestPayoutMode::validatePage()
{
    wizNew* wiz =qobject_cast<wizNew*>(wizard());
    wiz->iPaymentMode =fromInt(cbImode->itemData(field(pnIMode).toInt()).toInt());
    return true;
}
int wpInterestPayoutMode::nextId() const
{
    return page_confirm_contract;
}

/*
* wizConfirmContract  -confirm the contract data before contract creation
*/
const QString pnConfirmContract {qsl("confirmContract")};

bool wpConfirmContract::saveContract()
{
    wizNew* wiz =qobject_cast<wizNew*>(wizard());
    if( not wiz)  return false;
    if( wiz->field(pnConfirmContract).toBool()) {
        contract c;
        c.setCreditorId(wiz->creditorId);
        c.setPlannedInvest(field(pnAmount).toDouble());
        c.setInterestRate(wiz->interest/100.);
        c.setLabel(field(pnLabel).toString());
        c.setConclusionDate(field(pnCDate).toDate());
        c.setNoticePeriod(wiz->noticePeriod);
        c.setPlannedEndDate(field(pnEDate).toDate());
        c.setInterestModel(wiz->iPaymentMode);
        c.setComment(field(pnContComment).toString());
        if( -1 == c.saveNewContract()) {
            qCritical() << "New contract could not be saved";
            QMessageBox::critical(getMainWindow(), "Fehler", "Der Vertrag konnte nicht "
                            "gespeichert werden. Details findest Du in der Log Datei");
            return false;
        } else {
            qInfo() << "New contract successfully saved";
            return true;
        }
    } else {
        qCritical() << "user did not confirm contract data";
        Q_ASSERT(true);
        return false;
    };
}
wpConfirmContract::wpConfirmContract(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Bestätige die Vertragsdaten"));
    QCheckBox* cbConfirm =new QCheckBox(qsl("Die Angaben sind korrekt!"));
    cbConfirm->setCheckState(Qt::CheckState::Unchecked);
    registerField(pnConfirmContract +qsl("*"), cbConfirm);
    QVBoxLayout* bl =new QVBoxLayout;
    bl->addWidget(cbConfirm);
    setLayout(bl);
//    connect(cbConfirm, SIGNAL(stateChanged(int)), this, SLOT(onConfirmData_toggled(int)));
    setCommitPage(true);
}
void wpConfirmContract::initializePage()
{   LOG_CALL;

    QString summary {qsl("Vertrag <b>%3</b> von <b>%1 %2</b> <p><table>"
                         //"<tr><td>Kennung: </td><td><b>%3</b><p></td></tr>"
                         "<tr><td>Betrag: </td><td><b>%4</b></td></tr>"
                         "<tr><td>Zinssatz: </td><td><b>%5 %</b></td></tr>"
                         "<tr><td>Zinsanrechnung: </td><td><b>%6<p></b></td></tr>"
                         "<tr><td>Abschlußdatum: </td><td><b>%7</b></td></tr>"
                         "<tr><td>Vertragsende: </td><td><b>%8</b></td></tr>"
                         "</table>")};
    QLocale l;


    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    if( wiz)
    {
        interestModel iMode{wiz->iPaymentMode};
        QString interestMode =toString(iMode);
        setSubTitle(summary.arg(field(pnFName).toString(), field(pnLName).toString(),
                                field(pnLabel).toString(),
                                l.toCurrencyString(field(pnAmount).toDouble()),
                                QString::number(wiz->interest/100., 'f', 2),
                                interestMode,
                                field(pnCDate).toDate().toString("dd.MM.yyyy"),
                                (wiz->noticePeriod == -1) ?
                                    wiz->field(pnEDate).toDate().toString(qsl("dd.MM.yyyy")) :
                                    QString::number(wiz->noticePeriod) +qsl(" Monate nach Kündigung")));
    } else Q_ASSERT(true);
}
bool wpConfirmContract::validatePage()
{   LOG_CALL;
    // we only get here, if finish was enabled -> we do not have to check
    // the checkbox. user can only cancel if checkbox not checked
    return saveContract();
}
//void wpConfirmContract::onConfirmData_toggled(int i)
//{ LOG_CALL;
//    completeChanged();
//}
//bool wpConfirmContract::isComplete() const
//{
//    return field(pnConfirmCreditor).toBool();
//}


/*
* wizNew - the wizard containing the pages to create a cerditor and a contract
*/
wizNew::wizNew(QWidget *p) : QWizard(p)
{   LOG_CALL;
    // setOption(QWizard::IndependentPages);
    setPage(page_new_or_existing, new wpNewOrExisting(this));
    setPage(page_address, new wpNewCreditorAddress(this));
    setPage(page_email,   new wpEmail(this));
    setPage(page_bankaccount, new wpBankAccount(this));
    setPage(page_confirm_creditor, new wpConfirmCreditor(this));
    setPage(page_label_and_amount, new wpLableAndAmount(this));
    setPage(page_interest_selection_Mode, new wpInterestSelectionMode(this));
    setPage(page_interest_from_investment, new wpInterestFromInvestment(this));
    setPage(page_interest_value_selection, new wpInterestSelection(this));
    setPage(page_interest_payment_mode, new wpInterestPayoutMode(this));
    setPage(page_contract_timeframe, new wpContractTimeframe(this));
    setPage(page_confirm_contract, new wpConfirmContract(this));
    QFont f = font(); f.setPointSize(10); setFont(f);
}
