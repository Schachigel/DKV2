#include "pch.h"

#include"helper.h"
#include "letters.h"

void printThankyouLetter( const contract& )
{   LOG_CALL;
    letterTemplate tlate(letterTemplate::templId::activation);
    tlate.setPlaceholder(qsl("datum"), QDate::currentDate().toString(qsl("dd. MMM yyyy")));
//    tlate.setPlaceholder(GMBH_ADDRESS1, getMetaInfo(GMBH_ADDRESS1));
//    tlate.setPlaceholder(GMBH_ADDRESS2, getMetaInfo(GMBH_ADDRESS2));
//    tlate.setPlaceholder(GMBH_PLZ, getMetaInfo(GMBH_PLZ));
//    tlate.setPlaceholder(GMBH_CITY, getMetaInfo(GMBH_CITY));
//    tlate.setPlaceholder(GMBH_STREET, getMetaInfo(GMBH_STREET));
//    tlate.setPlaceholder(GMBH_EMAIL, getMetaInfo(GMBH_EMAIL));
//    tlate.setPlaceholder(GMBH_URL, getMetaInfo(GMBH_URL));

Q_ASSERT( not "repair");
//    tlate.setPlaceholder("vertraege.betrag", locale.toCurrencyString( v.Betrag()));
//    tlate.setPlaceholder("vertraege.buchungsdatum", v.StartZinsberechnung().toString("dd. MMM yyyy"));
//    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
//    tlate.setPlaceholder("kreditoren.vorname", v.Vorname());
//    tlate.setPlaceholder("kreditoren.nachname", v.Nachname());
//    tlate.setPlaceholder("kreditoren.strasse", v.getKreditor().getValue("Strasse").toString());
//    tlate.setPlaceholder("kreditoren.plz", v.getKreditor().getValue("Plz").toString());
//    tlate.setPlaceholder("kreditoren.stadt", v.getKreditor().getValue("Stadt").toString());
//    tlate.setPlaceholder("kreditoren.email", v.getKreditor().getValue("Email").toString());

//    QString filename = tlate.fileNameFromId(v.Kennung());
//    if( tlate.print(filename))
//        showFileInFolder(filename);
}

void printTerminationLetter( const contract& , QDate kDate, int )
{   LOG_CALL;
    letterTemplate tlate(letterTemplate::templId::termination);
    tlate.setPlaceholder(qsl("datum"), QDate::currentDate().toString(qsl("dd. MMM yyyy")));
    tlate.setPlaceholder(qsl("kuendigungsdatum"), kDate.toString(qsl("dd.MM.yyyy")));
//    tlate.setPlaceholder(GMBH_ADDRESS1, getMetaInfo(GMBH_ADDRESS1));
//    tlate.setPlaceholder(GMBH_ADDRESS2, getMetaInfo(GMBH_ADDRESS2));
//    tlate.setPlaceholder(GMBH_PLZ, getMetaInfo(GMBH_PLZ));
//    tlate.setPlaceholder(GMBH_CITY, getMetaInfo(GMBH_CITY));
//    tlate.setPlaceholder(GMBH_STREET, getMetaInfo(GMBH_STREET));
//    tlate.setPlaceholder(GMBH_EMAIL, getMetaInfo(GMBH_EMAIL));
//    tlate.setPlaceholder(GMBH_URL, getMetaInfo(GMBH_URL));
Q_ASSERT( not "repair");
//    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
//    tlate.setPlaceholder("vertraege.kfrist", QString::number(kMonate));
//    tlate.setPlaceholder("vertraege.laufzeitende", v.LaufzeitEnde().toString("dd.MM.yyyy"));
//    tlate.setPlaceholder("kreditoren.vorname", v.Vorname());
//    tlate.setPlaceholder("kreditoren.nachname", v.Nachname());
//    tlate.setPlaceholder("kreditoren.strasse", v.getKreditor().getValue("Strasse").toString());
//    tlate.setPlaceholder("kreditoren.plz", v.getKreditor().getValue("Plz").toString());
//    tlate.setPlaceholder("kreditoren.stadt", v.getKreditor().getValue("Stadt").toString());
//    tlate.setPlaceholder("kreditoren.email", v.getKreditor().getValue("Email").toString());

//    QString filename = tlate.fileNameFromId(v.Kennung());
//    if( tlate.print(filename))
//        showFileInFolder(filename);
}

void printFinalLetter( const contract& , QDate )
{   LOG_CALL;
    letterTemplate tlate(letterTemplate::templId::accountClosing);
//    tlate.setPlaceholder(GMBH_ADDRESS1, getMetaInfo(GMBH_ADDRESS1));
//    tlate.setPlaceholder(GMBH_ADDRESS2, getMetaInfo(GMBH_ADDRESS2));
//    tlate.setPlaceholder(GMBH_PLZ, getMetaInfo(GMBH_PLZ));
//    tlate.setPlaceholder(GMBH_CITY, getMetaInfo(GMBH_CITY));
//    tlate.setPlaceholder(GMBH_STREET, getMetaInfo(GMBH_STREET));
//    tlate.setPlaceholder(GMBH_EMAIL, getMetaInfo(GMBH_EMAIL));
//    tlate.setPlaceholder(GMBH_URL, getMetaInfo(GMBH_URL));
Q_ASSERT( not "repair");
//    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
//    tlate.setPlaceholder("datum", QDate::currentDate().toString("dd. MMM yyyy"));
//    tlate.setPlaceholder("vertraege.buchungsdatum", contractEnd.toString("dd.MM.yyyy"));
//    tlate.setPlaceholder("kreditoren.iban", v.getKreditor().getValue("Iban").toString());
//    tlate.setPlaceholder("tbh.kennung", "Vertragskennung");
//    tlate.setPlaceholder("tbh.old", "Vorheriger Wert");
//    tlate.setPlaceholder("tbh.zins", "Zins");
//    tlate.setPlaceholder("tbh.new", "Abschließender Wert");

//    QString filename = tlate.fileNameFromId(v.Kennung());
//    if( tlate.print(filename))
//        showFileInFolder(filename);
}
