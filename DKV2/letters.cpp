#include <QLocale>

#include <letters.h>


void printThankyouLetter( const Vertrag& v)
{
    QLocale locale;
    letterTemplate tlate(letterTemplate::geldeingang);
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
