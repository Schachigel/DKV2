#include "helper.h"
#include "contracttablemodel.h"

namespace {
QDate dateFromNP(const QString& s)
{
    /* the format from the sql is
     * (x Monate)
     * or
     * dd.mm.yyyy
     * or
     * (x Monate) /n dd.mm.yyy
     */

    QStringList lines =s.split(qsl("\xA"), Qt::SkipEmptyParts);
    QString dateStr;
    if( 0 == lines.count()) {
        qWarning() << "empty contract termination string";
        return QDate();
    }
    if( 1 == lines.count()){
        if( lines[0].startsWith(qsl("(")))
            return QDate();
        else dateStr =lines[0];
    } else {
        dateStr =lines[1];
    }
    return QDateTime::fromString(dateStr, qsl("dd.MM.yyyy")).date();
}
int daysToContractEnd(const QString& s)
{
    QDate d =dateFromNP(s);
    if( d.isValid())
        return QDate::currentDate().daysTo(d);
    else
        return daysUntilTheEndOfTheFuckingWorld;
}
} // namespace

void ContractTableModel::setCol13ExtraData()
{
    for( int row=0; row< rowCount(); row++) {
        int cid =data(index( row, 0), Qt::DisplayRole).toInt();
        int dtce =daysToContractEnd(data(index(row, cp_ContractEnd), Qt::DisplayRole).toString());
        extraData.insert(cid, dtce);
    }
}

QVariant ContractTableModel::data(const QModelIndex& index, int role) const
{
    if( role == Qt::UserRole) {
        return extraData.value(data(index.siblingAtColumn(0), Qt::DisplayRole).toInt());
    }
    if (role == Qt::FontRole) {
        int days =data(index, Qt::UserRole).toInt();
        if( 90 > days ) {
            QFont f =qvariant_cast<QFont>(QSqlTableModel::data(index, Qt::FontRole));
            f.setBold(true);
            return QVariant::fromValue(f);
        }
    }
    if( role == Qt::TextColorRole) {
        int days =data(index, Qt::UserRole).toInt();
        if( 30 > days)
            return QVariant::fromValue(QColor(Qt::red));
    }
    return QSqlTableModel::data(index, role);
}
