#ifndef DLGINTERESTBREAKDOWN_H
#define DLGINTERESTBREAKDOWN_H

#include <QDialog>

class QLabel;
class QTableWidget;
class QPushButton;
class QDialogButtonBox;

#include "contract.h"

class dlgInterestBreakdown : public QDialog
{
public:
    explicit dlgInterestBreakdown(const QString& creditorName,
                                  const QString& contractLabel,
                                  contractId_t contractId,
                                  const QDate& bookingDate,
                                  const contract::interestBreakdown& breakdown,
                                  QWidget* parent = nullptr);

private:
    static QString sliceKindText(contract::interestSlice::kind kind);
    static QString modeText(contract::midYearInterestMode mode);
    QString toCsv() const;
    QString toText() const;
    void saveToFile();

    QString creditorName;
    QString contractLabel;
    contractId_t contractId = Invalid_contract_id;
    QDate bookingDate;
    contract::interestBreakdown breakdown;

    QLabel* headerLabel = nullptr;
    QLabel* summaryLabel = nullptr;
    QLabel* errorLabel = nullptr;
    QTableWidget* table = nullptr;
    QPushButton* saveButton = nullptr;
    QDialogButtonBox* buttons = nullptr;
};

#endif // DLGINTERESTBREAKDOWN_H
