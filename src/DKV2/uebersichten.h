#ifndef UEBERSICHTEN_H
#define UEBERSICHTEN_H

const QString MAGIC_SUBHEADER_MARKER ="!!__--SUBHEADER--__!!";

struct tablelayout
{
// types
    struct section {
        QString header;
        QVector<QStringList> data;
        bool hasSubsections =false;
    };
    enum cellType {
        colHeader,
        emptyLine,
        sectionHeader,
        dataFirstColOdd,
        dataMiddleColOdd,
        dataLastColOdd,
        dataFirstColEven,
        dataMiddleColEven,
        dataLastColEven,
        maxCellType
    };

    tablelayout(QTextDocument* td) : td (td) {};
// interface
    void renderTable();

// data
    QStringList cols;
    QVector<section> sections;
    QTextDocument* td;
private:
    int colCount();
    int rowCount();
    int insertColHeader(QTextTable* tt);
    int fillSectionHeader(QTextTable* tt, const int secIndex, const int row);
    int fillSectionData(QTextTable* tt, const int secIndex, const int row);
    int fillSectionDataWithSubsections(QTextTable* tt, const int secIndex, const int row);
    int fillEmptyRow(QTextTable* tt, const int row);
    void setCellFormat(QTextTableCell& cell, cellType ct);
private: // data
    qsizetype _colCount =-1;
    qsizetype _rowCount =-1;
    QVector<int> optColWidth;

    const bool bold =true;
    const bool regular =false;

    // font sizes in pt
    const int fontSize_colHeader =12;
    const int fontSize_secHeader =12;
    const int fontSize_data      =11;
    const int fontSize_emptyRow  =1;

    struct cellTypeFormat {
        const int pointSize;
        const bool bold;
        const QColor fColor;
        const QColor bColor;
        const Qt::Alignment alignment;
        int blockFormatIndex =0;
        int charFormatIndex  =0;
    };

    cellTypeFormat tableformats[maxCellType] ={
    /*colHeader*/      {fontSize_colHeader, bold,    Qt::black, QColor(200, 200, 240), Qt::AlignCenter, 0, 0},
    /*emptyLine*/      {fontSize_emptyRow,  regular, Qt::white, Qt::white,             Qt::AlignCenter, 0, 0},
    /*sectionHeader*/  {fontSize_secHeader, bold,    Qt::black, QColor(240, 210, 210), Qt::AlignLeft  , 0, 0},
    /*dataFirstColOdd*/{fontSize_data,      regular, Qt::black, QColor(230, 230, 230), Qt::AlignLeft  , 0, 0},
    /*dataMid.ColOdd*/ {fontSize_data,      regular, Qt::black, QColor(230, 230, 230), Qt::AlignCenter, 0, 0},
    /*dataLastColOdd*/ {fontSize_data,      regular, Qt::black, QColor(230, 230, 230), Qt::AlignRight , 0, 0},
    /*dataFirstColEv*/ {fontSize_data,      regular, Qt::black, QColor(245, 245, 245), Qt::AlignLeft  , 0, 0},
    /*dataMid.ColEv*/  {fontSize_data,      regular, Qt::black, QColor(245, 245, 245), Qt::AlignCenter, 0, 0},
    /*dataLastColEv*/  {fontSize_data,      regular, Qt::black, QColor(245, 245, 245), Qt::AlignRight , 0, 0}
    /*subsection head*/
    };

};

struct uebersichten
{
    uebersichten(QTextDocument* td) : td(td) {
        td->clear();
        td->setUndoRedoEnabled(false);
    };
    uebersichten() =delete;
    enum uetype {
        SHORTINFO =0,
        PAYED_INTEREST_BY_YEAR,
        BY_CONTRACT_ENDING,
        INTEREST_DISTRIBUTION,
        CONTRACT_RUNTIME_DISTRIB,
        PERPETUAL_INVESTMENTS_CHECK_BY_CONTRACTS,
        PERPETUAL_INVESTMENTS_CHECK_BY_BOOKINGS
    };
    //static int fromUeType(uetype t) { return static_cast<int>(t);}
    static uetype fromInt(int i) { return static_cast<uetype>(i);}
    void renderDocument( uetype t);
private:
    QTextDocument* td;
    void prep(const QString& head, const QString& desc);
    void renderShortInfo();
    void renderPayedInterestByYear();
    void renderContractsByContractEnd();
    void renderInterestDistribution();
    void renderContractRuntimeDistrib();
    void renderPerpetualInvestmentsCheckContracts();
    void renderPerpetualInvestmentsCheckBookings();
//    void renderContractsByInterestByYear();
private: /*static*/
    static QString titles[];
};

#endif // UEBERSICHTEN_H
