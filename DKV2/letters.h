#ifndef LETTERS_H
#define LETTERS_H

#include "vertrag.h"
#include "letterTemplate.h"

void printThankyouLetter( const Contract& v, QSqlDatabase db = defaultDb());
void printTerminationLetter( const Contract& v, QDate kDate, int kMonate, QSqlDatabase db = defaultDb());
void printFinalLetter( const Contract& v, QDate contractEnd, QSqlDatabase db = defaultDb());

#endif // LETTERS_H
