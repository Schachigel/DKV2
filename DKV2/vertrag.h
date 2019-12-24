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
    Vertrag(int kId = -1, QString ken = "",
            double betrag =0., double wert =0., int zId =-1,
            QDate vd =QDate::currentDate(),
            bool thesa =true, bool aktiv =false,
            QDate startd =EndOfTheFuckingWorld,
            QDate endd = EndOfTheFuckingWorld)
            :
        id(-1), kreditorId(kId), kennung(ken),
        betrag(betrag), wert(wert), zinsId(zId),
        thesaurierend(thesa), active(aktiv),
        vertragsdatum(vd), startZinsberechnung(startd),
        laufzeitEnde(endd), letzteZinsgutschrift(0.) {
        if( !thesaurierend) this->wert = 0.;
    }
    // getter
    int getVid() const { return id;}
    double Betrag() const { return betrag;}
    double Wert() const { return wert;}
    int KreditorId() const { return kreditorId;}
    QString Kennung() const {return kennung;}
    int ZinsId() const {return zinsId;}
    double Zins() const {return letzteZinsgutschrift;}
    QDate Vertragsabschluss() const {return vertragsdatum;}
    QDate LaufzeitEnde() const {return laufzeitEnde;}
    bool Thesaurierend() const {return thesaurierend;}
    double Zinsfuss() const {return zinsFuss;}
    QDate StartZinsberechnung() const {return startZinsberechnung;}
    QString Vorname() const {return dkGeber.getValue("Vorname").toString();}
    QString Nachname() const {return dkGeber.getValue("Nachname").toString();}
    Kreditor getKreditor() const { return dkGeber;}
    // setter
    void setVid(int i){ id = i;}

    // interface
    bool ausDb(int id, bool mitBelegdaten= false);
    bool verbucheNeuenVertrag();
    bool aktiviereVertrag(const QDate& aDate);
    bool verbucheJahreszins(const QDate& YearEnd);
    // statics
    bool passivenVertragLoeschen();
    bool aktivenVertragLoeschen(const QDate& termin);
private:
    // helper
    bool BelegSpeichern(const int BArt, const QString& msg);
    int speichereNeuenVertrag() const;
    bool speichereBelegNeuerVertrag();
    void updateAusDb(){ausDb(id, true);}
    bool speichereJahresabschluss(const QDate& end);
    bool speichereBelegJahresabschluss(const QDate& end);
    // data
    int id;
    int kreditorId;
    QString kennung;
    double betrag;
    double wert;
    int zinsId;
    bool thesaurierend;
    bool active;
    QDate vertragsdatum;
    QDate startZinsberechnung;
    QDate laufzeitEnde;
    // Belegdaten
    double zinsFuss;
    double letzteZinsgutschrift;
    Kreditor dkGeber;
    QString buchungsdatenJson;
};

#endif // VERTRAG_H
