#ifndef CSVWRITER_H
#define CSVWRITER_H
#include "helper_core.h"

inline constexpr QChar defFieldSeparator = u';';
inline constexpr QChar defQuotingChar = u'"';
inline const   QString lineSeparator = qsl("\r\n");

using csvField=QString;
using csvRecord=QList<csvField>;

class CsvWriter
{
public:
    enum class TrimInput {
        NoTrimming,
        TrimWhitespace
    };

    enum class QuoteMode {
        WhereNecessary,
        AllFields
    };

public:
    CsvWriter(const QChar& fieldSeperator =defFieldSeparator,
              const QChar& quoting=defQuotingChar,
              const TrimInput t =TrimInput::TrimWhitespace,
              const QuoteMode q =QuoteMode::WhereNecessary)
        : fieldSeparator(fieldSeperator), quotingChar(quoting), trim_fields(t), quoteMode(q)
    {}
public:
    // specify the columns in the table (csv). These names will make the first record
    void addColumn(const QString& fieldname);
    void addColumns(const QList<QString>& fieldnames);
    void appendValueToNextRecord(const QString& value);
    void startNextRecord();
    void appendRecord(const QList<QString>& record);

    QString toString() const;
private:
    // const data / configuration
    const QChar fieldSeparator/*{qsl(";")}*/;
    const QChar quotingChar/*{qsl("\"")}*/;
    const TrimInput trim_fields/*{remove_leading_and_trailing_whitespace}*/;
    const QuoteMode quoteMode/*{quote_where_necessary}*/;
    // variabel data
    csvRecord headers;
    csvRecord next;
    QList<csvRecord> records;
private:
    // functions
    // make sure to write any data into csv, so that it can be read from most csv reade and ms excel
    QString  prepStringAsField(const QString& s);
    QString& appendFieldToString(QString& l, const csvField& f, bool firstInLine) const;
    QString& appendRecordToString(QString& line, const csvRecord& newRecord) const;
};

QString sql_To_CsvString(QString Sql);


#endif // CSVWRITER_H
