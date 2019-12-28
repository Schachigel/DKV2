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
    const QVector<Vertrag>& getThesaV() const {return thesaV;}
    const QVector<Vertrag>& get_nThesaV() const {return n_thesaV;}
    int jahr() const {return Jahr;}

private:
    int JahreszahlFuerAbschluss();
    int Jahr;
    QVector<Vertrag> thesaV;
    QVector<Vertrag> n_thesaV;
};


#endif // JAHRESABSCHLUSS_H
