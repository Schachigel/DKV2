#ifndef LETTERS_H
#define LETTERS_H

#include "vertrag.h"
#include "letterTemplate.h"

void printThankyouLetter( const Contract& v);
void printTerminationLetter( const Contract& v, QDate kDate, int kMonate);
void printFinalLetter( const Contract& v, QDate contractEnd);

#endif // LETTERS_H
