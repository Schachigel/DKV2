#ifndef JAHRESABSCHLUSS_H
#define JAHRESABSCHLUSS_H

#include <QString>
#include <QVector>

#include "contract.h"

class jahresabschluss
{
public:
    jahresabschluss();
    bool execute();
    int abzuschliessendesJahr() const {return Jahr;}
    const QVector<contract>& getThesaV() const {return thesaV;}
    const QVector<contract>& get_nThesaV() const {return n_thesaV;}
    int jahr() const {return Jahr;}

private:
    int JahreszahlFuerAbschluss();
    int Jahr;
    QVector<contract> thesaV;
    QVector<contract> n_thesaV;
};


#endif // JAHRESABSCHLUSS_H
