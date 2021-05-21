#include "helper.h"
#include "contracttablemodel.h"

namespace {
QDate dateFromNP(const QString& s)
{
    QStringList lines =s.split(qsl("\xA"), Qt::SkipEmptyParts);
    QString dateStr;
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

QVariant ContractTableModel::data(const QModelIndex& index, int role) const
{
    static int days =daysUntilTheEndOfTheFuckingWorld;
    if( index.column() == 13 ) {
        if (role == Qt::FontRole) {
            days =daysToContractEnd(QSqlTableModel::data(index, Qt::DisplayRole).toString());
            if( 90 > days ) {
                QFont f =qvariant_cast<QFont>(QSqlTableModel::data(index, Qt::FontRole));
                f.setBold(true);
                return QVariant::fromValue(f);
            }
        }
        if( role == Qt::TextColorRole) {
            if( 30 > days)
                return QVariant::fromValue(QColor(Qt::red));
        }
    }
    return QSqlTableModel::data(index, role);
}
