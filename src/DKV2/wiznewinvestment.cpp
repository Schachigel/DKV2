#include "pch.h"

#include "helper.h"
#include "appconfig.h"

#include "wiznewinvestment.h"

/* ////////////////////////////
  wpInvestmentSummary
*/
wpInvestmentSummary::wpInvestmentSummary(QWidget* w) : QWizardPage(w)
{   LOG_CALL;
    setTitle(qsl("Zusammenfassung"));
    subTitleLabel->setText(qsl("Bitte prüfe die Daten"));

    QCheckBox* cb =new QCheckBox(qsl("Die Angaben sind korrekt"));
    registerField(pnKorrekt+qsl("*"), cb);

    QVBoxLayout* vl =new QVBoxLayout();
    vl->addWidget(subTitleLabel);
    vl->addWidget(cb);
    setLayout(vl);
}
void wpInvestmentSummary::initializePage()
{
    QString msg {qsl("Bitte prüfe die Eingaben:<p>"
                     "<table><tr><td>Zinssatz:</td><td>%1 %</td></tr>"
                     "<tr><td>Beginn:</td><td>%2</td></tr>"
                     "<tr><td>Ende:</td><td>%3</td></tr>"
                     "<tr><td>Typ:</td><td>%4</td></tr></table>")};
    msg =msg.arg(QString::number(field(pnZSatz).toInt()/100.));

    QDate von =field(pnVon).toDate();
    QDate bis =field(pnBis).toDate ();
    if( bis == EndOfTheFuckingWorld) {
        msg =msg.arg(qsl("-"));
        msg =msg.arg(qsl("(fortlaufend)"));
        msg =msg.arg(field(pnTyp).toString());
    } else {
        msg =msg.arg(von.toString(qsl("dd.MM.yyyy")));
        msg =msg.arg(bis.toString(qsl("dd.MM.yyyy")));
        msg =msg.arg(field(pnTyp).toString());
    }
    subTitleLabel->setText(msg);
    subTitleLabel->setWordWrap(true);
}
/* ////////////////////////////
  wpType
*/
wpType::wpType(QWidget* w) : QWizardPage(w)
{   LOG_CALL;
    setTitle(qsl("Titel/Name der Anlage"));
    subTitleLabel->setText(qsl("Gib einen verständlichen Namen für diese Art der Anlage ein"));
    subTitleLabel->setWordWrap(true);
    QLabel *l = new QLabel(qsl("Anlage Typ"));
    l->setToolTip(qsl("Kurzbeschreibung der Anlage"));
    QLineEdit* le =new QLineEdit();
    l->setBuddy(le);
    le->setToolTip(l->toolTip());
    registerField(pnTyp, le);
    QHBoxLayout *lh = new QHBoxLayout();
    lh->addWidget(l);
    lh->addWidget(le);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(subTitleLabel);
    layout->addLayout(lh);
    setLayout(layout);
}
void wpType::initializePage()
{   LOG_CALL_W(qsl("wpType"));
    QString prop {qsl("%1; max. 100.000")};
    if(field(pnBis).toDate () == EndOfTheFuckingWorld)
        setField(pnTyp, prop.arg(qsl("(Fortlaufend)")));
    else
        setField(pnTyp, prop.arg(field(pnVon).toDate().toString(qsl("bis dd.MM.yyyy"))));
}

/* ////////////////////////////
  wpTimeFrame
*/
wpTimeFrame::wpTimeFrame(QWidget* w) : QWizardPage(w)
{   LOG_CALL;
    setTitle(qsl("Ausgabezeitraum"));
    subTitleLabel->setText(qsl("Gib an, in welchem Zeitfenster Verträge zu dieser Anlage gezählt werden."));
    subTitleLabel->setWordWrap(true);

    QLabel* lVon =new QLabel(qsl("Erstausgabe"));
    lVon->setToolTip(qsl("Verträge mit dem zuvor angegebenen Zinssatz werden ab diesem Datum zu der Geldanlage gezählt"));
    QLabel* lBis =new QLabel(qsl("Ende der Ausgabe"));
    lBis->setToolTip(qsl("Verträge mit dem zuvor angegebenen Zinssatz werden bis zu diesem Datum zu der Geldanlage gezählt"));

    cbFloating =new QCheckBox(qsl("Ohne Datum (\"fortlaufend\")"));
    cbFloating->setToolTip (qsl("Bei einer Anlage ohne Ende Datum werden immer die letzten 12 Monate als Referenzzeitraum verwendet"));
    cbFloating->setCheckState (Qt::Unchecked);
    connect(cbFloating, &QCheckBox::checkStateChanged, this, &wpTimeFrame::onSwitchFloating);

    deVon =new QDateEdit();
    deVon->setDisplayFormat(qsl("dd.MM.yyyy"));
    lVon->setBuddy(deVon);
    deVon->setToolTip(lVon->toolTip());
    registerField(pnVon, deVon/*, "date", "dateChanged(QDate)"*/);

    deBis =new QDateEdit();
    deBis->setDisplayFormat(qsl("dd.MM.yyyy"));
    lBis->setBuddy(deBis);
    deBis->setToolTip(lBis->toolTip());
    registerField(pnBis, deBis/*, "date", "dateChanged(QDate)"*/);

    connect(deVon, &QDateTimeEdit::dateChanged, this, &wpTimeFrame::onStartDate_changed);

    QHBoxLayout* hlVon =new QHBoxLayout();
    hlVon->addWidget(lVon);
    hlVon->addWidget(deVon);
    QHBoxLayout* hlBis =new QHBoxLayout();
    hlBis->addWidget(lBis); hlBis->addWidget(deBis);
    QVBoxLayout* vl =new QVBoxLayout();
    vl->addWidget(subTitleLabel);
    vl->addWidget (cbFloating);
    vl->addLayout(hlVon);
    vl->addLayout(hlBis);
    setLayout(vl);
}

void wpTimeFrame::onSwitchFloating(int state)
{
    if( state == Qt::CheckState::Checked){
        deVon->setDate (BeginingOfTime);
        deVon->setEnabled (false);
        deBis->setDate (EndOfTheFuckingWorld);
        deBis->setEnabled (false);
    } else {
        wizNewInvestment* wiz =qobject_cast<wizNewInvestment*> (wizard());
        deVon->setDate (wiz->getInitStartDate ());
        deVon->setEnabled (true);
        deBis->setDate (deVon->date ().addYears (1));
        deBis->setEnabled (true);
    }
}

void wpTimeFrame::initializePage()
{   LOG_CALL_W(qsl("wpTimeFrame"));
}
bool wpTimeFrame::validatePage()
{   LOG_CALL_W("wpTimeFrame");
    QDate von =field(pnVon).toDate();
    QDate bis =field(pnBis).toDate();
    return von < bis;
}
void wpTimeFrame::onStartDate_changed()
{   LOG_CALL;
    if( cbFloating->checkState () == Qt::Unchecked)
        setField(pnBis, field(pnVon).toDate().addYears(1));
}

/* ////////////////////////////
  wpNewInvestInterest
*/
wpNewInvestInterest::wpNewInvestInterest(QWidget* p) : QWizardPage(p)
{   LOG_CALL;
    setTitle(qsl("Geldanlage festlegen"));
    subTitleLabel->setText(qsl("Mit dieser Dialogfolge kannst Du eine Geldanlage definieren.<br>"
                                "Gib zunächst den Zinssatz der Geldanlage ein:<p>"));
    subTitleLabel->setWordWrap(true);

    QLabel* lZinssatz =new QLabel(qsl("Zinssatz"));

    QComboBox* cbInterest =new QComboBox;
    lZinssatz->setBuddy(cbInterest);
    registerField(pnZSatz, cbInterest/*, "currentText", "currentTextChanged"*/);
    const int mI =dbConfig::readValue(MAX_INTEREST).toInt();
    for( int i =0; i <= mI; i++)
        cbInterest->addItem(QString::number(i/100., 'f', 2), QVariant(i));
    cbInterest->setCurrentIndex(qMin(90, cbInterest->count()));
    cbInterest->setToolTip(qsl("Verträge mit diesem Zinssatz werden zu dieser Geldanlage gehören."));
    QGridLayout* layout =new QGridLayout();
    layout->addWidget(subTitleLabel, 0, 0, 1, 3);
    layout->addWidget(lZinssatz, 1, 0, 2, 1);
    layout->addWidget(cbInterest, 1, 1, 1, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 2);
    setLayout(layout);
}

/* ////////////////////////////
  wizNewInvestment
*/
wizNewInvestment::wizNewInvestment(QWidget* p) : QWizard(p)
{
    addPage(new wpNewInvestInterest());
    addPage(new wpTimeFrame());
    addPage(new wpType());
    addPage(new wpInvestmentSummary());
}

void wizNewInvestment::initStartDate (QDate d)
{
    initialStartDate =d;
    setField(pnVon, d);
}
