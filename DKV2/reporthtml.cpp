#include <QDate>
#include <QLocale>

#include "dkdbhelper.h"
#include "reporthtml.h"

// statistics pages - Helper Fu
QString tag(QString string, QString tag, QStringList attribs =QStringList())
{
    QString ret("<"+tag);
    for(QString s : attribs) {
        ret += " " +s;
    }
    return ret + ">" + string + "</" + tag + ">";
}

QString tableRow4( QString left, QString center, QString center2, QString right)
{
    left   =tag(left, "td", {"style='text-align: right;'"});
    center =tag(center, "td", {"style='text-align: center;'"});
    center2=tag(center2, "td", {"style='text-align: center;'"});
    right  =tag(right, "td", {"style='text-align: left;'"});
    return tag( left + center + center2 + right, "tr");
}
QString tableRow3( QString left, QString center, QString right)
{
    left   = "<td style='text-align: right;' >" + left   + "</td>";
    center = "<td style='text-align: center;'>" + center + "</td>";
    right  = "<td style='text-align: left;'  >" + right  + "</td>";
    return "<tr>" + left + center + right  + "</tr>";
}
QString tableRow2(QString left, QString right)
{
    left = "<td style='text-align: right;'>" + left  + "</td>";
    right= "<td style='text-align: left;' >" + right + "</td>";
    return "<tr>" + left + right  + "</tr>";
}
QString emptyRow( )
{
    return "<tr><td style='padding: 1px; font-size: small;'></td><td style='padding: 1px; font-size: small';></td></tr>";
}

QString h2(QString v) {
    return tag(v, "h2");
}
QString h1(QString v) {
    return tag(v, "h1");
}
QString td( QString v)
{
    return "<td>" + v + "</td>";
}
QString startTable()
{
    return "<table cellpadding='8' bgcolor=#DDD>";
}
QString endTable()
{
    return "</table>";
}
QString row( QString cont)
{
    return "<tr>" + cont + "</tr>";
}
QString startRow()
{
    return "<tr>";
}
QString endRow()
{
    return "</t>";
}
QString newLine(QString line)
{
    return "<br>" + line;
}

QString htmlOverviewTable()
{
    QLocale locale;
    QString ret;
    DbSummary dbs =calculateSummary();
    ret += h1("Übersicht DKs und DK Geber")+ newLine( "Stand: " + QDate::currentDate().toString("dd.MM.yyyy<br>"));
    ret += startTable();
    ret += tableRow2("Anzahl DK Geber*innen von aktiven Verträgen:", QString::number(dbs.AnzahlDkGeber));
    ret += tableRow2("Anzahl aktiver Direktkredite:" , QString::number(dbs.AnzahlAktive));
    ret += tableRow2("Wert der aktiven Direktkredite:"  , locale.toCurrencyString(dbs.WertAktive) + "<br><small>(Ø " + locale.toCurrencyString(dbs.WertAktive/dbs.AnzahlAktive) + ")</small>");
    ret += tableRow2("Durchschnittlicher Zinssatz:<br><small>(Gewichtet mit Vertragswert)</small>", QString::number(dbs.DurchschnittZins, 'f', 3) + "%");
    ret += tableRow2("Jährliche Zinskosten:", locale.toCurrencyString(dbs.WertAktive * dbs.DurchschnittZins/100.));
    ret += tableRow2("Mittlerer Zinssatz:", QString::number(dbs.MittlererZins, 'f', 3) + "%");
    ret += emptyRow();
    ret += tableRow2("Anzahl mit jährl. Zinsauszahlung:", QString::number(dbs.AnzahlAuszahlende));
    ret += tableRow2("Summe:", locale.toCurrencyString(dbs.BetragAuszahlende));
    ret += emptyRow();
    ret += tableRow2("Anzahl ohne jährl. Zinsauszahlung:", QString::number(dbs.AnzahlThesaurierende));
    ret += tableRow2("Wert inkl. Zinsen:", locale.toCurrencyString(dbs.WertThesaurierende));
    ret += emptyRow();
    ret += tableRow2("Anzahl ausstehender (inaktiven) DK", QString::number(dbs.AnzahlPassive));
    ret += tableRow2("Summe ausstehender (inaktiven) DK", locale.toCurrencyString(dbs.BetragPassive));
    ret += endTable();
    return ret;
}
QString htmlContractsByContractEndTable()
{
    QLocale locale;
    QString ret;
    ret += h1("Auslaufende Verträge") + newLine( "Stand: "  + QDate::currentDate().toString("dd.MM.yyyy<br>"));
     QVector<ContractEnd> ce;
     calc_contractEnd(ce);
     if( !ce.isEmpty()) {
         ret += startTable();
         ret += tableRow3( h2("Jahr"), h2( "Anzahl"),  h2( "Summe"));
         for( auto x: ce)
             ret += tableRow3( QString::number(x.year), QString::number(x.count), locale.toCurrencyString(x.value));
         ret += endTable();
     }
     else
         ret += "<br><br><i>keine Verträge mit vorgemerktem Vertragsende</i>";
     return ret;
}
QString htmlContractsByYearByInterestTable()
{
    QLocale locale;
    QString ret;
    QVector<YZV> yzv;
    calc_anualInterestDistribution( yzv);
    if( !yzv.isEmpty()) {
        ret += h1("Verteilung der Zinssätze pro Jahr") + "<br> Stand:"  + QDate::currentDate().toString("dd.MM.yyyy<br>");
        ret += startTable() +  startRow();
        ret += td(h2("Jahr")) + td( h2( "Zinssatz")) +td(h2("Anzahl")) + td( h2( "Summe"));
        ret += endRow();
        for( auto x: yzv) {
            ret += tableRow4( QString::number(x.year), QString("%1%").arg(x.intrest, 2, 'g'), QString::number(x.count), locale.toCurrencyString(x.sum));
        }
        ret += endTable();
    }
    return ret;
}
QString htmlContractsByRuntimeTable()
{
    QString ret;
    ret += h1("Vertragslaufzeiten") + "<br> Stand:" + QDate::currentDate().toString("dd.MM.yyyy<br>");
    ret += startTable();
    QVector<rowData> rows = contractRuntimeDistribution();
    ret += tableRow3( h2(rows[0].text), h2(rows[0].value), h2(rows[0].number));
    for( int i = 1; i < rows.count(); i++)
        ret += tableRow3
                (rows[i].text, rows[i].value, rows[i].number);
    ret += endTable();
    return ret;
}
QString htmlAllContractInfo()
{
    dbtable t("ContractDataActiveContracts");
    t.append(dbfield("Id", QVariant::Type::Int));
    t.append(dbfield("KreditorId", QVariant::Type::Int));
    t.append(dbfield("Vorname"));
    t.append(dbfield("Nachname"));
    t.append(dbfield("Strasse"));
    t.append(dbfield("Plz"));
    t.append(dbfield("Stadt"));
    t.append(dbfield("Email"));
    t.append(dbfield("Iban"));
    t.append(dbfield("Bic"));
    t.append(dbfield("Strasse"));
    t.append(dbfield("Zinssatz", QVariant::Type::Double));
    t.append(dbfield("Wert", QVariant::Type::Double));
    t.append(dbfield("Aktivierungsdatum", QVariant::Type::Date));
    t.append(dbfield("Kuendigungsfrist", QVariant::Type::Int));
    t.append(dbfield("Vertragsende", QVariant::Type::Date));
    t.append(dbfield("thesa", QVariant::Type::Bool));

    QString ret =startTable();
    ret += startRow();
    for(auto field : t.Fields()) {
        ret +=tag(field.name(), "th");
    }
    ret += endRow();
    QVector<QSqlRecord> data = executeSql(t.Fields());
    for(auto rec : data) {
        ret += startRow();
        for( int i =0; i<rec.count(); i++) {
            ret += td(rec.field(i).value().toString());
        }
        ret += endRow();
    }
    ret += endTable();
    return ret;
}
QString reportHtml(Uebersichten u)
{
    QString html ="<html><body>"
                    "<style>"
                      "table { border-width: 0px; font-family: Verdana; font-size: large; }"
                      "td { }"
                    "</style>"
                    "%1"
                  "</body></html>";
    switch( u ) {
    case OVERVIEW: {
        html =html.arg(htmlOverviewTable());
        break;
    }
    case BY_CONTRACT_END: {
        html =html.arg(htmlContractsByContractEndTable());
        break;
    }
    case INTEREST_DISTRIBUTION: {
        html =html.arg(htmlContractsByYearByInterestTable());
        break;
    }
    case CONTRACT_TERMS: {
        html =html.arg(htmlContractsByRuntimeTable());
        break;
    }
    case ALL_CONTRACT_INFO: {
        html =html.arg(htmlAllContractInfo());
        break;
    }
    default:
    {Q_ASSERT(false);}
    }
    return html;
}

