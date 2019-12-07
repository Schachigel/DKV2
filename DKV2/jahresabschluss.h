#ifndef JAHRESABSCHLUSS_H
#define JAHRESABSCHLUSS_H

#include <QString>
#include <QVector>
#include "vertrag.h"

class jahresabschluss
{
public:
    jahresabschluss();
    bool execute();
    int abzuschliessendesJahr() const {return Jahr;}
    const QVector<Vertrag> getTesaV() const {return tesaV;}
    const QVector<Vertrag> get_nTesaV() const {return n_tesaV;}

private:
    int JahreszahlFuerAbschluss();
    int Jahr;
    QVector<Vertrag> tesaV;
    QVector<Vertrag> n_tesaV;
};


#endif // JAHRESABSCHLUSS_H
