#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "helper.h"
#include "creditor.h"
#include "wiznew.h"

wizNewOrExistingPage::wizNewOrExistingPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertrag anlegen"));
    setSubTitle(qsl("Mit dieser Dialogfolge kannst Du einen Kredigeber*in und Vertrag anlegen."));
    rbNew =new QRadioButton(qsl("Neuen Kreditgeber*in anlegen"));
    rbExisting =new QRadioButton(qsl("Existierenden Kreditgeber*in verwenden"));
    registerField(qsl("create_new"), rbNew);
    cbCreditors = new QComboBox();
    registerField(qsl("creditor"), cbCreditors);
    QVBoxLayout* l =new QVBoxLayout();
    l->addWidget(rbNew);
    l->addWidget(rbExisting);
    l->addWidget(cbCreditors);
    setLayout(l);
    connect(rbExisting, SIGNAL(toggled(bool)), this, SLOT(onExistingCreditor_toggled(bool)));
}
void wizNewOrExistingPage::initializePage()
{
    setField(qsl("create_new"), true);
    QList<QPair<int, QString>> Personen;
    KreditorenListeMitId(Personen);
    for(auto& Entry :qAsConst(Personen)) {
        cbCreditors->addItem( Entry.second, QVariant((Entry.first)));
    }
    cbCreditors->setEnabled(false);
}
void wizNewOrExistingPage::onExistingCreditor_toggled(bool b)
{
    cbCreditors->setEnabled(b);
}
bool wizNewOrExistingPage::validatePage()
{   LOG_CALL;

    if( field(qsl("create_new")).toBool())
        qInfo() << "User chose to create a new creditor";
    else {
        wizNew* wiz =dynamic_cast<wizNew*> (wizard());
        wiz->creditorId = cbCreditors->itemData(field("creditor").toInt()).toLongLong();
        qInfo() << "User selected user Id " << wiz->creditorId;
    }
    return true;
}
int wizNewOrExistingPage::nextId() const
{
    if( field(qsl("create_new")).toBool())
        return page_address;
    else
        return page_contract_data;
}

wizNewCreditorAddressPage::wizNewCreditorAddressPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Adresse"));
    setSubTitle(qsl("Gib die Adressdaten ein."));
    QLineEdit* leFirstName =new QLineEdit;
    registerField(qsl("firstname"), leFirstName);
    QLineEdit* leLastName  =new QLineEdit;
    registerField(qsl("lastname"),  leLastName);
    QLineEdit* leStreet    =new QLineEdit;
    registerField(qsl("street"),    leStreet);
    QLineEdit* lePlz       =new QLineEdit;
    registerField(qsl("pcode"),     lePlz);
    QLineEdit* leCity      =new QLineEdit;
    registerField(qsl("city"),     leCity);

    QLabel* l1 =new QLabel(qsl("Name"));
    QLabel* l2 =new QLabel(qsl("Straße"));
    QLabel* l3 =new QLabel(qsl("Plz/Ort"));

    QGridLayout* grid =new QGridLayout;
    grid->addWidget(l1,          0, 0, 1, 1);
    grid->addWidget(leFirstName, 0, 1, 1, 2);
    grid->addWidget(leLastName,  0, 3, 1, 2);

    grid->addWidget(l2,          1, 0, 1, 1);
    grid->addWidget(leStreet,    1, 1, 1, 4);

    grid->addWidget(l3,          2, 0, 1, 1);
    grid->addWidget(lePlz,       2, 1, 1, 1);
    grid->addWidget(leCity,      2, 2, 1, 3);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 2);
    grid->setColumnStretch(2, 2);
    grid->setColumnStretch(3, 2);
    grid->setColumnStretch(4, 2);

    setLayout(grid);
}
void wizNewCreditorAddressPage::initializePage()
{

}
bool wizNewCreditorAddressPage::validatePage()
{
    if( field(qsl("firstname")).toString().isEmpty()
            &&
        field(qsl("lastname")).toString().isEmpty())
    {
        return false;
    }
    return true;
}
int wizNewCreditorAddressPage::nextId() const
{
    return page_email;
}

wizEmailPage::wizEmailPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("E-mail"));
    setSubTitle(qsl("Gib hier die E-Mail Adresse und eine Anmerkung an. Als Anmerkung könntest du den Kontakt im Projekt oder sonstige Besonderheiten speichern."));
    QLabel* l1 =new QLabel(qsl("E-mail"));
    QLineEdit* leEmail =new QLineEdit;
    registerField(qsl("email"), leEmail);
    QLabel* l2 =new QLabel(qsl("Anmerkung"));
    QPlainTextEdit* eComment     =new QPlainTextEdit;
    registerField(qsl("comment"), eComment);
    QGridLayout* g =new QGridLayout;
    g->addWidget(l1,      0, 0, 1, 1);
    g->addWidget(leEmail, 0, 1, 1, 4);
    g->addWidget(l2,      1, 0, 1, 1);
    g->addWidget(eComment, 1, 1, 3, 4);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 2);
    g->setColumnStretch(2, 2);
    g->setColumnStretch(3, 2);
    g->setColumnStretch(4, 2);
    setLayout(g);
}

wizNewContractData::wizNewContractData(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertragsdaten"));
}

wizNew::wizNew(QWidget *p) : QWizard(p)
{
    setPage(page_new_or_existing, new wizNewOrExistingPage(p));
    setPage(page_address, new wizNewCreditorAddressPage(p));
    setPage(page_email,   new wizEmailPage(p));
    setPage(page_contract_data, new wizNewContractData(p));
}
