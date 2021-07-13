#include "appconfig.h"
#include "dkdbhelper.h"
#include "uebersichten.h"

const bool bold =true;
const bool regular =false;

void setTextTableCellFontPointSize(QTextTableCell& cell, int pointsize, bool bold)
{
    QFont f =cell.format().font();
    f.setPointSize(pointsize);
    f.setBold(bold);
    QTextCharFormat format =cell.format();
    format.setFont(f);
    cell.setFormat(format);
}

void setTextTableCellAlignment(QTextTableCell& cell, Qt::Alignment a)
{
    QTextBlockFormat bf =cell.firstCursorPosition().blockFormat();
    bf.setAlignment(a);
    cell.firstCursorPosition().setBlockFormat(bf);
}

void setTextTableCellBackColor(QTextTableCell& cell, QColor color)
{
    auto format =cell.format();
    format.setBackground(color);
    cell.setFormat(format);
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
    _rowCount += sections.count() -1;
    return _rowCount;
}

int tablelayout::insertColHeader(QTextTable* table)
{
    if( 0 == cols.count())
        return 0;

    QTextTableFormat ttf =table->format();
    ttf.setHeaderRowCount(1);
    for( int colIndex =0; colIndex < cols.count(); colIndex++) {
        QTextTableCell cell =table->cellAt(0, colIndex);
        setTextTableCellFontPointSize(cell, fontSize_colHeader, bold);
        setTextTableCellAlignment(cell, Qt::AlignCenter);
        setTextTableCellBackColor(cell, QColor(200, 200, 240));
        auto cp =cell.firstCursorPosition();
        cp.insertText(cols[colIndex]);
    }
    fillEmptyRow(table, 1);

    return 2;
}

int tablelayout::fillSectionHeader(QTextTable* table, const int secIndex, const int row)
{
    section& sec =sections[secIndex];
    if( 0 == sec.header.count())
        return 0;
    table->mergeCells(row, 0, 1, table->columns());
    QTextTableCell cell =table->cellAt(row, 0);
    setTextTableCellFontPointSize(cell, fontSize_secHeader, bold);
    setTextTableCellAlignment(cell, Qt::AlignLeft);
    setTextTableCellBackColor(cell, QColor(240, 210, 210));
    cell.firstCursorPosition().insertText(sec.header);
    return 1;
}

Qt::Alignment alignmentByCol(int col, int max)
{
    if( col == 0)
        return Qt::AlignLeft;
    if( col == max)
        return Qt::AlignRight;
    return Qt::AlignCenter;
}

int tablelayout::fillSectionData(QTextTable* table, const int secIndex, const int row)
{
    section& sec =sections[secIndex];
    for( int rowInData =0; rowInData < sec.data.count(); rowInData++) {
        QStringList& rowData =sec.data[rowInData];
        for( int colInData =0; colInData < rowData.count(); colInData++) {
            QTextTableCell cell =table->cellAt(row + rowInData, colInData);
            setTextTableCellFontPointSize(cell, fontSize_data, regular);
            setTextTableCellAlignment(cell, alignmentByCol(colInData, _colCount -1));
            if( rowInData%2)
                setTextTableCellBackColor(cell, QColor(230, 230, 230));
            else
                setTextTableCellBackColor(cell, QColor(245, 245, 245));
            auto cp =cell.firstCursorPosition();
            cp.insertText(sec.data[rowInData][colInData]);
        }
    }
    return sec.data.count();
}

int tablelayout::fillEmptyRow(QTextTable* table, const int row)
{
    table->mergeCells(row, 0, 1, table->columns());
    QTextTableCell cell =table->cellAt(row, 0);
    setTextTableCellFontPointSize(cell, fontSize_emptyRow, regular);
    return 1;
}

bool tablelayout::renderTable( QTextDocument* td)
{
    QTextCursor tc (td);
    tc.movePosition(QTextCursor::End);
    QTextTable* table =tc.insertTable(rowCount(), colCount());
    qDebug() << table->rows() << " / " << table->columns();

    QTextTableFormat format =table->format();
    format.setBorderCollapse(true);
    format.setCellPadding(5);
    format.setAlignment(Qt::AlignLeft);
    table->setFormat(format);

    QTextFrameFormat frameFormat =table->frameFormat();
    frameFormat.setWidth(400);
    table->setFrameFormat(frameFormat);

    // format and insert column header
    int currentRow =insertColHeader(table);

    // format section(s) header and data
    for (int sectionIndex =0; sectionIndex < sections.count(); sectionIndex++) {
        if( sectionIndex > 0)
            currentRow += fillEmptyRow(table, currentRow);
        currentRow += fillSectionHeader(table, sectionIndex, currentRow);
        currentRow += fillSectionData(table, sectionIndex, currentRow);
    }
    return true;
}

QString uebersichten::titles[]
{
    qsl("Übersicht"),
    qsl("Ausgezahlte / Angerechnete Zinsen nach Jahr"),
    qsl("Auslaufende Verträge"),
    qsl("Verteilung der Zinssätze pro Jahr"),
    qsl("Laufzeiten")
};


void uebersichten::prep( uebersichten::uetype t)
{
    td->clear();
    // set document defaults
    QFont f =td->defaultFont();
    f.setFamily(qsl("Verdana"));
    f.setPixelSize(12);
    td->setDefaultFont(f);

    // add Title, project info and current date
    QTextCursor tc(td);
    tc.insertHtml(qsl("<h1>%1</h1><p>").arg(titles[t]));
    QString headerline =qsl("<big>%1</big> - Stand: %2<p>");
    //qsl("<div style=\"text-align:left; float:left\">%1</div> <div style=\"text-align:right;\">Stand: %2</div>");
    tc.insertHtml(headerline.arg( dbConfig::readString(GMBH_ADDRESS1), QDate::currentDate().toString(Qt::ISODate)));
}

void uebersichten::renderDocument( uebersichten::uetype t)
{
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
    tablelayout tl;

    tl.sections.push_back({qsl("Aktive Verträge"), overviewActiveContracts()});
    tl.renderTable(td);
}

void uebersichten::renderInterestByYear()
{

}

void uebersichten::renderContractsByContractEnd()
{

}

void uebersichten::renderInterestDistribution()
{

}

void uebersichten::renderContractTerminations()
{

}
