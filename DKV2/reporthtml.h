#ifndef REPORTHTML_H
#define REPORTHTML_H

#include <QString>

enum Uebersichten
{
    UEBERSICHT = 0,
    VERTRAGSENDE,
    ZINSVERTEILUNG,
    LAUFZEITEN
};

QString reportHtml(Uebersichten u);

#endif // REPORTHTML_H
