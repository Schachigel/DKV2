
#include "helper.h"
#include "helpersql.h"
#include "helperfile.h"
#include "csvwriter.h"

void csvwriter::addColumn(QString header)
{   LOG_CALL_W(header);
    Q_ASSERT(rows.empty()); // no add. columns after adding data
    header.replace(separator, "#");
    headers.append(header.trimmed());
}

int csvwriter::addColumns(QString headers)
{   LOG_CALL_W(headers);
    QList<QString> list = headers.split(separator);
    for(auto s : list)
    {
        addColumn(s);
    }
    return list.size();
}

void csvwriter::appendToRow( QString value)
{   LOG_CALL_W(value);
    value.replace(separator, "#");
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
    for( auto s : cols)
    {
        appendToRow(s);
    }
}

void csvwriter::addRow(QString row)
{   LOG_CALL_W(row);
    QList<QString> list = row.split(separator);
    addRow(list);
}

QString csvwriter::appendCsvLine( QString line, QString appendix)
{   LOG_CALL_W(line);
    if( line.size()) line += separator + " ";
    return line + appendix;
}

QString csvwriter::out()
{   LOG_CALL;
    QString out;
    for( auto i : headers)
    {
        out = appendCsvLine(out, i);
    }
    for( auto j : rows)
    {
        QString line;
        for( auto k : j)
        {
            line = appendCsvLine( line, k);
        }
        out += "\n" + line;
    }
    return out;
}

bool csvwriter::save(QString filename)
{   LOG_CALL_W(filename);
    backupFile(filename);
    QFile file(filename);
    if( !file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        qCritical() << "could not open csv file for writing: " << filename;
        return false;
    }
    QTextStream s(&file);
    s << out();
    return true;
}

bool table2csv(QString filename, QVector<dbfield> fields, QVector<QVariant::Type> types, QString where)
{    LOG_CALL;
    csvwriter csv (";");
    for(auto f : fields)
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
                csv.appendToRow(d.toString("dd.MM.yyyy"));
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
