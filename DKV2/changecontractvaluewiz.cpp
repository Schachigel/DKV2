
#include <QLabel>
#include <QVBoxLayout>
#include <QRadioButton>

#include "changecontractvaluewiz.h"

QWizardPage* createIntroPage()
{
    QWizardPage* p = new QWizardPage;
    p->setTitle("Ein- / Auszahlungen in einen bestehenden Vertrag");
    QLabel* label = new QLabel("Soll eine Einzahlung oder Auszahlung gemacht werden?");
    label->setWordWrap(true);
    QRadioButton* rb1 = new QRadioButton("Einzahlung");
    QRadioButton* rb2 = new QRadioButton("Auszahlung");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(rb1);
    layout->addWidget(rb2);
    p->setLayout(layout);
    return p;
}

bool changeContractValueW(qlonglong /*contractId*/)
{
    QWizard wiz;
    wiz.addPage(createIntroPage());
//    wiz.addPage(createAmountPage());
//    wiz.addPage(createSummaryPage());
    wiz.setWindowTitle("Ein- / Auszahlung");
    return true;
}
