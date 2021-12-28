#ifndef CONTRACTTABLEMODEL_H
#define CONTRACTTABLEMODEL_H

#include <QSqlTableModel>
#include <QObject>

enum colmn_Pos {
    cp_vid,
    cp_Creditor_id,
    cp_Investment_id,
    cp_Creditor,
    cp_ContractLabel,
    cp_Comment,
    cp_ContractDate,
    cp_InitialBooking,
    cp_InterestActive,
    cp_ContractValue,
    cp_InterestRate,
    cp_InterestMode,
    cp_InterestBearing,
    cp_Interest,
    cp_ContractEnd,
    cp_colCount
};

enum column_pos_del {
    cp_d_vid,
    cp_d_Creditor_id,
    cp_d_Creditor,
    cp_d_ContractLabel,
    cp_d_ContractActivation,
    cp_d_ContractTermination,
    cp_d_InitialValue,
    cp_d_InterestRate,
    cp_d_InterestMode,
    cp_d_Interest,
    cp_d_TotalDeposit,
    cp_d_FinalPayout
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
