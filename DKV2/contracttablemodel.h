#ifndef CONTRACTTABLEMODEL_H
#define CONTRACTTABLEMODEL_H

#include <QSqlTableModel>
#include <QObject>

enum colmn_Pos {
    cp_vid,
    cp_Creditor_id,
    cp_Creditor,
    cp_ContractLabel,
    cp_Comment,
    cp_ContractDate,
    cp_ActivationDate,
    cp_ContractValue,
    cp_InterestRate,
    cp_InterestMode,
    cp_InterestBearing,
    cp_Interest,
    cp_LastBooking,
    cp_ContractEnd
};

class ContractTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit ContractTableModel(QObject* p) : QSqlTableModel(p) {};
    QVariant data(const QModelIndex& index, int role) const override;
    void setCol13ExtraData();
private:
    QMap<int, int> extraData;
};

#endif // CONTRACTTABLEMODEL_H
