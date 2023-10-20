
#include "helpersql.h"
#include "helperfin.h"
#include "appconfig.h"
#include "dkdbviews.h"
#include "dkdbhelper.h"
#include "uebersichten.h"


void tablelayout::setCellFormat(QTextTableCell& cell, cellType ct) {
    if(tableformats[ct].charFormatIndex) {
        QTextCharFormat tcf =td->allFormats().at(tableformats[ct].charFormatIndex).toCharFormat();
        cell.setFormat(tcf);
    } else {
        QTextCharFormat tcf =cell.format();
        tcf.setFontPointSize(tableformats[ct].pointSize);
        tcf.setFontWeight(tableformats[ct].bold ? QFont::Bold : QFont::Normal);
        tcf.setForeground(tableformats[ct].fColor);
        tcf.setBackground(tableformats[ct].bColor);
        cell.setFormat(tcf);
        tableformats[ct].charFormatIndex = cell.tableCellFormatIndex();
    }
    if(tableformats[ct].blockFormatIndex) {
        QTextBlockFormat tbf =td->allFormats().at(tableformats[ct].blockFormatIndex).toBlockFormat();
        cell.firstCursorPosition().setBlockFormat(tbf);
    } else {
        QTextBlockFormat bf =cell.firstCursorPosition().blockFormat();
        bf.setAlignment(tableformats[ct].alignment);
        cell.firstCursorPosition().setBlockFormat(bf);
        tableformats[ct].blockFormatIndex =cell.firstCursorPosition().block().blockFormatIndex();
    }
}

int tablelayout::colCount()
{
    if( _colCount not_eq -1)
        return _colCount;
    _colCount =cols.count();
    for( auto const & sec : qAsConst(sections)) {
        for( auto const & dataset : qAsConst(sec.data)) {
            int nbrOfDataCols =dataset.count();
            if( nbrOfDataCols > _colCount)
                _colCount =nbrOfDataCols;
        }
    }
    return _colCount;
}

int tablelayout::rowCount()
{
    if( _rowCount not_eq -1)
        return _rowCount;
    _rowCount =0;
    // Spaltenüberschriften
    if( cols.count() > 0)
        _rowCount += 2; // header and separator line
    // Sektionen
    for(int i =0; i < sections.count(); i++) {
        section& sec =sections[i];
        if( sec.header.size())
            _rowCount += 1;
        // data per section
        _rowCount += sec.data.count();
    }
    // section spacing: no empty line for the last setction
    _rowCount += (sections.count() > 1) ? sections.count()-1 : 0;
    return _rowCount;
}

int tablelayout::insertColHeader(QTextTable* table)
{   LOG_CALL;
    if( 0 == cols.count())
        return 0;

    QTextTableFormat ttf =table->format();
    ttf.setHeaderRowCount(1);
    table->setFormat(ttf);

    for( int colIndex =0; colIndex < cols.count(); colIndex++) {
        QTextTableCell cell =table->cellAt(0, colIndex);
        setCellFormat( cell, colHeader);
        cell.firstCursorPosition().insertText(cols[colIndex]);
    }

    fillEmptyRow(table, 1);
    return 2;
}

int tablelayout::fillSectionHeader(QTextTable* table, const int secIndex, const int row)
{   LOG_CALL;
    section& sec =sections[secIndex];
    if( 0 == sec.header.count())
        return 0;
    table->mergeCells(row, 0, 1, table->columns());
    QTextTableCell cell =table->cellAt(row, 0);
    setCellFormat( cell, sectionHeader);
    cell.firstCursorPosition().insertText(sec.header);
    return 1;
}

int tablelayout::fillSectionData(QTextTable* table, const int secIndex, const int row)
{   LOG_CALL;
    section& sec =sections[secIndex];
    for( int rowInData =0; rowInData < sec.data.count(); rowInData++) {
        QStringList& rowData =sec.data[rowInData];
        for( int colInData =0; colInData < rowData.count(); colInData++) {
            QTextTableCell cell =table->cellAt(row + rowInData, colInData);
            if( rowInData%2) {
                // odd rows
                if(  colInData == 0)
                    setCellFormat( cell,dataFirstColOdd);
                else if( colInData == table->columns()-1)
                    setCellFormat( cell, dataLastColOdd);
                else setCellFormat( cell, dataMiddleColOdd);
            } else {
                // even rows
                if(  colInData == 0)
                    setCellFormat( cell,dataFirstColEven);
                else if( colInData == table->columns()-1)
                    setCellFormat( cell, dataLastColEven);
                else setCellFormat( cell, dataMiddleColEven);
            }
            cell.firstCursorPosition().insertHtml(sec.data[rowInData][colInData]);
        }
    }
    return sec.data.count();
}

int tablelayout::fillSectionDataWithSubsections(QTextTable* table, const int secIndex, const int row)
{   LOG_CALL;
    section& sec =sections[secIndex];
    for( int rowInData =0; rowInData < sec.data.count(); rowInData++) {
        QStringList& rowData =sec.data[rowInData];
        if( rowData[0] == MAGIC_SUBHEADER_MARKER) {
            QTextTableCell cell =table->cellAt(row + rowInData, 0);
            setCellFormat( cell,sectionHeader);
            table->mergeCells (row +rowInData, 0, 1, table->columns ());
            cell.firstCursorPosition().insertHtml(sec.data[rowInData][1]);
            continue;
        }
        for( int colInData =0; colInData < rowData.count(); colInData++) {
            QTextTableCell cell =table->cellAt(row + rowInData, colInData);
            if( rowInData%2) {
                // odd rows
                if(  colInData == 0)
                    setCellFormat( cell,dataFirstColOdd);
                else if( colInData == table->columns()-1)
                    setCellFormat( cell, dataLastColOdd);
                else setCellFormat( cell, dataMiddleColOdd);
            } else {
                // even rows
                if(  colInData == 0)
                    setCellFormat( cell,dataFirstColEven);
                else if( colInData == table->columns()-1)
                    setCellFormat( cell, dataLastColEven);
                else setCellFormat( cell, dataMiddleColEven);
            }
            cell.firstCursorPosition().insertHtml(sec.data[rowInData][colInData]);
        }
    }
    return sec.data.count();
}

int tablelayout::fillEmptyRow(QTextTable* table, const int row)
{   // LOG_CALL_W(i2s(row));
    QTextTableCell cell =table->cellAt(row, 0);
    setCellFormat( cell, emptyLine);
    table->mergeCells(row, 0, 1, table->columns());
    return 1;
}

void tablelayout::renderTable( )
{   // LOG_CALL;
    QTextCursor tc (td);
    tc.movePosition(QTextCursor::End);
    if( (not (rowCount() and colCount()))
            or (0 == sections.count())){
        qInfo() << "not rendering empty table";
        return;
    }
    QTextTable* table =tc.insertTable(rowCount(), colCount());
    int currentRow =insertColHeader(table);
    QTextTableFormat format =table->format();
    format.setCellSpacing(0);
    format.setCellPadding(5);
    format.setAlignment(Qt::AlignLeft);
    table->setFormat(format);
    // format section(s) header and data
    for (int sectionIndex =0; sectionIndex < sections.count(); sectionIndex++) {
        if( sectionIndex > 0)
            currentRow += fillEmptyRow(table, currentRow);
        currentRow += fillSectionHeader(table, sectionIndex, currentRow);
        if( sections[sectionIndex].hasSubsections)
            currentRow += fillSectionDataWithSubsections(table, sectionIndex, currentRow);
        else
            currentRow += fillSectionData(table, sectionIndex, currentRow);
    }
    return;
}

///////////////////////////////////////////
// class uebersichten
///////////////////////////////////////////
void uebersichten::prep( const QString& head, const QString& desc)
{   LOG_CALL;
    // set document defaults
    QFont f =td->defaultFont();
    f.setFamily(qsl("Verdana"));
    td->setDefaultFont(f);

    // add Title, project info and current date
    QTextCursor tc(td);
    tc.insertHtml(qsl("<h2>%1</h2><p>").arg(head));
    QString headerline =qsl("<big>%1</big> - Stand: %2<p>");
    //qsl("<div style=\"text-align:left; float:left\">%1</div> <div style=\"text-align:right;\">Stand: %2</div>");
    tc.insertHtml(headerline.arg( dbConfig::readString(GMBH_ADDRESS1), QDate::currentDate().toString(Qt::ISODate)));
    tc.insertHtml(qsl("<br>%1<br>").arg(desc));
}

void uebersichten::renderDocument( uebersichten::uetype t)
{   // LOG_CALL_W(i2s(t));
    switch (t){
    case uetype::SHORTINFO:
        renderShortInfo();
        break;
    case uetype::PAYED_INTEREST_BY_YEAR:
        renderPayedInterestByYear();
        break;
    case uetype::BY_CONTRACT_ENDING:
        renderContractsByContractEnd();
        break;
    case uetype::INTEREST_DISTRIBUTION:
        renderInterestDistribution();
        break;
    case uetype::CONTRACT_RUNTIME_DISTRIB:
        renderContractRuntimeDistrib();
        break;
    case uetype::PERPETUAL_INVESTMENTS_CHECK_BY_CONTRACTS:
        renderPerpetualInvestmentsCheckContracts ();
        break;
    case uetype::PERPETUAL_INVESTMENTS_CHECK_BY_BOOKINGS:
        renderPerpetualInvestmentsCheckBookings ();
        break;

//    case uetype::CONTRACT_by_interest_By_Year
//        renderContractsByInterestByYear();
//        break
    default:
        Q_ASSERT(not "one shoule never come here");
        break;
    }
}

void uebersichten::renderShortInfo()
{   LOG_CALL;
    QString head (qsl("Übersicht"));
    QString describe(qsl(R"str(
Die Kurzinfo gibt einen Überblick über grundlegende Informationen zu den Direktkrediten.
Für <i>aktive<i> Verträge läuft bereits die Verzinsung. Bei <i>inaktiven</i> Verträgen
steht die Einzahlung durch die Kreditgeber*in noch aus.
<br>Zinswerte sind hier ungenau, da der Zeitpunkt einer Ein- oder Auszahlung nicht berücksichtigt wird.
)str"));
    prep(head, describe);
    tablelayout tl(td);
    tl.sections.push_back({qsl("Aktive und Inaktive Verträge"), overviewShortInfo(sqlOverviewAllContracts)});
    tl.sections.push_back({qsl("Aktive Verträge"), overviewShortInfo(sqlOverviewActiveContracts)});
    tl.sections.push_back({qsl("InAktive Verträge"), overviewShortInfo(sqlOverviewInActiveContracts)});
    tl.renderTable();
}

void uebersichten::renderPayedInterestByYear()
{   LOG_CALL;
    QString head {qsl("Ausgezahlte Zinsen pro Jahr")};
    QString desc {qsl("Die Liste zeigt für jedes Jahr, welche Zinsen ausbezahlt oder, für thesaurierende Verträge und Verträge ohne Zinsauszahlung, angerechnet wurden.")};
    prep(head, desc);
    QVector<QSqlRecord> records;
    if( not executeSql( sqlInterestByYearOverview, records)) {
        return;
    }
    tablelayout tl(td);
    // tl.cols = QStringList...)
    tablelayout::section currentSec;
    currentSec.hasSubsections =true;
    QString subsection;
    for( int i=0; i <records.count(); i++) {
        QString curYear = records[i].value(qsl("Year")).toString();
        if( currentSec.header not_eq curYear) {
            // change of year -> save complte section, ...
            if( i not_eq 0) tl.sections.push_back(currentSec);
            // ... and start new section
            currentSec.header =curYear;
            currentSec.data.clear();
            subsection =records[i].value(qsl("BA")).toString();
            currentSec.data.push_back ({MAGIC_SUBHEADER_MARKER, subsection});
        }
        if( subsection not_eq records[i].value(qsl("BA")).toString()) {
            subsection =records[i].value(qsl("BA")).toString();
            currentSec.data.push_back ({MAGIC_SUBHEADER_MARKER, subsection});
        }
        if (records[i].value(qsl("Summe")).toDouble() != 0.0) {
            currentSec.data.push_back(
                {
//                    records[i].value(qsl("BA")).toString(),
                        records[i].value(qsl("Thesa")).toString(),
                        s_d2euro(records[i].value(qsl("Summe")).toDouble())
                });
        }
    }
    if( currentSec.data.count() or currentSec.header.count())
        tl.sections.push_back(currentSec);
    tl.renderTable();
}

void uebersichten::renderContractsByContractEnd()
{
    QString head {qsl("Auslaufende Verträge")};
    QString desc {qsl("Anzahl und Wert der Verträge, die in den kommenden Jahren enden.")};
    prep(head, desc);

    tablelayout tl (td);
    tl.cols =QStringList({qsl("Jahr"), qsl("Anzahl"), qsl("Summe")});

    QVector<contractEnd_rowData> data;
    calc_contractEnd(data);
    tablelayout::section sec;
    for( int i =0; i < data.count(); i++) {
        sec.data.push_back(QStringList({i2s(data[i].year),
                                        i2s(data[i].count),
                                       s_d2euro(data[i].value)}));
    }
    if( sec.data.count())
        tl.sections.push_back(sec);
    tl.renderTable();
}

void uebersichten::renderInterestDistribution()
{   //LOG_CALL;
    QString head(qsl("Anzahl Verträge nach Zinssatz und Jahr"));
    QString desc(qsl("Anahl und Wert der abgeschlossene Verträge nach Kalenderjahren."));
    prep( head, desc);

    QVector<QSqlRecord> records;
    if( not executeSql(sqlContractsByYearByInterest, records))
        return;
    tablelayout tl(td);
    tl.cols =QStringList({qsl("Zinssatz"), qsl("Summe"), qsl("Anzahl")});

    tablelayout::section currentSec;
    for( int i=0; i<records.count(); i++) {
        QString year =records[i].value(qsl("Year")).toString();
        if( currentSec.header not_eq year) {
            // store filled SECTION only for i>0
            if( i not_eq 0) tl.sections.push_back(currentSec);
            // start new section
            currentSec.header =year;
            currentSec.data.clear();
        }
        currentSec.data.push_back({
                                      prozent2prozent_str(records[i].value(qsl("Zinssatz")).toDouble()),
                                      s_d2euro(records[i].value(qsl("Summe")).toDouble()),
                                      records[i].value(qsl("Anzahl")).toString()});
    }
    if( currentSec.header.size())
        tl.sections.push_back(currentSec);
    tl.renderTable();
}

void uebersichten::renderContractRuntimeDistrib()
{
    QString head {qsl("Laufzeitenverteilung")};
    QString desc{qsl("Anzahl und Wert der Verträge nach ihrer Laufzeit")};
    prep( head, desc);
    QVector<contractRuntimeDistrib_rowData> data =contractRuntimeDistribution();
    tablelayout tl(td);
    tl.cols =QStringList({"", qsl("Anzahl"), qsl("Volumen")});
    tablelayout::section sec;
    for (int i=0; i< data.size(); i++) {
        sec.data.push_back(QStringList({data[i].text, data[i].number, data[i].value}));
    }
    tl.sections.push_back(sec);
    tl.renderTable();
}

void uebersichten::renderPerpetualInvestmentsCheckContracts()
{
    QString head {qsl("Prüfung der Grenzwerte ('100.000er Regel') für fortlaufende Geldanlagen anhand der Vertragswerte")};
    QString desc {qsl("Diese Tabelle gibt für fortlaufende Geldanlagen die Sumnme der Vertragswerte (ohne Zinsen!) im Jahr vor dem jeweiligen Vertragsabschluß an. "
                     "Es werden auch bereits beendete Verträge berücksichtigt, sofern sie in diesem Zeitraum abgeschlossen wurden.")};
    prep(head, desc);
    tablelayout tl(td);
    tl.cols =QStringList{qsl("Vert.\nDatum"), qsl("Vertrags-\nkennung"), qsl("Anzahl neu"), qsl("Vertragswert(e)"), qsl("Summe der\nletzten 12M") };

    QVector<QStringList> data =perpetualInvestmentByContracts ();
    if( data.isEmpty())
        return;
    QString investment;
    tablelayout::section curSec;
    for( int i=0; i< data.length (); i++) {
        if( data[i][0] not_eq investment) {
            // new section starts here
            if( i>0) {
                tl.sections.push_back (curSec);
                curSec.data.clear ();
            }
            curSec.header =investment =data[i][0];
        }
        QStringList row;
        for (int j=1; j<data[i].length (); j++) {
            row.append (data[i][j]);
        }
        curSec.data.append(row);
    }
    if( curSec.data.size ())
        tl.sections.push_back (curSec);
    tl.renderTable ();
}

void uebersichten::renderPerpetualInvestmentsCheckBookings()
{
    QString head {qsl("Prüfung der Grenzwerte ('100.000er Regel') für fortlaufende Geldanlagen anhand der gemachten Buchungen")};
    QString desc {qsl("Diese Tabelle gibt für fortlaufende Geldanlagen die Summe aller Buchungswerte an. Es werden alle Ein- und Auszahlungen, Jahresendzinsen und unterjährige Zinsen "
                     "im Jahr vor der jeweiligen Buchung aufsummiert. Für Verträge, deren initiale Einzahlung noch aussteht, wird der Vertragswert zum Vertragsdatum verwendet. "
                     "Es werden auch Buchungen von bereits beendete Verträge berücksichtigt, sofern in diesen Zeitraum fallen.")};

    prep(head, desc);
    tablelayout tl(td);
    tl.cols =QStringList{qsl("Datum"), qsl("Anzahl\nBuchungen"), qsl("Wert der Buchungen"), qsl("Gesamtwert der\nGeldanlage")};

    QVector<QStringList> data =perpetualInvestment_bookings();
    if( data.isEmpty())
        return;
    QString investment;
    tablelayout::section curSec;
    for( int i=0; i< data.length (); i++) {
        if( data[i][0] not_eq investment) {
            // new section starts here
            if( i>0) {
                tl.sections.push_back (curSec);
                curSec.data.clear ();
            }
            curSec.header =investment =data[i][0];
        }
        QStringList row;
        for (int j=1; j<data[i].length (); j++) {
            row.append (data[i][j]);
        }
        curSec.data.append(row);
    }
    if( curSec.data.size ())
        tl.sections.push_back (curSec);
    tl.renderTable ();
}
