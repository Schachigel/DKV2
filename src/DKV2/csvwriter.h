#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "helper_core.h"

static const QRegularExpression reLineBreak  {qsl("[\\n\\r]+")}; // all \r\n combinations
static const QRegularExpression reWhiteSpace {qsl("\\s+")};
static const QString defFieldSeparator {qsl(";")};
static const QString defFieldDelimiter {qsl("\"")};
static const QString lineSeparator     { qsl("\r\n")};

using csvField=QString;
using csvRecord=QList<csvField>;

enum trim_input {
    no_trimming =0,
    remove_leading_and_trailing_whitespace

};
enum quoteAlways {
    quote_where_necessary =0,
    quote_all_fields
};


class csvWriter
{
public:
    csvWriter(const QString& sep =defFieldSeparator,
              const QString& delim=defFieldDelimiter,
              const trim_input t =remove_leading_and_trailing_whitespace,
              const quoteAlways q =quote_where_necessary)
        : fieldSeparator(sep), fieldDelimiter(delim), trim_fields(t), quoteMode(quote_where_necessary)
    { Q_ASSERT(sep.length()>=1); Q_ASSERT(sep.length()>=1);
    }
public:
    // specify the columns in the table (csv). These names will make the first record
    void addColumn(const QString& fieldname);
    void addColumns(const QList<QString>fieldnames);
    void appendValueToNextRecord(const QString& value);
    void startNextRecord();
    void appendRecord(const QList<QString> record);

    QString& appendFieldToString(QString& l, const csvField& f) const;
    QString& appendRecordToString(QString& line, const csvRecord& newRecord) const;
    QString toString() const;

    bool saveAndShowInExplorer(const QString& filname) const;
private:
    // const data / configuration
    const QString fieldSeparator/*{qsl(";")}*/;
    const QString fieldDelimiter/*{qsl("\"")}*/;
    const trim_input trim_fields/*{remove_leading_and_trailing_whitespace}*/;
    const quoteAlways quoteMode/*{quote_where_necessary}*/;
    // variabel data
    csvRecord headers;
    csvRecord next;
    QList<csvRecord> records;
private:
    // functions
    // make sure to write any data into csv, so that it can be read from most csv reade and ms excel
    QString prepStringAsField(const QString& s);

};

bool StringLists2csv(const QString& filename, const QStringList& header, const QVector<QStringList>& lists);

#endif // CSVWRITER_H
