#ifndef VERTRAG_H
#define VERTRAG_H

#include <QString>
#include <QDate>
#include <QVariant>
#include "helper.h"
#include "kreditor.h"
#include "dkdbhelper.h"

class Contract
{
public:
    Contract(qlonglong kId = -1, QString ken = "",
            double betrag =0., double wert =0., qlonglong zId =-1,
            QDate vd =QDate::currentDate(),
            bool thesa =true, bool aktiv =false,
            QDate startd =EndOfTheFuckingWorld,
            int kfrist=6,
            QDate endd = EndOfTheFuckingWorld)
            :
        id(-1), kreditorId(kId), kennung(ken),
        betrag(betrag), wert(wert), zinsId(zId),
        thesaurierend(thesa), active(aktiv),
        vertragsdatum(vd), startZinsberechnung(startd),
          laufzeitEnde(endd), kFrist(kfrist), letzteZinsgutschrift(0.) {
        if( kId != -1) initCreditor();
        if( !thesaurierend) this->wert = 0.;
    }
    // getter
    qlonglong getVid() const { return id;}
    double Betrag() const { return betrag;}
    double Wert() const { return wert;}
    qlonglong KreditorId() const { return kreditorId;}
    QString Kennung() const {return kennung;}
    qlonglong ZinsId() const {return zinsId;}
    double Zins() const {return letzteZinsgutschrift;}
    QDate Vertragsabschluss() const {return vertragsdatum;}
    QDate LaufzeitEnde() const {return laufzeitEnde;}
    bool Thesaurierend() const {return thesaurierend;}
    double Zinsfuss() const {return zinsFuss;}
    QDate StartZinsberechnung() const {return startZinsberechnung;}
    QString Vorname() const {return dkGeber.getValue("Vorname").toString();}
    QString Nachname() const {return dkGeber.getValue("Nachname").toString();}
    Kreditor getKreditor() const { return dkGeber;}
    bool isActive() const {return active;}
    int Kuendigungsfrist() const { return kFrist;}
    // setter
    void setVid(int i){ id = i;}

    // interface
    bool loadContractFromDb(qlonglong id);
    bool validateAndSaveNewContract(QString& meldung);
    bool bookNewContract();
    bool activateContract(const QDate& aDate);
    bool bookAnnualInterest(const QDate& YearEnd);
    // statics
    bool deleteInactiveContract();
    bool cancelActiveContract(const QDate& kTermin);
    bool terminateActiveContract(const QDate& termin);
private:
    // helper
    bool saveRecord(const qlonglong BArt, const QString& msg);
    bool speichereBelegKuendigung();
    int saveNewContract() const;
    bool saveRecordNewContract();
    void updateAusDb(){loadContractFromDb(id);}
    bool saveAnnualPayment(const QDate& end);
    bool saveRecordAnnualPayment(const QDate& end);
    void initCreditor();
    // data
    qlonglong id;
    qlonglong kreditorId;
    QString kennung;
    double betrag;
    double wert;
    qlonglong zinsId;
    bool thesaurierend;
    bool active;
    QDate vertragsdatum;
    QDate startZinsberechnung;
    QDate laufzeitEnde;
    int kFrist;
    // Belegdaten
    double zinsFuss;
    double letzteZinsgutschrift;
    Kreditor dkGeber;
    QString buchungsdatenJson;
};

#endif // VERTRAG_H
