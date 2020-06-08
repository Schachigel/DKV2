#ifndef CHANGECONTRACTVALUEWIZ_H
#define CHANGECONTRACTVALUEWIZ_H

#include <QVariant>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWizard>

class wizChangeContract_IntroPage : public QWizardPage
{
public:
    wizChangeContract_IntroPage(QWidget* parent = nullptr) : QWizardPage(parent)
    {
        setTitle("Ein- / Auszahlungen");
        setSubTitle("In dieser Dialogfolge kannst Du Ein- oder Auszahlungen zu einem Vertrag verbuchen");
        QLabel* label = new QLabel("Soll eine Einzahlung oder Auszahlung gemacht werden?");
        label->setWordWrap(true);
        QRadioButton* rb1 = new QRadioButton("Einzahlung");
        QRadioButton* rb2 = new QRadioButton("Auszahlung");

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->addWidget(rb1);
        layout->addWidget(rb2);
        setLayout(layout);
        registerField("deposit", rb1);
    }
};

struct wizChangeContract_AmountPage : public QWizardPage
{
    wizChangeContract_AmountPage(QWidget* parent = nullptr) : QWizardPage(parent)
    {
        bool deposit = field("deposit").toBool();
        setTitle(deposit ? "Einzahlungsbetrag" : "Auszahlungsbetrag");
        setSubTitle("In dieser Dialogfolge kannst Du Ein- oder Auszahlungen zu einem Vertrag verbuchen");
        QVBoxLayout*  layout = new QVBoxLayout;
        QLabel* l = new QLabel( (deposit) ?
              "Wie hoch ist der Einzahlungsbetrag in Euro" :
              "Wie hoch ist der Auszahlungsbetrag in Euro");
        l->setWordWrap(true);
        layout->addWidget(l);
        QLineEdit* le = new QLineEdit;
        layout->addWidget(le);
        setLayout(layout);
        registerField("amount", le);
    }

};

struct wizChangeContractWiz : public QWizard
{
    wizChangeContractWiz(QWidget* p =nullptr) : QWizard(p)
    {
        addPage(new wizChangeContract_IntroPage);
        addPage(new wizChangeContract_AmountPage);
    }
};

bool changeContractValueWiz(qlonglong contractId);

#endif // CHANGECONTRACTVALUEWIZ_H
