#ifndef VERTRAG_H
#define VERTRAG_H

#include <QString>
#include <QDate>
#include <QVariant>
#include "helper.h"
#include "kreditor.h"
#include "dkdbhelper.h"

class Vertrag
{
public:
    Vertrag(qlonglong kId = -1, QString ken = "",
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
        if( kId != -1) initKreditor();
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
    bool ausDb(qlonglong id, bool mitBelegdaten= false);
    bool validateAndSaveNewContract(QString& meldung);
    bool verbucheNeuenVertrag();
    bool aktiviereVertrag(const QDate& aDate);
    bool verbucheJahreszins(const QDate& YearEnd);
    // statics
    bool loeschePassivenVertrag();
    bool kuendigeAktivenVertrag(const QDate& kTermin);
    bool beendeAktivenVertrag(const QDate& termin);
private:
    // helper
    bool BelegSpeichern(const qlonglong BArt, const QString& msg);
    bool speichereBelegKuendigung();
    int speichereNeuenVertrag() const;
    bool speichereBelegNeuerVertrag();
    void updateAusDb(){ausDb(id, true);}
    bool speichereJahresabschluss(const QDate& end);
    bool speichereBelegJahresabschluss(const QDate& end);
    void initKreditor();
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
