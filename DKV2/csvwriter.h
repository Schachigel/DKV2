#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <QString>
#include <QList>
#include <QStringList>

#include "dbfield.h"

class csvwriter
{
public:
    void addColumn(QString header);
    int  addColumns(QString headers);
    void appendToRow(QString value);
    void addRow(QList<QString> cols);
    void addRow(QString row);

    QString out();
    void save(QString filname);
private:
    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

void table2csv(QVector<dbfield>& fields, QString where, QString filename, QString con = "");

#endif // CSVWRITER_H
