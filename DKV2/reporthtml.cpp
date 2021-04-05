#include <QDate>
#include <QLocale>

#include "appconfig.h"
#include "helper.h"
#include "dkdbhelper.h"
#include "dkdbviews.h"
#include "dbstatistics.h"
#include "reporthtml.h"

// statistics pages - Helper Fu
QString tag(const QString& string, const QString& tag, const QStringList& attribs =QStringList())
{
    QString ret(qsl("<")+tag);
    for(auto& s : qAsConst(attribs)) {
        ret += qsl(" ") +s;
    }
    return ret + qsl(">") + string + qsl("</") + tag + qsl(">");
}

QString tableRow4(const QString& left, const QString& center, const QString& center2, const QString& right)
{
    QString iLeft   =tag(left,    qsl("td"), {qsl("style='text-align: right;'")});
    QString iCenter =tag(center,  qsl("td"), {qsl("style='text-align: center;'")});
    QString iCenter2=tag(center2, qsl("td"), {qsl("style='text-align: center;'")});
    QString iRight  =tag(right,   qsl("td"), {qsl("style='text-align: left;'")});
    return tag( iLeft + iCenter + iCenter2 + iRight, "tr");
}
QString tableRow3( const QString& left, const QString& center, const QString& right)
{
    QString l = qsl("<td style='text-align: right;' >") + left   + qsl("</td>");
    QString c = qsl("<td style='text-align: center;'>") + center + qsl("</td>");
    QString r = qsl("<td style='text-align: left;'  >") + right  + qsl("</td>");
    return "<tr>" + l + c + r + "</tr>";
}
QString tableRow2(const QString& left, const QString& right)
{
    QString l = qsl("<td style='text-align: right;'>") + left  + qsl("</td>");
    QString r= qsl("<td style='text-align: left;' >") + right + qsl("</td>");
    return qsl("<tr>") + l + r + qsl("</tr>");
}
QString tableRow1(const QString& text, int colspan =2)
{
    QString opening(qsl("<tr><td style=\"background - color:rgb(215, 215, 210);\" colspan=\"%1\";>"));
    return  opening.arg(colspan) + text + qsl("</td></tr>");
}
QString emptyRow( int cols) {
//    QString ret =tag(qsl(""), qsl("tr"), {"style=\"background - color:rgb(215, 215, 210);\" colspan=\"%1\"; height: 50%;>"});
    QString ret = qsl("<tr><td colspan=%1 style='padding: 1px; font-size: small;'></td></tr>");
    ret =ret.arg(QString::number(cols));
    return ret;
}
QString emptyRow( )
{
    return qsl("<tr><td style='padding: 1px; font-size: small;'></td><td style='padding: 1px; font-size: small';></td></tr>");
}
QString b(const QString& b) {
    return tag(b, qsl("b"));
}
QString h2(const QString& v) {
    return tag(v, qsl("h2"));
}
QString h1(const QString& v) {
    return tag(v, qsl("h1"));
}
QString td(const QString& v)
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
QString newLine(const QString& line)
{
    return qsl("<br>") + line;
}

QString htmlOverviewTableBlock(const QString& headline, dbStats::dataset ds)
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

QString htmlShortInfoTable()
{
    QString ret;

    ret += h1(qsl("Übersicht DKs und DK Geber"));
    ret += dbConfig::readValue(GMBH_ADDRESS1).toString();
    ret += qsl(" - Stand: ") + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    dbStats stats(dbStats::calculate);
    ret += htmlOverviewTableBlock(qsl("Alle Aktiven Verträge"), stats.activeContracts[dbStats::t_nt]);
    ret += htmlOverviewTableBlock(qsl("Alle noch nicht aktiven Verträge"), stats.inactiveContracts[dbStats::t_nt]);
    ret += htmlOverviewTableBlock(qsl("Alle Verträge"), stats.allContracts[dbStats::t_nt]);
    return ret;
}


QString htmlOverviewTable()
{
    QString ret;

    ret += h1(qsl("Übersicht DKs und DK Geber"));
    ret += dbConfig::readValue(GMBH_ADDRESS1).toString();
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
    ret += dbConfig::readValue(GMBH_ADDRESS1).toString();
    ret += qsl(" - Stand: ") + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    ret += startTable();
    ret +=tableRow4(h2(qsl("Jahr")), h2(qsl("Zinstyp")), h2(qsl("Zinsmodus")), h2(qsl("Summe")));
    QVector<QSqlRecord> records;
    QLocale local;
    if( not executeSql(qsl("SELECT * FROM (%1)").arg(sqlInterestByYearOverview), QVariant(), records)){
        ret += tableRow4(qsl("fehler bei der Datenabfrage"), QString(), QString(), QString());
        ret += endTable();
        return ret;
    }
    int lastYear =-1;
    for( auto rec : records) {
        if( lastYear not_eq rec.value(0).toInt()) {
            ret += emptyRow(4);
        }
        ret += tableRow4(rec.value(0).toString(),
                         rec.value(2).toString(),
                         rec.value(3).toString(),
                         locale.toCurrencyString(rec.value(1).toDouble()));
        lastYear =rec.value(0).toUInt();
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
     if( not ce.isEmpty()) {
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
    calc_annualInterestDistribution( yzv);
    if( yzv.isEmpty())
        return ret;
    ret += h1(qsl("Verteilung der Zinssätze pro Jahr")) + qsl("<br> Stand:")  + QDate::currentDate().toString(qsl("dd.MM.yyyy<br>"));
    ret += startTable() +  startRow();
    ret += td(h2(qsl("Jahr"))) + td( h2(qsl("Zinssatz"))) +td(h2(qsl("Anzahl"))) + td( h2( qsl("Summe")));
    ret += endRow();
    int year =-1;
    for( auto& x: qAsConst(yzv)) {
        if( year not_eq x.year)
            ret += emptyRow(4);
        ret += tableRow4( QString::number(x.year), QString(qsl("%1%")).arg(x.intrest, 2, 'g'), QString::number(x.count), locale.toCurrencyString(x.sum));
        year =x.year;
    }
    ret += endTable();
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
//QString htmlAllContractInfo()
//{
//    dbtable t(vnContractsActiveDetailsView);
//    t.append(dbfield(qsl("Id"), QVariant::Type::Int));
//    t.append(dbfield(qsl("KreditorId"), QVariant::Type::Int));
//    t.append(dbfield(qsl("Vertragskennung")));
//    t.append(dbfield(qsl("Vorname")));
//    t.append(dbfield(qsl("Nachname")));
//    t.append(dbfield(qsl("Strasse")));
//    t.append(dbfield(qsl("Plz")));
//    t.append(dbfield(qsl("Stadt")));
//    t.append(dbfield(qsl("Email")));
//    t.append(dbfield(qsl("Iban")));
//    t.append(dbfield(qsl("Bic")));
//    t.append(dbfield(qsl("Strasse")));
//    t.append(dbfield(qsl("Zinssatz"), QVariant::Type::Double));
//    t.append(dbfield(qsl("Wert"), QVariant::Type::Double));
//    t.append(dbfield(qsl("Aktivierungsdatum"), QVariant::Type::Date));
//    t.append(dbfield(qsl("Kuendigungsfrist"), QVariant::Type::Int));
//    t.append(dbfield(qsl("Vertragsende"), QVariant::Type::Date));
//    t.append(dbfield(qsl("thesa"), QVariant::Type::Bool));

//    QString ret =startTable();
//    ret += startRow();
//    for(auto& field : t.Fields()) {
//        ret +=tag(field.name(), qsl("th"));
//    }
//    ret += endRow();
//    QVector<QSqlRecord> data = executeSql(t.Fields());
//    for(auto& rec : qAsConst(data)) {
//        ret += startRow();
//        for( int i =0; i<rec.count(); i++) {
//            ret += td(rec.field(i).value().toString());
//        }
//        ret += endRow();
//    }
//    ret += endTable();
//    return ret;
//}
QString reportHtml(Uebersichten u)
{   LOG_CALL;
    QString html =qsl("<html><body>"
                    "<style>"
                      "table { font-family: Verdana; font-size: medium; border: 1px solid black; border-collapse: collapse; }"
                      "td { background-color:rgb(235,235,235); padding: 6px; border: 1px solid black; }"
                      "tr { background-color: #f5f5f5; border: 1px solid black; }"
                    "</style>"
                    "%1"
                  "</body></html>");
    switch( u ) {
    case SHORTINFO: {
        return html.arg(htmlShortInfoTable());
    }
    case OVERVIEW: {
        return html.arg(htmlOverviewTable());
    }
    case PAYED_INTEREST_BY_YEAR: {
        return html.arg(htmlPayedInterestByYearTable());
    }
    case BY_CONTRACT_END: {
        return html.arg(htmlContractsByContractEndTable());
    }
    case INTEREST_DISTRIBUTION: {
        return html.arg(htmlContractsByYearByInterestTable());
    }
    case CONTRACT_TERMS: {
        return html.arg(htmlContractsByRuntimeTable());
    }
//    case ALL_CONTRACT_INFO: {
//        return html.arg(htmlAllContractInfo());
//    }
    default:
    {Q_ASSERT(false);}
    return QString();
    }
}

