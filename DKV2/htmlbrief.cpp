#include <QFile>
#include <QSettings>

#include "filehelper.h"
#include "dkdbhelper.h"
#include "htmlbrief.h"

htmlTemplate::htmlTemplate(QString s) : tpl(s)
{
    if( s.isEmpty()) return;
    findPositions();
}

void htmlTemplate:: findPositions()
{
    int open{0}, close{0};
    do
    {
        open = tpl.indexOf("[[", close);
        if( open == -1) break;
        close = tpl.indexOf("]]", open);
        if( close == -1) break;

        positions.append( tpl.mid(open, close +2 -open));
    } while(true);
}


//const QString htmlBriefTempalte::get()
//{
//    if( htmlTemplate.isEmpty())
//    {
//        QFile file(":/res/letter.html");
//        htmlTemplate = file.readAll();
//    }
//    return htmlTemplate;
//}

//const QStringList htmlBriefTempalte::getPositions()
//{
//    const QString& s = get();
//    QStringList ret;
//    do
//    {
//        int open{0}, close{0};
//        open = s.indexOf("[[");
//        if( open == -1) break;
//        close = s.indexOf("]]", open);
//        if( close == -1) break;
//        ret.append( s.mid(open, close));
//    } while(true);
//    return ret;
//}

//void htmlBrief::parsePositions(QString s)
//{
//    QStringList sl = s.split("\n", QString::SkipEmptyParts);
//    QString Position;
//    QString PositionText;
//    for( QString line: sl)
//    {
//        line = line.trimmed();
//        if (line.startsWith("[["))
//        {   // new Position
//            if( Position.isEmpty())
//            {   // first position
//                Position = line;
//            }
//            else
//            {   // save current position
//                textPositions[Position] = PositionText;
//                PositionText.clear();
//            }
//            continue;
//        }
//        if( !PositionText.isEmpty()) PositionText += "\n";
//        PositionText += line;
//    }
//}

//bool htmlBrief::loadFromDb()
//{
//    QString s = Eigenschaft(resourceName()).toString();
//    if( s.isEmpty()) return false;
//    parsePositions(s);
//    return true;
//}

//bool htmlBrief::loadFromFile()
//{
//    QFile file(getDbFolder()+ "\\" +resourceName());
//    QString content = file.readAll();
//    if( content.isEmpty()) return false;
//    parsePositions(content);
//    return true;
//}

//bool htmlBrief::isValid()
//{
//    // not empty?
//    // all Positions of 'temp' resolved? -> no more '[['
//    // all Replacements in all Positions resolved? -> no more '<<'
//    return true;
//}

//QString htmlBrief::get()
//{
//    // repalce replacements in positions
//    // replace positions in template
//    // return ...
//    return QString();
//}
