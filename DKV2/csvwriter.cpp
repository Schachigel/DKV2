
#include <QtGlobal>
#include <QString>
#include <QList>
#include <QStringList>
#include <QTextStream>

#include "filehelper.h"
#include "csvwriter.h"

void csvwriter::addColumn(QString header)
{
    Q_ASSERT(rows.empty()); // no add. columns after adding data
    header.replace(";", "#");
    headers.append(header.trimmed());
}

int csvwriter::addColumns(QString headers)
{
    QList<QString> list = headers.split(";");
    for(auto s : list)
    {
        addColumn(s);
    }
    return list.size();
}

void csvwriter::appendToRow( QString value)
{
    value.replace(";", "#");
    currentRow.append(value.trimmed());
    if( currentRow.size() == headers.size())
    {
        rows.append( currentRow);
        currentRow.clear();
    }
}

void csvwriter::addRow(QList<QString> cols)
{
    Q_ASSERT(cols.size() == headers.size());
    for( auto s : cols)
    {
        appendToRow(s);
    }
}

void csvwriter::addRow(QString row)
{
    QList<QString> list = row.split(";");
    addRow(list);
}

QString appendCsvLine( QString line, QString appendix)
{
    if( line.size()) line += "; ";
    return line + appendix;
}

QString csvwriter::out()
{
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

void csvwriter::save(QString filename)
{
    backupFile(filename);
    QFile file(filename);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream s(&file);
    s << out();
}
