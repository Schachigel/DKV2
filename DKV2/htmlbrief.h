#ifndef HTMLBRIEF_H
#define HTMLBRIEF_H

#include <QString>
#include <QMap>


class htmlTemplate
{
public:
    htmlTemplate(QString s=QString());
    bool fromFile(QString fn); // read file into tpl
    bool setPositionText(QString pos, QString txt);
    void reset() { findPositions();}
    QString out();
    const QMap<QString, QString> Positions() const { return positions;}

private:
    QString tpl;
    QMap<QString, QString> positions;
    void findPositions();
};

#endif // HTMLBRIEF_H
