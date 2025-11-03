#include "pch.h"

#include "helperfile.h"
#include "appconfig.h"
#include "csvwriter.h"

void csvwriter::addColumn(const QString& header)
{   LOG_CALL_W(header);
    QString h(header);
    Q_ASSERT(rows.empty()); // no add. columns after adding data
    h.replace(separator, qsl(","));
    h.replace(lineBreak, qsl(" | "));
    headers.append(h.trimmed());
}

int csvwriter::addColumns(const QString& headers)
{   LOG_CALL_W(headers);
    QList<QString> list = headers.split(separator);
    return addColumns(list);
}

int csvwriter::addColumns(const QStringList headers)
{   LOG_CALL;
    for(auto& s : std::as_const(headers))
    {
        addColumn(s);
    }
    return headers.size();
}

void csvwriter::appendToRow(const QString& value)
{   //LOG_CALL_W(value);
    QString v(value);
    v.replace(separator, qsl(","));
    v.replace(lineBreak, qsl(" | "));
    currentRow.append(v.trimmed());
    if( currentRow.size() == headers.size()) {
        rows.append( currentRow);
        currentRow.clear();
    }
}

void csvwriter::addRow(const QStringList &cols)
{   //LOG_CALL;
    Q_ASSERT(cols.size() == headers.size());
    for( auto& s : std::as_const(cols)) {
        appendToRow(s);
    }
}

void csvwriter::addRow(const QString& row)
{   //LOG_CALL_W(row);
    QList<QString> list = row.split(separator);
    addRow(list);
}

QString csvwriter::appendCsvLine(const QString& line, const QString& appendix) const
{   //LOG_CALL;
    QString l(line);
    if( l.size()) l += separator + qsl(" ");
    return l + appendix;
}


QString csvwriter::toString() const
{   LOG_CALL;
    QString out;
    for( auto& i : std::as_const(headers)) {
        out = appendCsvLine(out, i);
    }
    for( auto& j : std::as_const(rows)) {
        QString line;
        for( auto& k : std::as_const(j))
        {
            line = appendCsvLine( line, k);
        }
        out += qsl("\n") + line;
    }
    return out;
}

bool csvwriter::saveAndShowInExplorer(const QString& proposedFileName) const
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

bool StringLists2csv(const QString& filename, const QStringList& header, const QVector<QStringList>& data)
{
    LOG_CALL;
    int numColumns =header.size();
    csvwriter csv (qsl(";"));
//    qDebug() << header;
    csv.addColumns(header);
    for( auto& line : std::as_const(data)) {
//        qDebug() << line;
        if(line.size() not_eq numColumns){
            qWarning() << "csv file not created due to wrong number of elements in " << line;
            return false;
        }
        csv.addRow(line);
    }
    return csv.saveAndShowInExplorer (filename);
}
