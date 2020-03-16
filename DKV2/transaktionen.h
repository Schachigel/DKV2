#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "vertrag.h"

void aktiviereVertrag(int vid);

void beendeVertrag(int vid);
void VertragsEnde_LaufzeitEnde( Contract& v);
void VertragsEnde_MitKFrist( Contract& v);
void VertragsEnde_PassiverV( Contract& v);

#endif // TRANSAKTIONEN_H
