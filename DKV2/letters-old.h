#ifndef LETTERS_OLD_H
#define LETTERS_OLD_H

#include "contract.h"
#include "appconfig.h"
#include "letterTemplate-old.h"

void printThankyouLetter( const contract& v);
void printTerminationLetter( const contract& v, QDate kDate, int kMonate);
void printFinalLetter( const contract& v, QDate contractEnd);

#endif // LETTERS_OLD_H
