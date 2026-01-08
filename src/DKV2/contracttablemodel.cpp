#include "contracttablemodel.h"
#include "helper.h"
#include "contract.h"

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList lines =s.split(qsl("\xA"), Qt::SkipEmptyParts);
#else
    QStringList lines =s.split(qsl("\xA"), QString::SkipEmptyParts);
#endif

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
    qint64 retval =-1;
    if( d.isValid())
        retval= QDate::currentDate().daysTo(d);
    else
        retval= daysUntilTheEndOfTheFuckingWorld;
//    qDebug() << "daysToContractEnd: " << s << ", " << retval;
    return (int) retval;
}
} // namespace

void ContractTableModel::setCol13ExtraData()
{
    int maxRow =rowCount();
    if( maxRow ==256)
        while (canFetchMore()) fetchMore();
    maxRow =rowCount();
    for( int row =0; row < maxRow; row++) {
        int cid =data(index( row, 0), Qt::DisplayRole).toInt();
        int dtce =daysToContractEnd(data(index(row, cp_ContractEnd), Qt::DisplayRole).toString());
        extraData.insert(cid, dtce);
//        qDebug() << "extra data: " << cid << ", " << dtce;
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
    if( role == Qt::ForegroundRole) {
        int days =data(index, Qt::UserRole).toInt();
        if( 30 > days)
            return QVariant::fromValue(QColor(Qt::red));
    }
    return QSqlTableModel::data(index, role);
}

bool ContractProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{

    if (left.column() == cp_ContractEnd) {
        int leftDtce = sourceModel()->data(left, Qt::UserRole).toInt();
        int rightDtce = sourceModel()->data(right, Qt::UserRole).toInt();
        QString leftString = sourceModel()->data(left, Qt::DisplayRole).toString();
        QString rightString = sourceModel()->data(right, Qt::DisplayRole).toString();

        static QRegularExpression re(qsl("\\((\\d+) *Monate\\)"));
        if (leftDtce == rightDtce) {
            int leftMonth = re.match(leftString).captured(1).toInt();
            int rightMonth = re.match(rightString).captured(1).toInt();
            return leftMonth < rightMonth;
        }
        else
            return leftDtce < rightDtce;
    }
    else if (left.column() == cp_ContractValue ||
             left.column() == cp_InterestBearing ||
             left.column() == cp_Interest )
    {
        QString leftString = sourceModel()->data(left, Qt::DisplayRole).toString();
        QString rightString = sourceModel()->data(right, Qt::DisplayRole).toString();
        static QRegularExpression re(qsl("([\\.,\\d]+)"));
        double leftValue = re.match(leftString).captured(1).toDouble();
        double rightValue = re.match(rightString).captured(1).toDouble();

        return leftValue < rightValue;
    }
    else
    {
        QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
        QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);
        if (leftData.metaType() != rightData.metaType())
            return false;

        switch (leftData.metaType().id())
        {
        case QMetaType::QString:
            return leftData.toString() < rightData.toString();
        case QMetaType::Double:
            return leftData.toDouble() < rightData.toDouble();
        case QMetaType::Int:
            return leftData.toInt() < rightData.toInt();
        case QMetaType::UInt:
            return leftData.toUInt() < rightData.toUInt();
        case QMetaType::LongLong:
            return leftData.toLongLong() < rightData.toLongLong();
        case QMetaType::ULongLong:
            return leftData.toULongLong() < rightData.toULongLong();
        case QMetaType::QDate :
            return leftData.toDate() < rightData.toDate();
        case QMetaType::QDateTime:
            return leftData.toDateTime() < rightData.toDateTime();
        default:
            return false;
        }
    }
}
