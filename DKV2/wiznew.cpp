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
#include "contract.h"
#include "wiznew.h"

/*
 * wizNewOrExistingPage - ask user if a new user should be created or an existing
 * one should be used
*/
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
    registerField(qsl("create_new"), rbNew);
    cbCreditors = new QComboBox();
    cbCreditors->setToolTip(qsl("Wähle hier den Kreditgeber*in aus, für den der Vertrag angelegt werden soll."));
    registerField(qsl("creditor"), cbCreditors);
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
        setField(qsl("create_new"), true);
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

    if( field(qsl("create_new")).toBool())
        qInfo() << "User chose to create a new creditor";
    else {
        wizNew* wiz =qobject_cast<wizNew*> (wizard());
        wiz->creditorId = cbCreditors->itemData(field("creditor").toInt()).toLongLong();
        creditor c(wiz->creditorId);
        setField(qsl("firstname"), c.firstname());
        setField(qsl("lastname"), c.lastname());
        setField(qsl("street"), c.street());
        setField(qsl("pcode"), c.postalCode());
        setField(qsl("city"), c.city());
        setField(qsl("country"), c.country());
        setField(qsl("email"), c.email());
        setField(qsl("comment"), c.comment());
        setField(qsl("iban"), c.iban());
        setField(qsl("bic"), c.bic());
        qInfo() << "User selected user Id " << wiz->creditorId;
    }
    return true;
}
int wpNewOrExisting::nextId() const
{   LOG_CALL;
    if( field(qsl("create_new")).toBool())
        return page_address;
    else
        return page_contract_data;
}

/*
 * wizNewCreditorAddressPage - ask address data for the new creditor
*/
wpNewCreditorAddress::wpNewCreditorAddress(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Adresse"));
    setSubTitle(qsl("Gib hier Namen und die Adressdaten ein."));
    QLineEdit* leFirstName =new QLineEdit;
    leFirstName->setToolTip(qsl("Die Felder für Vor- und Nachname dürfen nicht beide leer sein"));
    registerField(qsl("firstname"), leFirstName);
    QLineEdit* leLastName =new QLineEdit;
    registerField(qsl("lastname"), leLastName);
    leLastName->setToolTip(qsl("Die Felder für Vor- und Nachname dürfen nicht beide leer sein"));
    QLineEdit* leStreet =new QLineEdit;
    leStreet->setToolTip(qsl("Die Felder Straße, Postleitzahl und Stadt dürfen nicht leer sein"));
    registerField(qsl("street"), leStreet);
    QLineEdit* lePlz =new QLineEdit;
    lePlz->setToolTip(qsl("Die Felder Straße, Postleitzahl und Stadt dürfen nicht leer sein"));
    registerField(qsl("pcode"), lePlz);
    QLineEdit* leCity =new QLineEdit;
    leCity->setToolTip(qsl("Die Felder Straße, Postleitzahl und Stadt dürfen nicht leer sein"));
    registerField(qsl("city"), leCity);
    QLineEdit* leCountry =new QLineEdit;
    leCountry->setToolTip(qsl("Trage hier ein Land ein, falls die Kreditor*in nicht in Deutschland wohnt"));
    registerField(qsl("country"), leCountry);

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
bool wpNewCreditorAddress::validatePage()
{   LOG_CALL;
    setField(qsl("firstname"), field(qsl("firstname")).toString().trimmed());
    setField(qsl("lastname"),  field(qsl("lastname")) .toString().trimmed());
    setField(qsl("street"),    field(qsl("street"))   .toString().trimmed());
    setField(qsl("pcode"),     field(qsl("pcode"))    .toString().trimmed());
    setField(qsl("city"),      field(qsl("city"))     .toString().trimmed());
    setField(qsl("country"),   field(qsl("country"))  .toString().trimmed());

    if( field(qsl("firstname")).toString().isEmpty()
            &&
        field(qsl("lastname")).toString().isEmpty())
        return false;
    if( field(qsl("street")).toString().isEmpty() ||
            field(qsl("pcode")).toString().isEmpty() ||
            field(qsl("city")).toString().isEmpty())
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
wpEmail::wpEmail(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("E-mail"));
    setSubTitle(qsl("Gib hier die E-Mail Adresse und eine Anmerkung an."));

    QLabel* l1 =new QLabel(qsl("E-mail"));
    QLineEdit* leEmail =new QLineEdit;
    leEmail->setToolTip(qsl("Die E-Mail Adresse muss im gültigen Format vorliegen "
                            "(s. https://de.wikipedia.org/wiki/E-Mail-Adresse)."));
    registerField(qsl("email"), leEmail);
    QLabel* l2 =new QLabel(qsl("Anmerkung"));
    QPlainTextEdit* eComment     =new QPlainTextEdit;
    registerField(qsl("comment"), eComment, "plainText", "textChanged");
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
    QString email =field(qsl("email")).toString().trimmed().toLower();
    setField(qsl("email"), email);
    if( ! email.isEmpty())
    {
        QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                              QRegularExpression::CaseInsensitiveOption);
        if( !rx.match(field(qsl("email")).toString()).hasMatch()) {
            QMessageBox::information(this, qsl("Fehler"), qsl("Das Format der e-mail Adresse ist ungültig: ") + email);
            return false;
        }
    }
    setField(qsl("comment"), field(qsl("comment")).toString().trimmed());
    return true;
}
int wpEmail::nextId() const
{   LOG_CALL;
    return page_bankaccount;
}

/*
 * wizEmailPage - ask bank account data
*/
wpBankAccount::wpBankAccount(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Bankdaten"));
    setSubTitle(qsl("Gib die Bankdaten der neuen Kreditor*in ein."));
    QLabel* l1 =new QLabel(qsl("IBAN"));
    QLineEdit* leIban =new QLineEdit;
    leIban->setToolTip(qsl("Es muss eine gültige IBAN eingegeben werden "
                           "(s. https://de.wikipedia.org/wiki/Internationale_Bankkontonummer)."));
    l1->setBuddy(leIban);
    registerField(qsl("iban"), leIban);
    QLabel* l2 =new QLabel(qsl("BIC <small>(optional)</small>"));
    QLineEdit* leBic =new QLineEdit;
    l2->setBuddy(leBic);
    registerField(qsl("bic"), leBic);

    QGridLayout* g =new QGridLayout;
    g->addWidget(l1);
    g->addWidget(leIban);
    g->addWidget(l2);
    g->addWidget(leBic);
    setLayout(g);
}
bool wpBankAccount::validatePage()
{   LOG_CALL;
    QString iban =field(qsl("iban")).toString().trimmed();
    setField(qsl("iban"), iban);
    if( ! iban.isEmpty()) {
        IbanValidator iv; int pos =0;
        if( iv.validate(iban, pos) != IbanValidator::State::Acceptable) {
            QMessageBox::information(this, "Fehler", "Die Iban ist ungültig");
            return false;
        }
    }
    setField(qsl("bic"), field(qsl("bic")).toString().trimmed());
    return true;
}
int wpBankAccount::nextId() const
{   LOG_CALL;
    return page_confirm_creditor;
}

/*
 * wizConfirmCreditorData : ask if creditor data is OK and if a contract should be created
*/
wpConfirmCreditor::wpConfirmCreditor(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Daten bestätigen"));
    QCheckBox* cbConfirmCreditor =new QCheckBox(qsl("Die Angaben sind richtig!"));
    cbConfirmCreditor->setToolTip(qsl("Du musst die Daten prüfen und ihre Richtigkeit "
                                      "bestätigen um sie speichern zu können."));
    registerField(qsl("confirmCreditor"), cbConfirmCreditor);
    cbConfirmCreditor->setChecked(false);
    cbCreateContract =new QCheckBox(qsl("Im Anschluß Vertragsdaten eingeben"));
    cbCreateContract->setToolTip(qsl("Selektiere dieses Feld, um gleich einen neuen Vertrag anzulegen"));
    registerField(qsl("confirmCreateContract"), cbCreateContract);
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
    QString firstn =field(qsl("firstname")).toString().isEmpty() ? qsl("(kein Vorname)")  : field(qsl("firstname")).toString();
    QString lastn  =field(qsl("lastname")).toString().isEmpty()  ? qsl("(kein Nachname)") : field(qsl("lastname")).toString();
    QString street =field(qsl("street")).toString().isEmpty()    ? qsl("(keine Strasse)") : field(qsl("street")).toString();

    QString pcode  =field(qsl("pcode")).toString().isEmpty()     ? qsl("(keine PLZ)")     : field(qsl("pcode")).toString();
    QString city   =field(qsl("city")).toString().isEmpty()      ? qsl("(keine Stadt)")   : field(qsl("city")).toString();
    QString country =field(qsl("country")).toString();
    QString xxx = pcode +qsl(" ") +city;
    if(!country.isEmpty())
        xxx += qsl(" (") +country +qsl(")");
    QString email  =field(qsl("email")).toString().isEmpty()     ? qsl("(keine E-Mail Adresse)") :field(qsl("email")).toString();
    QString comment=field(qsl("comment")).toString().isEmpty()   ? qsl("(keine Anmerkung)"): field(qsl("comment")).toString();
    QString iban   =field(qsl("iban")).toString().isEmpty()      ? qsl("(keine IBAN)")     : field(qsl("iban")).toString();
    QString bic    =field(qsl("bic")).toString().isEmpty()       ? qsl("(keine BIC)")      : field(qsl("bic")).toString();

    setSubTitle(creditorSummary.arg(firstn, lastn, street, xxx,
                                    email, comment, iban, bic));
    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    if( wiz)
        cbCreateContract->setChecked(true);
    else
        cbCreateContract->setChecked(false);
}
bool wpConfirmCreditor::validatePage()
{   LOG_CALL;
    creditor c;
    if( field(qsl("confirmCreditor")).toBool()) {
        c.setFirstname( field(qsl("firstname")).toString());
        c.setLastname(  field(qsl("lastname")).toString());
        c.setStreet(    field(qsl("street")).toString());
        c.setPostalCode(field(qsl("pcode")).toString());
        c.setCity(      field(qsl("city")).toString());
        c.setCountry(   field(qsl("country")).toString());
        c.setEmail(     field(qsl("email")).toString());
        c.setComment(   field(qsl("comment")).toString());
        c.setIban(      field(qsl("iban")).toString());
        c.setBic(       field(qsl("bic")).toString());
    } else {
        QMessageBox::information(this, qsl("Bestätigung fehlt"), qsl("Du musst die Richtigkeit der Daten bestätigen oder die Dialogfolge abbrechen."));
        return false;
    }
    {
        wizNew* wiz =qobject_cast<wizNew*> (wizard());
        if( wiz) {
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
    }
    {
        wizEditCreditor* wizEdit =qobject_cast<wizEditCreditor*> (wizard());
        if( wizEdit) {
            c.setId(wizEdit->creditorId);
            return (c.update() >0);
        }
    }
    Q_ASSERT("one should never come here");
    return false;
}
int wpConfirmCreditor::nextId() const
{   LOG_CALL;
    if( field(qsl("confirmCreateContract")).toBool())
        return page_contract_data;
    else
        return -1;
}
void wpConfirmCreditor::onConfirmCreateContract_toggled(int state)
{   LOG_CALL;
    qInfo() << "onConfirmCreateContract..." << state;
    setFinalPage( ! state);
}

/*
 * wizNewContractData - ask basic contract data
*/
wpNewContractData::wpNewContractData(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Vertragsdaten"));

    QLabel* l1 =new QLabel(qsl("Kennung"));
    QLineEdit* leKennung =new QLineEdit;
    leKennung->setToolTip(qsl("Über die Kennung wird der Vertrag z.B. in einem Schriftwechsel identifiziert."));
    registerField(qsl("label"), leKennung);
    l1->setBuddy(leKennung);

    QLabel* l2 =new QLabel(qsl("Betrag"));
    QLineEdit* leAmount =new QLineEdit;
    leAmount->setToolTip(qsl("Der Kreditbetrag muss größer als ") + dbConfig::readValue(MIN_AMOUNT).toString() + qsl("Euro sein"));
    registerField(qsl("amount"), leAmount);
    leAmount->setValidator(new QIntValidator(this));
    l2->setBuddy(leAmount);

    QLabel* l3 =new QLabel(qsl("Zinssatz"));
    cbInterest =new QComboBox;
    l3->setBuddy(cbInterest);
    int maxIndex =dbConfig::readValue(MAX_INTEREST).toInt();
    for( int i =0; i <= maxIndex; i++)
        cbInterest->addItem(QString::number(double(i)/100., 'f', 2), QVariant(i));
    cbInterest->setCurrentIndex(std::min(100, cbInterest->count()));
    QLabel* l4 =new QLabel(qsl("Thesaurierend"));
    QComboBox* cbThesa =new QComboBox;
    cbThesa->addItem("Zinsen werden ausgezahlt");
    cbThesa->addItem("Zinsen werden angespart und verzinst");
    cbThesa->addItem("Zinsen werden angespart aber nicht verzinst");
    cbThesa->setToolTip("Bei thesaurierenden Verträgen erfolgt keine jährliche Auszahlung der Zinsen."
                        " Die Zinsen werden dem Kreditbetrag hinzugerechnet und in Folgejahren mit verzinst.");
    cbThesa->setCurrentIndex(1);
    registerField(qsl("thesa"), cbThesa);
    QGridLayout* g =new QGridLayout;
    g->addWidget(l1, 0, 0);
    g->addWidget(leKennung, 0, 1);
    g->addWidget(l2, 1, 0);
    g->addWidget(leAmount, 1, 1);
    g->addWidget(l3, 2, 0);
    g->addWidget(cbInterest, 2, 1);
    g->addWidget(l4, 3, 0);
    g->addWidget(cbThesa, 3, 1);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 4);

    setLayout(g);
}
void wpNewContractData::initializePage()
{   LOG_CALL;
    if( init) {
        QString creditorInfo { qsl("%1, %2<p><small>%3 %4 %5</small>")};
        setSubTitle(creditorInfo.arg(field(qsl("lastname")).toString(),
                                     field(qsl("firstname")).toString(), field(qsl("street")).toString(),
                                     field(qsl("pcode")).toString(), field(qsl("city")).toString()));
        setField(qsl("label"), proposeContractLabel());
    }
    init =false;
}
bool wpNewContractData::validatePage()
{   LOG_CALL;
    QString msg;
    if( field(qsl("label")).toString().isEmpty()) msg =qsl("Die Vertragskennung darf nicht leer sein");
    int minContractValue = dbConfig::readValue(MIN_AMOUNT).toInt();
    if( field(qsl("amount")).toInt() < minContractValue)
        msg =qsl("Der Wert des Vertrags muss größer sein als der konfigurierte Minimalwert "
                 "eines Vertrages von ") +dbConfig::readValue(MIN_AMOUNT).toString() +qsl(" Euro");
    if( ! msg.isEmpty()) {
        QMessageBox::critical(this, qsl("Fehler"), msg);
        return false;
    }
    wizNew* wiz =qobject_cast<wizNew*>( wizard());
    if( wiz) wiz->interest = double(cbInterest->currentIndex())/100.;
    wizEditCreditor* wizE =qobject_cast<wizEditCreditor*>( wizard());
    if( wizE) wizE->interest = double(cbInterest->currentIndex())/100.;
    return true;
}
int wpNewContractData::nextId() const
{   LOG_CALL;
    return page_contract_term;
}

/*
 * wizContractTiming - contract date, notice period, termination date
*/
wpContractTiming::wpContractTiming(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Vertragsbeginn und -Ende"));
    setSubTitle(qsl("Für das Vertragsende kann eine Kündigungsfrist <b>oder</b> ein festes Vertragsende vereinbart werden."));
    deCDate =new QDateEdit(QDate::currentDate());
    deCDate->setDisplayFormat("dd.MM.yyyy");
    QLabel* l1 =new QLabel(qsl("Vertragsdatum"));
    l1->setBuddy(deCDate);

    cbNoticePeriod =new QComboBox;
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
    deTerminationDate->setDisplayFormat("dd.MM.yyyy");
    QLabel* l3=new QLabel(qsl("Vertragsende"));
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
void wpContractTiming::initializePage()
{   LOG_CALL;
    if( init) {
        cbNoticePeriod->setCurrentIndex(4);
        deTerminationDate->setDate(EndOfTheFuckingWorld);
        deTerminationDate->setEnabled(false);
    }
    init =false;
}
void wpContractTiming::onNoticePeriod_currentIndexChanged(int i)
{   LOG_CALL;
    if( i == 0) {
        deTerminationDate->setDate(QDate::currentDate().addYears(5));
        deTerminationDate->setEnabled(true);
    } else {
        deTerminationDate->setDate(EndOfTheFuckingWorld);
        deTerminationDate->setEnabled(false);
    }
}
bool wpContractTiming::validatePage()
{   LOG_CALL;

    wizNew* wiz =qobject_cast<wizNew*> (wizard());
    if( wiz) {
        wiz->noticePeriod =cbNoticePeriod->itemData(cbNoticePeriod->currentIndex()).toInt();
        wiz->date =deCDate->date();
        wiz->termination =deTerminationDate->date();
        return true;
    }
    wizEditCreditor* wizE =qobject_cast<wizEditCreditor*> (wizard());
    if( wizE) {
        wizE->noticePeriod =cbNoticePeriod->itemData(cbNoticePeriod->currentIndex()).toInt();
        wizE->date =deCDate->date();
        wizE->termination =deTerminationDate->date();
        return true;
    }
    Q_ASSERT(true);
    return true;
}
int wpContractTiming::nextId() const
{   LOG_CALL;
    return page_confirm_contract;
}

/*
* wizConfirmContract  -confirm the contract data before contract creation
*/
template <typename t>
bool saveContractFromWizard(t wiz)
{
    contract c;
    c.setCreditorId(wiz->creditorId);
    c.setPlannedInvest(wiz->field(qsl("amount")).toDouble());
    c.setInterestRate(wiz->interest);
    c.setLabel(wiz->field(qsl("label")).toString());
    c.setConclusionDate(wiz->date);
    c.setNoticePeriod(wiz->noticePeriod);
    c.setPlannedEndDate(wiz->termination);
    c.setInterestModel(fromInt(wiz->field(qsl("thesa")).toInt()));
    if( -1 == c.saveNewContract()) {
        qCritical() << "New contract could not be saved";
        QMessageBox::critical(getMainWindow(), "Fehler", "Der Vertrag konnte nicht "
                                                         "gespeichert werden. Details findest Du in der Log Datei");
        return false;
    } else {
        qInfo() << "New contract successfully saved";
        return true;
    }
}
wpContractConfirmation::wpContractConfirmation(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Bestätige die Vertragsdaten"));
    QCheckBox* cbConfirm =new QCheckBox(qsl("Die Angaben sind korrekt!"));
    cbConfirm->setCheckState(Qt::CheckState::Unchecked);
    registerField(qsl("confirmContract"), cbConfirm);
    QVBoxLayout* bl =new QVBoxLayout;
    bl->addWidget(cbConfirm);
    setLayout(bl);
    connect(cbConfirm, SIGNAL(stateChanged(int)), this, SLOT(onConfirmData_toggled(int)));
}
void wpContractConfirmation::initializePage()
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
    {
        wizNew* wiz =qobject_cast<wizNew*> (wizard());
        if( wiz) {
            int iMode{field(qsl("thesa")).toInt()};
            QString interestMode{"Auszahlend"};
            if( iMode == 1) interestMode = "Thesaurierend";
            if( iMode == 2) interestMode = "Fester Zins ohne Zinsauszahlung";
            setSubTitle(summary.arg(field(qsl("firstname")).toString(), field(qsl("lastname")).toString(),
                                    field(qsl("label")).toString(),
                                    l.toCurrencyString(field(qsl("amount")).toDouble()),
                                    QString::number(wiz->interest, 'f', 2),
                                     interestMode,
                                    wiz->date.toString("dd.MM.yyyy"),
                                    (wiz->noticePeriod == -1) ?
                                        wiz->termination.toString(qsl("dd.MM.yyyy")) :
                                        QString::number(wiz->noticePeriod) +qsl(" Monate nach Kündigung")));
        }
    }
    {
        wizEditCreditor* wizE =qobject_cast<wizEditCreditor*> (wizard());
        if( wizE)
            setSubTitle(summary.arg(field(qsl("firstname")).toString(), field(qsl("lastname")).toString(),
                                    field(qsl("label")).toString(),
                                    l.toCurrencyString(field(qsl("amount")).toDouble()),
                                    QString::number(wizE->interest, 'f', 2),
                                    field(qsl("thesa")).toBool() ? qsl("thesaurierend") : qsl("auszahlend"),
                                    wizE->date.toString("dd.MM.yyyy"),
                                    (wizE->noticePeriod == -1) ?
                                                               wizE->termination.toString(qsl("dd.MM.yyyy")) :
                                                               QString::number(wizE->noticePeriod) +qsl(" Monate nach Kündigung")));
    }
    return;
}
bool wpContractConfirmation::validatePage()
{   LOG_CALL;
    // we only get here, if finish was enabled -> we do not have to check
    // the checkbox. user can only cancel if checkbox not checked
    {
        wizNew* wiz =qobject_cast<wizNew*>(wizard());
        if( wiz)
            return saveContractFromWizard(wiz);
    }
    {
        wizEditCreditor* wiz =qobject_cast<wizEditCreditor*>(wizard());
        if( wiz)
            return saveContractFromWizard(wiz);
    }
    Q_ASSERT(true);
    return false;
}
void wpContractConfirmation::onConfirmData_toggled(int )
{ LOG_CALL;
    completeChanged();
}
bool wpContractConfirmation::isComplete() const
{
    return field("confirmContract").toBool();
}


/*
* wizNew - the wizard containing the pages to create a cerditor and a contract
*/
wizNew::wizNew(QWidget *p) : QWizard(p)
{   LOG_CALL;
    setPage(page_new_or_existing, new wpNewOrExisting(this));
    setPage(page_address, new wpNewCreditorAddress(this));
    setPage(page_email,   new wpEmail(this));
    setPage(page_bankaccount, new wpBankAccount(this));
    setPage(page_confirm_creditor, new wpConfirmCreditor(this));
    setPage(page_contract_data, new wpNewContractData(this));
    setPage(page_contract_term, new wpContractTiming(this));
    setPage(page_confirm_contract, new wpContractConfirmation(this));
    QFont f = font(); f.setPointSize(10); setFont(f);
}

wizEditCreditor::wizEditCreditor(QWidget *p) : QWizard(p)
{   LOG_CALL;
    setPage(page_address, new wpNewCreditorAddress(this));
    setPage(page_email,   new wpEmail(this));
    setPage(page_bankaccount, new wpBankAccount(this));
    setPage(page_confirm_creditor, new wpConfirmCreditor(this));
    setPage(page_contract_data, new wpNewContractData(this));
    setPage(page_contract_term, new wpContractTiming(this));
    setPage(page_confirm_contract, new wpContractConfirmation(this));
    QFont f = font(); f.setPointSize(10); setFont(f);
}
