#ifndef LETTERS_H
#define LETTERS_H

#include "contract.h"
#include "appconfig.h"
#include "letterTemplate.h"

void printThankyouLetter( const contract& v);
void printTerminationLetter( const contract& v, QDate kDate, int kMonate);
void printFinalLetter( const contract& v, QDate contractEnd);

#endif // LETTERS_H
