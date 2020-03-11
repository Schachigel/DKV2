#include <QLocale>

#include <letters.h>


void printThankyouLetter( const Contract& v)
{
    QLocale locale;
    letterTemplate tlate(letterTemplate::Geldeingang);
    tlate.setPlaceholder("datum", QDate::currentDate().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("gmbh.address1", getProperty("gmbh.address1"));
    tlate.setPlaceholder("gmbh.address2", getProperty("gmbh.address2"));
    tlate.setPlaceholder("gmbh.plz", getProperty("gmbh.plz"));
    tlate.setPlaceholder("gmbh.stadt", getProperty("gmbh.stadt"));
    tlate.setPlaceholder("gmbh.strasse", getProperty("gmbh.strasse"));
    tlate.setPlaceholder("gmbh.email", getProperty("gmbh.email"));
    tlate.setPlaceholder("gmbh.url", getProperty("gmbh.url"));
    tlate.setPlaceholder("vertraege.betrag", locale.toCurrencyString( v.Betrag()));
    tlate.setPlaceholder("vertraege.buchungsdatum", v.StartZinsberechnung().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
    tlate.setPlaceholder("kreditoren.vorname", v.Vorname());
    tlate.setPlaceholder("kreditoren.nachname", v.Nachname());
    tlate.setPlaceholder("kreditoren.strasse", v.getKreditor().getValue("Strasse").toString());
    tlate.setPlaceholder("kreditoren.plz", v.getKreditor().getValue("Plz").toString());
    tlate.setPlaceholder("kreditoren.stadt", v.getKreditor().getValue("Stadt").toString());
    tlate.setPlaceholder("kreditoren.email", v.getKreditor().getValue("Email").toString());

    QString filename(v.Kennung());
    if( tlate.print(filename))
        showFileInFolder(filename);

}

void printTerminationLetter( const Contract& v, QDate kDate, int kMonate)
{
    QLocale locale;
    letterTemplate tlate(letterTemplate::Kuendigung);
    tlate.setPlaceholder("datum", QDate::currentDate().toString("dd. MMM yyyy"));
    tlate.setPlaceholder("kuendigungsdatum", kDate.toString("dd.MM.yyyy"));
    tlate.setPlaceholder("gmbh.address1", getProperty("gmbh.address1"));
    tlate.setPlaceholder("gmbh.address2", getProperty("gmbh.address2"));
    tlate.setPlaceholder("gmbh.plz", getProperty("gmbh.plz"));
    tlate.setPlaceholder("gmbh.stadt", getProperty("gmbh.stadt"));
    tlate.setPlaceholder("gmbh.strasse", getProperty("gmbh.strasse"));
    tlate.setPlaceholder("gmbh.email", getProperty("gmbh.email"));
    tlate.setPlaceholder("gmbh.url", getProperty("gmbh.url"));
    tlate.setPlaceholder("vertraege.kennung", v.Kennung());
    tlate.setPlaceholder("vertraege.kfrist", QString::number(kMonate));
    tlate.setPlaceholder("vertraege.laufzeitende", v.LaufzeitEnde().toString("dd.MM.yyyy"));
    tlate.setPlaceholder("kreditoren.vorname", v.Vorname());
    tlate.setPlaceholder("kreditoren.nachname", v.Nachname());
    tlate.setPlaceholder("kreditoren.strasse", v.getKreditor().getValue("Strasse").toString());
    tlate.setPlaceholder("kreditoren.plz", v.getKreditor().getValue("Plz").toString());
    tlate.setPlaceholder("kreditoren.stadt", v.getKreditor().getValue("Stadt").toString());
    tlate.setPlaceholder("kreditoren.email", v.getKreditor().getValue("Email").toString());

    QString filename(v.Kennung());
    if( tlate.print(filename))
        showFileInFolder(filename);

}
