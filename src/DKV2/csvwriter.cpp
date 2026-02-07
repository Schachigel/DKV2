#include "csvwriter.h"
#include <qassert.h>
//#include "filewriter.h"

QString CsvWriter::prepStringAsField(const QString& s)
{
    QString out = (trim_fields == TrimInput::TrimWhitespace)
    ? s.trimmed() : s;

    // explizit: leeres Feld
    if (out.isEmpty())
        return QString(2,quotingChar); // ""

    const bool hasQuote = out.contains(quotingChar);
    if (hasQuote)
        out.replace(quotingChar, QString(2, quotingChar));

    const bool hasOuterSpace =
        (trim_fields == TrimInput::NoTrimming) &&
        !out.isEmpty() &&
        (out.front().isSpace() || out.back().isSpace());

    const bool mustQuote =
        quoteMode == QuoteMode::AllFields ||
        hasQuote ||
        out.contains(fieldSeparator) ||
        out.contains('\n') ||
        out.contains('\r') ||
        hasOuterSpace;   // <- add this

    if (mustQuote) {
        out.prepend(quotingChar);
        out.append(quotingChar);
    }

    return out;
}

void CsvWriter::addColumn(const QString& header)
{
    if (records.size()){
        qCritical() << "CsvWriter::addColumn: a row can only be added before records were added";
        return;
    }
    headers.append({prepStringAsField(header)});
}

void CsvWriter::addColumns(const QList<QString>& HeadersToBeAdded)
{
    for(auto& s : std::as_const(HeadersToBeAdded))
        addColumn(s);
}

void CsvWriter::appendValueToNextRecord(const QString& value)
{
    if( headers.isEmpty()) {
        // no headers- we can not check validity of records
        next.append({prepStringAsField(value)});
    } else {
        if( next.size() >= headers.size()) {
            qCritical() << "appendValueToNextRecord: mixed header / non header mode csv? this should not be possible";
            Q_UNREACHABLE();
            return;
        }
        // if headers are given, record size must not exceed headers size
        next.append({prepStringAsField(value)});
        if( next.size() == headers.size()) {
            records.append(next);
            next.clear();
        }
    }
}

void CsvWriter::startNextRecord()
{
    if( headers.size()) {
        qCritical() << "with a header row given, records are stored automatically when complete";
        return;
    }
    records.append(next);
    next.clear();
}

void CsvWriter::appendRecord(const QList<QString>& record)
{
    if( headers.size() <= 0 || record.size() == headers.size()) {
        for (const auto& recordEntry : record ) {
            appendValueToNextRecord(recordEntry);
        }
        return;
    }
    qCritical() << "appendRecord: mixed header / non header mode csv? this should not be possible";
    Q_UNREACHABLE();
    return;
}

QString& CsvWriter::appendFieldToString(QString& str, const csvField& newField, bool firstInLine) const
{
    if (!firstInLine)
        str += fieldSeparator;

    return str += newField;
}

QString& CsvWriter::appendRecordToString(QString& str, const csvRecord& newRecord) const
{
    if( newRecord.size() == 0) return str;

    if( str.size())
        str +=lineSeparator;

    QString thisLine;
    bool firstInLine =true;
    for( const auto& field : newRecord) {
        appendFieldToString(thisLine, field, firstInLine);
        firstInLine =false;
    }
    str += thisLine;
    return str;
}

QString CsvWriter::toString() const
{
    QString Output;
    if( headers.size())
        appendRecordToString(Output, headers);
    if( records.size())
        for (const auto & record : records) {
            appendRecordToString(Output, record);
        }
    if( next.size())
        appendRecordToString(Output, next);
    return Output;
}

QString sqltableToCsvString(QString sql, QVector<QPair<QString, QVariant>> params ={});
