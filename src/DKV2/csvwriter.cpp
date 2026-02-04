#include "csvwriter.h"
#include "filewriter.h"

QString csvWriter::prepStringAsField(const QString& s)
{
    QString preped {(trim_fields == trim_input::remove_leading_and_trailing_whitespace) ? s.trimmed() : s};
    preped.replace(fieldDelimiter, fieldDelimiter+fieldDelimiter);
    if( preped.contains(fieldSeparator)
        || preped.contains(fieldDelimiter)
        || preped.contains(reWhiteSpace)
        || preped.contains(reLineBreak))
    {
        preped.prepend(fieldDelimiter);
        preped.append(fieldDelimiter);
    }
    return preped;
}

void csvWriter::addColumn(const QString& header)
{
    if (records.size()){
        qCritical() << "csvWriter::addColumn: a row can only be added before records were added";
        return;
    }
    headers.append({prepStringAsField(header)});
}

void csvWriter::addColumns(const QStringList HeadersToBeAdded)
{
    for(auto& s : std::as_const(HeadersToBeAdded))
        addColumn(s);
}

void csvWriter::appendValueToNextRecord(const QString& value)
{

    if( headers.isEmpty()) {
        // no headers- we can not check validity of records
        next.append({prepStringAsField(value)});
    } else {
        Q_ASSERT( next.size() < headers.size());
        // if headers are given, record size must not exceed headers size
        next.append({prepStringAsField(value)});
        if( next.size() == headers.size()){
            records.append(next);
            next.clear();
        }
    }
}

void csvWriter::startNextRecord()
{
    if( headers.size())
        qCritical() << "with a header row given, records are stored automatically when complete";
    records.append(next);
    next.clear();
}

void csvWriter::appendRecord(const QList<QString> record)
{
    Q_ASSERT( headers.size() > 0 // ONLY with NO header, there could be more data then headers
             && record.size() == headers.size());
    for (const auto& recordEntry : record ) {
        appendValueToNextRecord(recordEntry);
    }
}

bool atStartOfLine (const QString& c) {
    if( c.size() == 0) return true;
    if( c.endsWith("\r")
        or c.endsWith("\n")
        or c.endsWith(lineSeparator))
        return true;
    return false;
}

QString& csvWriter::appendFieldToString(QString& str, const csvField& newField) const
{
    if (atStartOfLine(str))
        if( newField.size())
            str += newField;
        else
            str += fieldSeparator;
    else
        str += fieldSeparator + newField;
    return str;
}
QString& csvWriter::appendRecordToString(QString& str, const csvRecord& newRecord) const
{
    if( newRecord.size() == 0) return str;

    if( str.size())
        str +=lineSeparator;

    QString thisLine;
    for( const auto& field : newRecord) {
        appendFieldToString(thisLine, field);
    }
    str += thisLine;
    return str;
}

QString csvWriter::toString() const
{
    QString Output;
    if( headers.size())
        appendRecordToString(Output, headers);
    if( records.size())
        for (const auto & record : records) {
            appendRecordToString(Output, record);
        }
    return Output;
}

bool StringLists2csv(const QString& filename, const QList<QString>& header, const QVector<QList<QString>>& data)
{
    LOG_CALL;
    qsizetype numColumns =header.size();
    csvWriter csv;
    csv.addColumns(header);
    for( auto& line : std::as_const(data)) {
        if(line.size() not_eq numColumns){
            qCritical() << "csv file not created due to wrong number of elements in " << line;
            return false;
        }
        csv.appendRecord(line);
    }
    return saveStringToUtf8File(filename, csv.toString()) && showInExplorer (filename);
}
