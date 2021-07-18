#include "appconfig.h"
#include "helperfin.h"
#include "dkdbviews.h"
#include "dkdbhelper.h"
#include "uebersichten.h"


void tablelayout::setCellFormat(QTextTableCell& cell, cellType ct) {
dbgTimer t (QStringLiteral("setFormat"));
    if(tableformats[ct].charFormatIndex) {
//        qDebug() << "using cf index: " << tableformats[ct].charFormatIndex;
        QTextCharFormat tcf =td->allFormats().at(tableformats[ct].charFormatIndex).toCharFormat();
        cell.setFormat(tcf);
        t.lab("cf from index");
    } else {
        QTextCharFormat tcf =cell.format();
        tcf.setFontPointSize(tableformats[ct].pointSize);
        tcf.setFontWeight(tableformats[ct].bold ? QFont::Bold : QFont::Normal);
        tcf.setForeground(tableformats[ct].fColor);
        tcf.setBackground(tableformats[ct].bColor);
        cell.setFormat(tcf);
//        qDebug() << "writing cf index: " << cell.tableCellFormatIndex();
        tableformats[ct].charFormatIndex = cell.tableCellFormatIndex();
        t.lab("new cf");
    }
    if(tableformats[ct].blockFormatIndex) {
//        qDebug() << "using bf index: " << tableformats[ct].blockFormatIndex;
        QTextBlockFormat tbf =td->allFormats().at(tableformats[ct].blockFormatIndex).toBlockFormat();
        cell.firstCursorPosition().setBlockFormat(tbf);
        t.lab("bf from index");
    } else {
        QTextBlockFormat bf =cell.firstCursorPosition().blockFormat();
        bf.setAlignment(tableformats[ct].alignment);
        cell.firstCursorPosition().setBlockFormat(bf);
//        qDebug() << "writing bf index: " << cell.firstCursorPosition().block().blockFormatIndex();
        tableformats[ct].blockFormatIndex =cell.firstCursorPosition().block().blockFormatIndex();
        t.lab("new bf");
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
        if( not sec.header.isEmpty())
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
        setCellFormat( cell, collHeader);
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
                else if( colInData == table->columns())
                    setCellFormat( cell, dataLastColOdd);
                else setCellFormat( cell, dataMiddleColOdd);
            } else {
                // even rows
                if(  colInData == 0)
                    setCellFormat( cell,dataFirstColEven);
                else if( colInData == table->columns())
                    setCellFormat( cell, dataLastColEven);
                else setCellFormat( cell, dataMiddleColEven);
            }
            cell.firstCursorPosition().insertText(sec.data[rowInData][colInData]);
        }
    }
    return sec.data.count();
}

int tablelayout::fillEmptyRow(QTextTable* table, const int row)
{   LOG_CALL_W(QString::number(row));
    QTextTableCell cell =table->cellAt(row, 0);
    setCellFormat( cell, emptyLine);
    table->mergeCells(row, 0, 1, table->columns());
    return 1;
}

bool tablelayout::renderTable( )
{   // LOG_CALL;
//    dbgTimer t(QStringLiteral("render table"));
    QTextCursor tc (td);
    tc.movePosition(QTextCursor::End);
    QTextTable* table =tc.insertTable(rowCount(), colCount());
    qDebug() << table->rows() << " / " << table->columns();
//    t.lab(QStringLiteral("table inserted"));
    int currentRow =insertColHeader(table);
//    t.lab(QStringLiteral("col header"));
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
        currentRow += fillSectionData(table, sectionIndex, currentRow);
//        t.lab("section filled");
    }

    qDebug() << table->format().columnWidthConstraints();
    return true;
}


///////////////////////////////////////////
// class uebersichten
///////////////////////////////////////////

QString uebersichten::titles[]
{
    qsl("Übersicht"),
    qsl("Vertragsabschlüsse nach Jahr und Zinssatz"),
    qsl("Auslaufende Verträge"),
    qsl("Verteilung der Zinssätze pro Jahr"),
    qsl("Laufzeiten")
};


void uebersichten::prep( uebersichten::uetype t)
{   LOG_CALL_W(QString::number(t));
    // set document defaults
    QFont f =td->defaultFont();
    f.setFamily(qsl("Verdana"));
    f.setPointSize(12);
    td->setDefaultFont(f);

    // add Title, project info and current date
    QTextCursor tc(td);
    tc.insertHtml(qsl("<h1>%1</h1><p>").arg(titles[t]));
    QString headerline =qsl("<big>%1</big> - Stand: %2<p>");
    //qsl("<div style=\"text-align:left; float:left\">%1</div> <div style=\"text-align:right;\">Stand: %2</div>");
    tc.insertHtml(headerline.arg( dbConfig::readString(GMBH_ADDRESS1), QDate::currentDate().toString(Qt::ISODate)));
}

void uebersichten::renderDocument( uebersichten::uetype t)
{   LOG_CALL_W(QString::number(t));
    prep(t);

    switch (t){
    case uetype::SHORTINFO:
        renderShortInfo();
        break;
    case uetype::PAYED_INTEREST_BY_YEAR:
        renderInterestByYear();
        break;
    case uetype::BY_CONTRACT_END:
        renderContractsByContractEnd();
        break;
    case uetype::INTEREST_DISTRIBUTION:
        renderInterestDistribution();
        break;
    case uetype::CONTRACT_TERMS:
        renderContractTerminations();
        break;
    default:
        Q_ASSERT(not "one shoule never come here");
        break;
    }
}

void uebersichten::renderShortInfo()
{   LOG_CALL;
    tablelayout tl(td);
    tl.sections.push_back({qsl("Aktive Verträge"), overviewContracts(sqlOverviewActiveContracts)});
    tl.sections.push_back({qsl("InAktive Verträge"), overviewContracts(sqlOverviewInActiveContracts)});
    tl.sections.push_back({qsl("Aktive und Inaktive Verträge"), overviewContracts(sqlOverviewAllContracts)});
    tl.renderTable();
}

void uebersichten::renderInterestByYear()
{   //LOG_CALL;
    QVector<QSqlRecord> records;
    if( not executeSql(sqlContractsByYearByInterest, QVariant(), records))
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
            d2euro(records[i].value(qsl("Summe")).toDouble()),
            records[i].value(qsl("Anzahl")).toString()});
    }
    if( not currentSec.header.isEmpty())
        tl.sections.push_back(currentSec);
    tl.renderTable();
}

void uebersichten::renderContractsByContractEnd()
{
    tablelayout tl (td);
    tl.cols =QStringList({"column!", "column 223", "another column"});
    tl.sections.push_back(tablelayout::section({qsl("Sec head"), {{qsl("Hallo"), qsl("Welt"), qsl("!")}}}));
    tl.sections.push_back(tablelayout::section({qsl("Sec sec Head"), {{qsl("Hallo"), qsl("Welt"), qsl("!")}}}));
    tl.renderTable();
}

void uebersichten::renderInterestDistribution()
{

}

void uebersichten::renderContractTerminations()
{

}
