#ifndef LETTERS_H
#define LETTERS_H

#include "vertrag.h"
#include "letterTemplate.h"

void printThankyouLetter( const Vertrag& v);
void printTerminationLetter( const Vertrag& v, QDate kDate, int kMonate);

#endif // LETTERS_H
