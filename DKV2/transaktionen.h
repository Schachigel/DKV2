#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "contract.h"

void newCreditorAndContract();

void editCreditor(qlonglong creditorId);
void changeContractComment(contract* c);
void changeContractTermination(contract* c);

void changeContractDate(contract* v);
void changeContractLabel(contract* v);
void changeInitialPaymentDate(contract* v);
void receiveInitialBooking(contract* v);
void activateInterest(contract* v);
void changeContractValue(contract* v);
void annualSettlement();
QVariantList getContractList(qlonglong creditorId, QDate startDate, QDate endDate, bool isTerminated);
void annualSettlementLetters();
void finalizeContractLetter(contract *c);
void deleteFinalizedContract(contract *c);

void deleteInactiveContract(contract *c);
void terminateContract(contract* c);
void terminateContract_Final( contract& v);
void cancelContract( contract& v);

qlonglong createInvestment_matchingContract(int& interest/*inout*/, QDate& from /*inout*/, QDate& to /*out*/);
void createInvestment();

#endif // TRANSAKTIONEN_H
