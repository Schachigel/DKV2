#include "helperfile.h"
#include "appconfig.h"
#include "csvwriter.h"


QString csvWriter::prepStringAsField(const QString& s)
{
    QString preped {(trim_fields == trim_input::remove_leading_and_trailing_whitespace) ? s.trimmed() : s};
    preped.replace(fieldDelimiter, fieldDelimiter+fieldDelimiter);
    if( preped.contains(fieldSeparator) || preped.contains(reWhiteSpace) || preped.contains(reLineBreak)) {
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
    Q_ASSERT( next.size() < headers.size());
    next.append({prepStringAsField(value)});
    if( next.size() == headers.size()){
        records.append(next);
        next.clear();
    }
}

void csvWriter::appendRecord(const QList<QString> record)
{
    Q_ASSERT( record.size() == headers.size());
    for (const auto& recordEntry : record ) {
        appendValueToNextRecord(recordEntry);
    }
}

QString& csvWriter::appendFieldToString(QString& line, const csvField& newField) const
{
    line += (line.size() && newField.size()) ? (fieldSeparator +newField) : newField; // there is already smthg there
    return line;
}
QString& csvWriter::appendRecordToString(QString& line, const csvRecord& newRecord) const
{
    if( line.size() && newRecord.size())
        line +=lineSeparator;

    QString thisLine;
    for( const auto& field : newRecord) {
        appendFieldToString(thisLine, field);
    }
    line += thisLine;
    return line;
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

bool csvWriter::saveAndShowInExplorer(const QString& proposedFileName) const
{   LOG_CALL_W(proposedFileName);
    QString fqFilePath {appConfig::Outdir() + qsl("/") + proposedFileName};
    moveToBackup (fqFilePath);

    QFile file(fqFilePath);
    if( not file.open(QIODevice::WriteOnly|QIODevice::Truncate))
        RETURN_ERR(false, qsl("could not open csv file for writing: "), proposedFileName);

    QTextStream s;
    s.setDevice (&file);
    s.setGenerateByteOrderMark(true);
    s << toString();

    showInExplorer(fqFilePath);
    return true;
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
    return csv.saveAndShowInExplorer (filename);
}
