
#include "helper.h"
#include "helpersql.h"
#include "helperfile.h"
#include "appconfig.h"
#include "csvwriter.h"

void csvwriter::addColumn(QString header)
{   LOG_CALL_W(header);
    Q_ASSERT(rows.empty()); // no add. columns after adding data
    header.replace(separator, qsl("#"));
    headers.append(header.trimmed());
}

int csvwriter::addColumns(QString headers)
{   LOG_CALL_W(headers);
    QList<QString> list = headers.split(separator);
    for(auto& s : qAsConst(list))
    {
        addColumn(s);
    }
    return list.size();
}

void csvwriter::appendToRow( QString value)
{   LOG_CALL_W(value);
    value.replace(separator, qsl("#"));
    currentRow.append(value.trimmed());
    if( currentRow.size() == headers.size())
    {
        rows.append( currentRow);
        currentRow.clear();
    }
}

void csvwriter::addRow(QList<QString> cols)
{   LOG_CALL;
    Q_ASSERT(cols.size() == headers.size());
    for( auto& s : qAsConst(cols))
    {
        appendToRow(s);
    }
}

void csvwriter::addRow(QString row)
{   LOG_CALL_W(row);
    QList<QString> list = row.split(separator);
    addRow(list);
}

QString csvwriter::appendCsvLine( QString line, QString appendix) const
{   LOG_CALL_W(line);
    if( line.size()) line += separator + qsl(" ");
    return line + appendix;
}


QString csvwriter::out() const
{   LOG_CALL;
    QString out;
    for( auto& i : qAsConst(headers))
    {
        out = appendCsvLine(out, i);
    }
    for( auto& j : qAsConst(rows))
    {
        QString line;
        for( auto& k : qAsConst(j))
        {
            line = appendCsvLine( line, k);
        }
        out += qsl("\n") + line;
    }
    return out;
}

bool csvwriter::save(const QString filename) const
{   LOG_CALL_W(filename);
    QString path {appConfig::Outdir() + qsl("/") + filename};
    backupFile(path);
    {
    QFile file(path);
    if( !file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qCritical() << "could not open csv file for writing: " << filename;
        return false;
    }
    QTextStream s(&file);
    s.setCodec("UTF-8");
    s.setGenerateByteOrderMark(true);
    s << out();
    }
    showFileInFolder(path);
    return true;
}

bool table2csv(const QString filename, const QVector<dbfield> fields, const QVector<QVariant::Type> types, const QString where)
{    LOG_CALL;
    csvwriter csv (qsl(";"));
    for(auto& f : qAsConst(fields))
        csv.addColumn(f.name());
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where);
    QSqlQuery q;
    if( !q.exec(sql)) {
        qCritical() << "sql faild to execute" << q.lastError() << Qt::endl << "SQL: " << q.lastQuery();
        return false;
    }
    QLocale locale;
    while( q.next()) {
        for (int i = 0; i < fields.count(); i++) {
            QVariant::Type t = types.isEmpty() ? fields[i].type() : types[i];
            switch(t) {
            case QVariant::Type::Int: {
                int v = q.record().value(fields[i].name()).toInt();
                csv.appendToRow(QString::number(v));
                break;
            }
            case QVariant::Type::String: {
                csv.appendToRow(q.record().value(fields[i].name()).toString());
                break;
            }
            case QVariant::Type::Date: {
                QDate d = q.record().value(fields[i].name()).toDate();
                csv.appendToRow(d.toString(qsl("dd.MM.yyyy")));
                break;
            }
            case QVariant::Type::Double: {
                double d = q.record().value(fields[i].name()).toDouble();
                csv.appendToRow(locale.toString(d, 'f', 2));
                break;
            }
            default:
                csv.appendToRow(q.record().value(fields[i].name()).toString());
            }
        }
    }

    return csv.save(filename);
}
