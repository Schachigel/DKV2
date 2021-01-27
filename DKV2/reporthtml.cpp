#include <QDate>
#include <QLocale>

#include "appconfig.h"
#include "helper.h"
#include "dkdbhelper.h"
#include "dbstatistics.h"
#include "reporthtml.h"

// statistics pages - Helper Fu
QString tag(QString string, QString tag, QStringList attribs =QStringList())
{
    QString ret(qsl("<")+tag);
    for(auto& s : qAsConst(attribs)) {
        ret += qsl(" ") +s;
    }
    return ret + qsl(">") + string + qsl("</") + tag + qsl(">");
}

QString tableRow4( QString left, QString center, QString center2, QString right)
{
    left   =tag(left,    qsl("td"), {qsl("style='text-align: right;'")});
    center =tag(center,  qsl("td"), {qsl("style='text-align: center;'")});
    center2=tag(center2, qsl("td"), {qsl("style='text-align: center;'")});
    right  =tag(right,   qsl("td"), {qsl("style='text-align: left;'")});
    return tag( left + center + center2 + right, "tr");
}
QString tableRow3( QString left, QString center, QString right)
{
    left   = qsl("<td style='text-align: right;' >") + left   + qsl("</td>");
    center = qsl("<td style='text-align: center;'>") + center + qsl("</td>");
    right  = qsl("<td style='text-align: left;'  >") + right  + qsl("</td>");
    return "<tr>" + left + center + right  + "</tr>";
}
QString tableRow2(QString left, QString right)
{
    left = qsl("<td style='text-align: right;'>") + left  + qsl("</td>");
    right= qsl("<td style='text-align: left;' >") + right + qsl("</td>");
    return qsl("<tr>") + left + right  + qsl("</tr>");
}
QString tableRow1(QString text, int colspan =2)
{
    QString opening(qsl("<tr><td style=\"background - color:rgb(215, 215, 210);\" colspan=\"%1\";>"));
    return  opening.arg(colspan) + text + qsl("</td></tr>");
}
QString emptyRow( int cols) {
    QString ret =tag(qsl(""), qsl("tr"), {"style=\"background - color:rgb(215, 215, 210);\" colspan=\"%1\";>"});
    return ret.arg(QString::number(cols));
}
QString emptyRow( )
{
    return qsl("<tr><td style='padding: 1px; font-size: small;'></td><td style='padding: 1px; font-size: small';></td></tr>");
}
QString b(QString b) {
    return tag(b, qsl("b"));
}
QString h2(QString v) {
    return tag(v, qsl("h2"));
}
QString h1(QString v) {
    return tag(v, qsl("h1"));
}
QString td( QString v)
{
    return qsl("<td>") + v + qsl("</td>");
}
QString startTable()
{
    return qsl("<table cellpadding='8' bgcolor=#EEEEDD style=border-width:1px>");
}
QString endTable()
{
    return qsl("</table>");
}
QString startRow()
{
    return qsl("<tr>");
}
QString endRow()
{
    return qsl("</tr>");
}
QString newLine(QString line)
{
    return qsl("<br>") + line;
}

QString htmlOverviewTableBlock(QString headline, dbStats::dataset ds)
{
    QString ret;
    ret += tableRow1(qsl("<b>") +headline +qsl("</b>"));
    ret += tableRow2(qsl("Anzahl DK Geber*innen"), i2s(ds.credCount.size()));
    ret += tableRow2(qsl("Anzahl Verträge  "),     i2s(ds.nbrContracts));
    ret += tableRow2(qsl("Gesamtwert "),           d2euro(ds.value)
        + qsl("<br><small>(Ø ") + d2euro(ds.value / ds.nbrContracts) + qsl(")</small>"));
    ret += tableRow2(qsl("Durchschnittlicher Zinssatz:<br><small>(Gewichtet mit Vertragswert)</small>"), d2percent(ds.weightedAvgInterestRate));
    ret += tableRow2(qsl("Jährliche Zinskosten:"), d2euro(ds.annualInterest));
    ret += tableRow2(qsl("Mittlerer Zinssatz:"), d2percent(ds.avgInterestRate));
    ret += emptyRow();

    return ret;
}

QString htmlOverviewTable()
{
    QString ret;

    ret += h1(qsl("Übersicht DKs und DK Geber"));
    ret += dbConfig::getValue(GMBH_ADDRESS1).toString();
    ret += qsl(" - Stand: ") + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    dbStats stats(dbStats::calculate);
    ret += htmlOverviewTableBlock(qsl("Alle Aktiven Verträge"), stats.activeContracts[dbStats::t_nt]);
    ret += htmlOverviewTableBlock(qsl("Aktive Verträge ohne Zinsauszahlung"), stats.activeContracts[dbStats::thesa]);
    ret += htmlOverviewTableBlock(qsl("Aktive Verträge mit Zinsauszahlung"), stats.activeContracts[dbStats::pout]);

    ret += htmlOverviewTableBlock(qsl("Alle noch nicht aktiven Verträge"), stats.inactiveContracts[dbStats::t_nt]);
    ret += htmlOverviewTableBlock(qsl("Inaktive Verträge ohne Zinsauszahlung"), stats.inactiveContracts[dbStats::thesa]);
    ret += htmlOverviewTableBlock(qsl("Inaktive Verträge mit Zinsauszahlung"), stats.inactiveContracts[dbStats::pout]);

    ret += htmlOverviewTableBlock(qsl("Alle Verträge"), stats.allContracts[dbStats::t_nt]);
    ret += htmlOverviewTableBlock(qsl("Verträge ohne Zinsauszahlung"), stats.allContracts[dbStats::thesa]);
    ret += htmlOverviewTableBlock(qsl("Verträge mit Zinsauszahlung"), stats.allContracts[dbStats::pout]);
    return ret;
}

QString htmlPayedInterestByYearTable()
{
    QLocale locale;
    QString ret {h1(qsl("Ausgezahlte und angerechnete Zinsen pro Jahr<br>"))};
    QVector<PayedInterest> pi;
    calc_payedInterestsByYear(pi);
    if( pi.isEmpty())
        return qsl("<br><br><i>Bisher wurden keine Zinsen gezahlt</i>");

    ret += startTable();
    ret +=tableRow3(h2(qsl("Jahr")), h2(qsl("Zinstyp")), h2(qsl("Summe")));
    int lastyear = 0;
    for( auto& x: qAsConst(pi)) {
        if( x.year != lastyear) {
            ret += emptyRow(3);
            lastyear = x.year;
        }
        ret +=startRow();
        ret +=qsl("<td style='text-align: right;' >") + QString::number(x.year) + qsl("</td>");
        ret +=qsl("<td style='text-align: left;' >") + x.interestTypeDesc + qsl("</td>");
        ret +=qsl("<td style='text-align: left;' >") + locale.toCurrencyString(x.value) + qsl("</td>");
        ret +=endRow();
    }
    ret += endTable();
    return ret;
}

QString htmlContractsByContractEndTable()
{
    QLocale locale;
    QString ret;
    ret += h1(qsl("Auslaufende Verträge")) + newLine( qsl("Stand: ")  + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>")));
     QVector<ContractEnd> ce;
     calc_contractEnd(ce);
     if( !ce.isEmpty()) {
         ret += startTable();
         ret += tableRow3(h2(qsl("Jahr")), h2(qsl("Anzahl")),  h2(qsl("Summe")));
         for( auto& x: qAsConst(ce))
             ret += tableRow3( QString::number(x.year), QString::number(x.count), locale.toCurrencyString(x.value));
         ret += endTable();
     }
     else
         ret += qsl("<br><br><i>keine Verträge mit vorgemerktem Vertragsende</i>");
     return ret;
}
QString htmlContractsByYearByInterestTable()
{
    QLocale locale;
    QString ret;
    QVector<YZV> yzv;
    calc_anualInterestDistribution( yzv);
    if( !yzv.isEmpty()) {
        ret += h1(qsl("Verteilung der Zinssätze pro Jahr")) + qsl("<br> Stand:")  + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
        ret += startTable() +  startRow();
        ret += td(h2(qsl("Jahr"))) + td( h2(qsl("Zinssatz"))) +td(h2(qsl("Anzahl"))) + td( h2( qsl("Summe")));
        ret += endRow();
        for( auto& x: qAsConst(yzv)) {
            ret += tableRow4( QString::number(x.year), QString(qsl("%1%")).arg(x.intrest, 2, 'g'), QString::number(x.count), locale.toCurrencyString(x.sum));
        }
        ret += endTable();
    }
    return ret;
}
QString htmlContractsByRuntimeTable()
{
    QString ret;
    ret += h1(qsl("Vertragslaufzeiten")) + qsl("<br> Stand:") + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    ret += startTable();
    QVector<rowData> rows = contractRuntimeDistribution();
    if (rows.count() == 0)
        return qsl("");
    ret += tableRow3( h2(rows[0].text), h2(rows[0].value), h2(rows[0].number));
    for( int i = 1; i < rows.count(); i++)
        ret += tableRow3
                (rows[i].text, rows[i].value, rows[i].number);
    ret += endTable();
    return ret;
}
QString htmlAllContractInfo()
{
    dbtable t(qsl("vVertraege_aktiv_detail"));
    t.append(dbfield(qsl("Id"), QVariant::Type::Int));
    t.append(dbfield(qsl("KreditorId"), QVariant::Type::Int));
    t.append(dbfield(qsl("Vertragskennung")));
    t.append(dbfield(qsl("Vorname")));
    t.append(dbfield(qsl("Nachname")));
    t.append(dbfield(qsl("Strasse")));
    t.append(dbfield(qsl("Plz")));
    t.append(dbfield(qsl("Stadt")));
    t.append(dbfield(qsl("Email")));
    t.append(dbfield(qsl("Iban")));
    t.append(dbfield(qsl("Bic")));
    t.append(dbfield(qsl("Strasse")));
    t.append(dbfield(qsl("Zinssatz"), QVariant::Type::Double));
    t.append(dbfield(qsl("Wert"), QVariant::Type::Double));
    t.append(dbfield(qsl("Aktivierungsdatum"), QVariant::Type::Date));
    t.append(dbfield(qsl("Kuendigungsfrist"), QVariant::Type::Int));
    t.append(dbfield(qsl("Vertragsende"), QVariant::Type::Date));
    t.append(dbfield(qsl("thesa"), QVariant::Type::Bool));

    QString ret =startTable();
    ret += startRow();
    for(auto& field : t.Fields()) {
        ret +=tag(field.name(), qsl("th"));
    }
    ret += endRow();
    QVector<QSqlRecord> data = executeSql(t.Fields());
    for(auto& rec : qAsConst(data)) {
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
    QString html =qsl("<html><body>"
                    "<style>"
                      "table { font-family: Verdana; font-size: medium; border: 1px solid black; border-collapse: collapse; }"
                      "td { background-color:rgb(235,235,235); padding: 6px; border: 1px solid black; }"
                      "tr { background-color: #f5f5f5; border: 1px solid black; }"
                    "</style>"
                    "%1"
                  "</body></html>");
    switch( u ) {
    case OVERVIEW: {
        return html.arg(htmlOverviewTable());
        break;
    }
    case PAYED_INTEREST_BY_YEAR: {
        return html.arg(htmlPayedInterestByYearTable());
    }
    case BY_CONTRACT_END: {
        return html.arg(htmlContractsByContractEndTable());
        break;
    }
    case INTEREST_DISTRIBUTION: {
        return html.arg(htmlContractsByYearByInterestTable());
        break;
    }
    case CONTRACT_TERMS: {
        return html.arg(htmlContractsByRuntimeTable());
        break;
    }
    case ALL_CONTRACT_INFO: {
        return html.arg(htmlAllContractInfo());
        break;
    }
    default:
    {Q_ASSERT(false);}
    return QString();
    }
}

