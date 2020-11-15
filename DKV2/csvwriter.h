#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <QString>
#include <QList>

#include "helpersql.h"
#include "dbfield.h"

class csvwriter
{
public:
    csvwriter(QString sep =qsl(";")) : separator(sep){ Q_ASSERT(sep.length()==1);}
    void addColumn(QString header);
    int  addColumns(QString headers);
    void appendToRow(QString value);
    void addRow(QList<QString> cols);
    void addRow(QString row);

    QString out() const;
    bool saveAndShowInExplorer(const QString filname) const;
private:
    QString appendCsvLine( QString line, QString appendix) const;

    QString separator;
    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

bool table2csv(const QString filename, const QVector<dbfield> fields, const QVector<QVariant::Type> types =QVector<QVariant::Type>(), const QString where =QString());

#endif // CSVWRITER_H
