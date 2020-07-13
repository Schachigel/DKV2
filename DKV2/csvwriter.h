#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <QString>
#include <QList>

#include "helpersql.h"
#include "dbfield.h"

class csvwriter
{
public:
    csvwriter(QString sep =";") : separator(sep){ Q_ASSERT(sep.length()==1);}
    void addColumn(QString header);
    int  addColumns(QString headers);
    void appendToRow(QString value);
    void addRow(QList<QString> cols);
    void addRow(QString row);

    QString out();
    bool save(QString filname);
private:
    QString appendCsvLine( QString line, QString appendix);

    QString separator;
    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

bool table2csv(QString filename, QVector<dbfield> fields, QVector<QVariant::Type> types =QVector<QVariant::Type>(), QString where ="");

#endif // CSVWRITER_H
