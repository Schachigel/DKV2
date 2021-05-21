#ifndef CONTRACTTABLEMODEL_H
#define CONTRACTTABLEMODEL_H

#include <QSqlTableModel>
#include <QObject>

class ContractTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit ContractTableModel(QObject* p) : QSqlTableModel(p) {};
    QVariant data(const QModelIndex& index, int role) const override;
};

#endif // CONTRACTTABLEMODEL_H
