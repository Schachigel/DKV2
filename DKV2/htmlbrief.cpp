#include <QFile>
#include <QSettings>

#include "helper.h"
#include "filehelper.h"
#include "dkdbhelper.h"
#include "htmlbrief.h"

htmlTemplate::htmlTemplate(QString s) : tpl(s)
{
    if( s.isEmpty()) return;
    findPositions();
}

bool htmlTemplate::fromFile(QString filename)
{ LOG_ENTRY_and_EXIT;
    Q_ASSERT(!filename.isEmpty());
    QFile file( filename);
    if( !file.exists())
    {
        qCritical() << "fromFile: template file not found";
        return false;
    }
    file.open(QFile::OpenModeFlag::ReadOnly);
    tpl = file.readAll();
    if( tpl.isEmpty()) qWarning() << "html template could not be loaded";
    findPositions();
    return true;
}

void htmlTemplate:: findPositions()
{
    positions.clear();
    int open{0}, close{0};
    do
    {
        open = tpl.indexOf("[[", close);
        if( open == -1) break;
        close = tpl.indexOf("]]", open);
        if( close == -1) break;

        positions [tpl.mid(open, close +2 -open)] = "";
    } while(true);
}

bool htmlTemplate::setPositionText(QString pos, QString txt)
{
    if( pos.isEmpty()) return false;
    if( pos[0] != '[') pos = "[" +pos;
    if( pos[1] != '[') pos = "[" +pos;
    if( pos[pos.size()-1] != ']') pos += "]";
    if( pos[pos.size()-2] != ']') pos += "]";

    if( !positions.contains(pos))
    {
        qCritical() << "position " << pos << " *not* in template";
        return false;
    }
    if( !positions.value(pos).isEmpty())
    {
        qCritical() << "position " << pos  << "already set - resetting";
    }
    positions[pos] = txt;
    return true;
}

QString htmlTemplate::out ()
{
    QString out = tpl;
    qDebug() << positions;

    QMapIterator<QString, QString> iter(positions);
    while( iter.hasNext())
    {
        iter.next();
        out.replace(iter.key(), iter.value());
    }
    return out;
}
