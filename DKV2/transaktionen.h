#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "vertrag.h"

bool aktiviereVertrag(int vid);

bool beendeVertrag(int vid);
bool VertragsEnde_LaufzeitEnde( Contract& v);
bool VertragsEnde_MitKFrist( Contract& v);
bool VertragsEnde_PassiverV( Contract& v);

#endif // TRANSAKTIONEN_H
