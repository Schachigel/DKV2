#include <QDate>
#include <QLocale>

#include "dkdbhelper.h"
#include "reporthtml.h"

// statistics pages - Helper Fu
QString tableRow( QString left, QString center, QString center2, QString right)
{
    left    = "<td style='text-align: right;' >" + left    + "</td>";
    center  = "<td style='text-align: center;'>" + center  + "</td>";
    center2 = "<td style='text-align: center;'>" + center2 + "</td>";
    right   = "<td style='text-align: left;'  >" + right   + "</td>";
    return "<tr>" + left + center + center2 + right  + "</tr>";
}
QString tableRow( QString left, QString center, QString right)
{
    left   = "<td style='text-align: right;' >" + left   + "</td>";
    center = "<td style='text-align: center;'>" + center + "</td>";
    right  = "<td style='text-align: left;'  >" + right  + "</td>";
    return "<tr>" + left + center + right  + "</tr>";
}
QString tableRow(QString left, QString right)
{
    left = "<td style='text-align: right;'>" + left  + "</td>";
    right= "<td style='text-align: left;' >" + right + "</td>";
    return "<tr>" + left + right  + "</tr>";
}
QString emptyRow( )
{
    return "<tr><td style='padding: 1px; font-size: small;'></td><td style='padding: 1px; font-size: small';></td></tr>";
}
QString h2(QString v)
{
    return "<h2>" + v + "</h2>";
}
QString h1(QString v)
{
    return "<h1>" + v + "</h1>";
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
    ret += tableRow("Anzahl DK Geber*innen von aktiven Verträgen:", QString::number(dbs.AnzahlDkGeber));
    ret += tableRow("Anzahl aktiver Direktkredite:" , QString::number(dbs.AnzahlAktive));
    ret += tableRow("Wert der aktiven Direktkredite:"  , locale.toCurrencyString(dbs.WertAktive) + "<br><small>(Ø " + locale.toCurrencyString(dbs.WertAktive/dbs.AnzahlAktive) + ")</small>");
    ret += tableRow("Durchschnittlicher Zinssatz:<br><small>(Gewichtet mit Vertragswert)</small>", QString::number(dbs.DurchschnittZins, 'f', 3) + "%");
    ret += tableRow("Jährliche Zinskosten:", locale.toCurrencyString(dbs.WertAktive * dbs.DurchschnittZins/100.));
    ret += tableRow("Mittlerer Zinssatz:", QString::number(dbs.MittlererZins, 'f', 3) + "%");
    ret += emptyRow();
    ret += tableRow("Anzahl mit jährl. Zinsauszahlung:", QString::number(dbs.AnzahlAuszahlende));
    ret += tableRow("Summe:", locale.toCurrencyString(dbs.BetragAuszahlende));
    ret += emptyRow();
    ret += tableRow("Anzahl ohne jährl. Zinsauszahlung:", QString::number(dbs.AnzahlThesaurierende));
    ret += tableRow("Wert inkl. Zinsen:", locale.toCurrencyString(dbs.WertThesaurierende));
    ret += emptyRow();
    ret += tableRow("Anzahl ausstehender (inaktiven) DK", QString::number(dbs.AnzahlPassive));
    ret += tableRow("Summe ausstehender (inaktiven) DK", locale.toCurrencyString(dbs.BetragPassive));
    ret += endTable();
    return ret;
}
QString htmlContractsByContractEndTable()
{
    QString ret;
    ret += h1("Vertragslaufzeiten") + "<br> Stand:" + QDate::currentDate().toString("dd.MM.yyyy<br>");
    ret += startTable();
    QVector<rowData> rows = contractRuntimeDistribution();
    ret += tableRow( h2(rows[0].text), h2(rows[0].value), h2(rows[0].number));
    for( int i = 1; i < rows.count(); i++)
        ret += tableRow(rows[i].text, rows[i].value, rows[i].number);
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
            ret += tableRow( QString::number(x.year), QString("%1%").arg(x.intrest, 2, 'g'), QString::number(x.count), locale.toCurrencyString(x.sum));
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
    ret += tableRow( h2(rows[0].text), h2(rows[0].value), h2(rows[0].number));
    for( int i = 1; i < rows.count(); i++)
        ret += tableRow(rows[i].text, rows[i].value, rows[i].number);
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
    switch( u )
    {
    case UEBERSICHT: {
        html =html.arg(htmlOverviewTable());
        break;
    }
    case VERTRAGSENDE: {
        html =html.arg(htmlContractsByContractEndTable());
        break;
    }
    case ZINSVERTEILUNG: {
        html =html.arg(htmlContractsByYearByInterestTable());
        break;
    }
    case LAUFZEITEN: {
        html =html.arg(htmlContractsByRuntimeTable());
        break;
    }
    default:
    {Q_ASSERT(false);}
    }
    return html;
}

