#ifndef HTMLBRIEF_H
#define HTMLBRIEF_H

#include <QString>
#include <QMap>


class htmlTemplate
{
public:
    htmlTemplate(QString s=QString());
    bool fromFile(QString fn); // read file into tpl
    const QStringList getPositions() const { return positions;}

private:
    QString tpl;
    QStringList positions;
    void findPositions();
};

//class htmlBrief
//{
//public:
//    htmlBrief(QString t): type(t){}
//    void setPositionText(QString Position, QString text);
//    bool loadFromDb();
//    bool loadFromFile();
//    bool isValid();
//    QString get();
//    QString resourceName()
//    { return "einsatztexte-" + type + ".txt";}
//    void parsePositions(QString s);

//private:
//    QString type;
//    htmlBriefTempalte temp;
//    // [[ANREDE]]
//    QMap<QString, QString> textPositions;
//    // <<Datum>>
//    QMap<QString, QString> textReplacements;

//// helper
//};

#endif // HTMLBRIEF_H
