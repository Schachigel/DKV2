#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "contract.h"

void aktiviereVertrag(int vid);

void beendeVertrag(int vid);
void VertragsEnde_LaufzeitEnde( contract& v);
void VertragsEnde_MitKFrist( contract& v);
void VertragsEnde_PassiverV( contract& v);

#endif // TRANSAKTIONEN_H
