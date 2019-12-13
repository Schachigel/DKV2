
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
    headers.append(header);
}

void csvwriter::appendToRow( QString value)
{
    currentRow.append(value);
    if( currentRow.size() == headers.size())
    {
        rows.append( currentRow);
        currentRow.clear();
    }
}

void csvwriter::addRow(QList<QString> cols)
{
    Q_ASSERT(cols.size() == headers.size());
    rows.append(cols);
}

QString appendCsvLine( QString line, QString appendix)
{
    if( line.size()) line += "; ";
    appendix = appendix.replace(';', '#');
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
