#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "helper.h"

static const QRegularExpression lineBreak (qsl("[\\n\\r]+")); // all \r\n combinations

class csvwriter
{
public:
    csvwriter(const QString& sep =qsl(";")) : separator(sep){ Q_ASSERT(sep.length()==1);}
    void addColumn(const QString& header);
    qsizetype addColumns(const QString& headers);
    qsizetype addColumns(const QStringList headers);
    void appendToRow(const QString& value);
    void addRow(const QStringList& cols);
    void addRow(const QString& row);

    QString toString() const;
    bool saveAndShowInExplorer(const QString& filname) const;
private:
    QString appendCsvLine( const QString& line, const QString& appendix) const;

    QString separator;

    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

bool StringLists2csv(const QString& filename, const QStringList& header, const QVector<QStringList>& lists);

#endif // CSVWRITER_H
