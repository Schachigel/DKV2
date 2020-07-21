#include <QVBoxLayout>

#include "helper.h"
#include "creditor.h"
#include "wiznew.h"

wizNewOrExistingPage::wizNewOrExistingPage(QWidget* p) : QWizardPage(p)
{
    setTitle(qsl("Vertrag anlegen"));
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

wizNew::wizNew(QWidget *p) : QWizard(p)
{
    setPage(page_new_or_existing, new wizNewOrExistingPage(p));
}
