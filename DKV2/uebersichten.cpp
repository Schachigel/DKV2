#include "appconfig.h"
#include "dkdbhelper.h"
#include "uebersichten.h"

int tablelayout::colCount()
{
    if( _colCount not_eq -1)
        return _colCount;
    _colCount =cols.count();
    for(int i =0; i < sections.count(); i++) {
        int nbrOfDataColumns =sections[i].data.count();
        if( nbrOfDataColumns > _colCount)
            _colCount =nbrOfDataColumns;
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
        auto cp =table->cellAt(0, colIndex).firstCursorPosition();
        cp.insertHtml(qsl("<b>%1<b>").arg(cols[colIndex]));
    }
    return 1;
}

int tablelayout::fillSectionHeader(QTextTable* table, const int secIndex, const int row)
{
    section& sec =sections[secIndex];
    if( 0 == sec.header.count())
        return 0;

    table->mergeCells(row, 0, 1, table->columns()-1);
    table->cellAt(row, 0).firstCursorPosition().insertHtml(qsl("<b>%1<b>").arg(sec.header));
    return 1;
}

int tablelayout::fillSectionData(QTextTable* table, const int secIndex, const int row)
{
    section& sec =sections[secIndex];
    for( int rowInData =0; rowInData < sec.data.count(); rowInData++) {
        QStringList& rowData =sec.data[rowInData];
        for( int colInData =0; colInData < rowData.count(); colInData++) {
            auto cp =table->cellAt(row + rowInData, colInData).firstCursorPosition();
            cp.insertHtml(qsl("%1").arg(sec.data[rowInData][colInData]));
        }
    }
    return sec.data.count();
}

int tablelayout::fillEmptyRow(QTextTable* table, const int row)
{
    table->mergeCells(row, 0, 1, table->columns());
    return 1;
}

bool tablelayout::renderTable( QTextDocument* td)
{
    QTextCursor tc (td);
    tc.movePosition(QTextCursor::End);
    QTextTable* table =tc.insertTable(rowCount(), colCount());
    qDebug() << table->rows() << " / " << table->columns();
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
    // add Title, project info and current date
    td->clear();
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
//    tl.sections.push_back({qsl("Aktive Verträge"), overviewActiveContracts()});
//    tl.sections.push_back({qsl("Noch nicht aktive Verträge"), QVector<QStringList>()});

    tl.cols << qsl("col1") << qsl("col2") << qsl("col3");
    tablelayout::section sec1 {qsl("Abschnitt 1"), {{qsl("Data11"), qsl("Data12"), qsl("Data13")}, {qsl("Data21"), qsl("Data22"), qsl("Data23")}}};
    tl.sections.push_back(sec1);
    tablelayout::section sec2 {qsl("Abschnitt 2"), {{qsl("Data11"), qsl("Data12"), qsl("Data13")}, {qsl("Data21"), qsl("Data22"), qsl("Data23")}, {qsl("Data31"), qsl("Data32"), qsl("Data33")}}};
    tl.sections.push_back(sec2);

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
