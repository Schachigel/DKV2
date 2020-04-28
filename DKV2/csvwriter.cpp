
#include <QtGlobal>
#include <QDebug>
#include <QString>
#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QSqlQuery>

#include "helper.h"
#include "sqlhelper.h"
#include "filehelper.h"
#include "csvwriter.h"

void csvwriter::addColumn(QString header)
{   LOG_CALL_W(header);
    Q_ASSERT(rows.empty()); // no add. columns after adding data
    header.replace(";", "#");
    headers.append(header.trimmed());
}

int csvwriter::addColumns(QString headers)
{   LOG_CALL_W(headers);
    QList<QString> list = headers.split(";");
    for(auto s : list)
    {
        addColumn(s);
    }
    return list.size();
}

void csvwriter::appendToRow( QString value)
{   LOG_CALL_W(value);
    value.replace(";", "#");
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
    QList<QString> list = row.split(";");
    addRow(list);
}

QString appendCsvLine( QString line, QString appendix)
{   LOG_CALL_W(line);
    if( line.size()) line += "; ";
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

bool table2csv(QVector<dbfield>& fields, QString where, QString filename, QSqlDatabase db)
{    LOG_CALL;
    csvwriter csv;
    for(auto f : fields)
        csv.addColumn(f.name());

    QString sql = SelectQueryFromFields(fields, where);
    QSqlQuery q (db);
    if( !q.exec(sql))
    {
        qCritical() << "sql faild to execute" << q.lastError() << endl << "SQL: " << q.lastQuery();
        return false;
    }

    while( q.next())
    {
        for (auto f: fields)
        {
            csv.appendToRow(q.record().value(f.name()).toString());
        }
    }

    return csv.save(filename);
}

bool table2csv(QVector<dbfield>& fields, QVector<QVariant::Type>& types, QString where, QString filename, QSqlDatabase db)
{    LOG_CALL;
    csvwriter csv;
    for(auto f : fields)
        csv.addColumn(f.name());

    QString sql = SelectQueryFromFields(fields, where);
    QSqlQuery q (db);
    if( q.exec(sql))
    {
        qCritical() << "sql faild to execute" << q.lastError() << endl << "SQL: " << q.lastQuery();
        return false;
    }

    while( q.next())
    {
        for (int i = 0; i < fields.count(); i++)
        {
            switch(types[i])
            {
            case QVariant::Type::Int:
            {
                int v = q.record().value(fields[i].name()).toInt();
                csv.appendToRow(QString::number(v));
                break;
            }
            case QVariant::Type::String:
            {
                csv.appendToRow(q.record().value(fields[i].name()).toString());
                break;
            }
            case QVariant::Type::Date:
            {
                QDate d = q.record().value(fields[i].name()).toDate();
                csv.appendToRow(d.toString("dd.MM.yyyy"));
                break;
            }
            case QVariant::Type::Double:
            {
                double d = q.record().value(fields[i].name()).toDouble();
                csv.appendToRow(QString::number(d, 'f', 2));
                break;
            }
            default:
                csv.appendToRow(q.record().value(fields[i].name()).toString());
            }
        }
    }

    return csv.save(filename);
}
