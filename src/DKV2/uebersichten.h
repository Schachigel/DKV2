#ifndef UEBERSICHTEN_H
#define UEBERSICHTEN_H

#include <array>
#include <QGuiApplication>

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

    tablelayout(QTextDocument* td, const QPalette& palette);
// interface
    void renderTable();

// data
    QStringList cols;
    QVector<section> sections;
    QTextDocument* td;
private:
    void initFormats();
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
    QPalette palette;

    const bool bold =true;
    const bool regular =false;

    // font sizes in pt
    const int fontSize_colHeader =12;
    const int fontSize_secHeader =12;
    const int fontSize_data      =11;
    const int fontSize_emptyRow  =1;

    struct cellTypeFormat {
        int pointSize =0;
        bool bold =false;
        QColor fColor;
        QColor bColor;
        Qt::Alignment alignment =Qt::AlignLeft;
        int blockFormatIndex =0;
        int charFormatIndex  =0;
    };

    std::array<cellTypeFormat, maxCellType> tableformats;

};

struct uebersichten
{
    uebersichten(QTextDocument* td, const QPalette& palette = QGuiApplication::palette()) : td(td), palette(palette) {
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
    QPalette palette;
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
