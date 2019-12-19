#include <QFile>
#include "htmlbrief.h"

QString htmlbrief::htmlTemplate;

const QString htmlbrief::getTemplate()
{
    if( htmlTemplate.isEmpty())
    {
        QFile file(":/res/letter.html");
        htmlTemplate = QString(file.readAll());
    }
    return htmlTemplate;
}

