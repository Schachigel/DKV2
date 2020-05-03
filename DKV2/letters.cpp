#include <QLocale>

#include"helper.h"
#include "letters.h"

void printThankyouLetter( const Contract& v, QSqlDatabase db)
{   LOG_CALL;
    QLocale locale;
    letterTemplate tlate(letterTemplate::Geldeingang);
    tlate.setPlaceholder("datum", QDate::currentDate().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("gmbh.address1", getMetaInfo("gmbh.address1", db));
    tlate.setPlaceholder("gmbh.address2", getMetaInfo("gmbh.address2", db));
    tlate.setPlaceholder("gmbh.plz", getMetaInfo("gmbh.plz", db));
    tlate.setPlaceholder("gmbh.stadt", getMetaInfo("gmbh.stadt", db));
    tlate.setPlaceholder("gmbh.strasse", getMetaInfo("gmbh.strasse", db));
    tlate.setPlaceholder("gmbh.email", getMetaInfo("gmbh.email", db));
    tlate.setPlaceholder("gmbh.url", getMetaInfo("gmbh.url", db));
    tlate.setPlaceholder("vertraege.betrag", locale.toCurrencyString( v.Betrag()));
    tlate.setPlaceholder("vertraege.buchungsdatum", v.StartZinsberechnung().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
    tlate.setPlaceholder("kreditoren.vorname", v.Vorname());
    tlate.setPlaceholder("kreditoren.nachname", v.Nachname());
    tlate.setPlaceholder("kreditoren.strasse", v.getKreditor().getValue("Strasse").toString());
    tlate.setPlaceholder("kreditoren.plz", v.getKreditor().getValue("Plz").toString());
    tlate.setPlaceholder("kreditoren.stadt", v.getKreditor().getValue("Stadt").toString());
    tlate.setPlaceholder("kreditoren.email", v.getKreditor().getValue("Email").toString());

    QString filename = tlate.fileNameFromId(v.Kennung());
    if( tlate.print(filename))
        showFileInFolder(filename);
}

void printTerminationLetter( const Contract& v, QDate kDate, int kMonate, QSqlDatabase db)
{   LOG_CALL;
    QLocale locale;
    letterTemplate tlate(letterTemplate::Kuendigung);
    tlate.setPlaceholder("datum", QDate::currentDate().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("kuendigungsdatum", kDate.toString("dd.MM.yyyy"));
    tlate.setPlaceholder("gmbh.address1", getMetaInfo("gmbh.address1", db));
    tlate.setPlaceholder("gmbh.address2", getMetaInfo("gmbh.address2", db));
    tlate.setPlaceholder("gmbh.plz", getMetaInfo("gmbh.plz", db));
    tlate.setPlaceholder("gmbh.stadt", getMetaInfo("gmbh.stadt", db));
    tlate.setPlaceholder("gmbh.strasse", getMetaInfo("gmbh.strasse", db));
    tlate.setPlaceholder("gmbh.email", getMetaInfo("gmbh.email", db));
    tlate.setPlaceholder("gmbh.url", getMetaInfo("gmbh.url", db));
    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
    tlate.setPlaceholder("vertraege.kfrist", QString::number(kMonate));
    tlate.setPlaceholder("vertraege.laufzeitende", v.LaufzeitEnde().toString("dd.MM.yyyy"));
    tlate.setPlaceholder("kreditoren.vorname", v.Vorname());
    tlate.setPlaceholder("kreditoren.nachname", v.Nachname());
    tlate.setPlaceholder("kreditoren.strasse", v.getKreditor().getValue("Strasse").toString());
    tlate.setPlaceholder("kreditoren.plz", v.getKreditor().getValue("Plz").toString());
    tlate.setPlaceholder("kreditoren.stadt", v.getKreditor().getValue("Stadt").toString());
    tlate.setPlaceholder("kreditoren.email", v.getKreditor().getValue("Email").toString());

    QString filename = tlate.fileNameFromId(v.Kennung());
    if( tlate.print(filename))
        showFileInFolder(filename);
}

void printFinalLetter( const Contract& v, QDate contractEnd, QSqlDatabase db)
{   LOG_CALL;
    QLocale locale;
    letterTemplate tlate(letterTemplate::Kontoabschluss);
    tlate.setPlaceholder("gmbh.address1", getMetaInfo("gmbh.address1", db));
    tlate.setPlaceholder("gmbh.address2", getMetaInfo("gmbh.address2", db));
    tlate.setPlaceholder("gmbh.plz", getMetaInfo("gmbh.plz", db));
    tlate.setPlaceholder("gmbh.stadt", getMetaInfo("gmbh.stadt", db));
    tlate.setPlaceholder("gmbh.strasse", getMetaInfo("gmbh.strasse", db));
    tlate.setPlaceholder("gmbh.email", getMetaInfo("gmbh.email", db));
    tlate.setPlaceholder("gmbh.url", getMetaInfo("gmbh.url", db));
    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
    tlate.setPlaceholder("datum", QDate::currentDate().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("vertraege.buchungsdatum", contractEnd.toString("dd.MM.yyyy"));
    tlate.setPlaceholder("kreditoren.iban", v.getKreditor().getValue("Iban").toString());
    tlate.setPlaceholder("tbh.kennung", "Vertragskennung");
    tlate.setPlaceholder("tbh.old", "Vorheriger Wert");
    tlate.setPlaceholder("tbh.zins", "Zins");
    tlate.setPlaceholder("tbh.new", "Abschließender Wert");

    QString filename = tlate.fileNameFromId(v.Kennung());
    if( tlate.print(filename))
        showFileInFolder(filename);
}
