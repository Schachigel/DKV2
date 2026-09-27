#include "uiitemformatter.h"

#include "booking.h"
#include "contract.h"
#include "helperfin.h"

void centralAlignedTextFormatter::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex& i) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(p, alignedOption, i);
}

QString doFormatDateItem(const QVariant& value)
{
    QDate date= value.toDate();
    if( date == QDate() or date > QDate(9990, 12, 31))
        return value.toString();
    else
        return date.toString(qsl("dd.MM.yyyy"));
}
QString DateItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return doFormatDateItem(value);
};
void DateItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString doFormatPercentItem(const QVariant& value)
{
    double percent = value.toDouble();
    // data is stored as 100th percent but the query compensatates that
    return qsl("%1%").arg(percent, 2, 'f', 2);
}
QString PercentItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return doFormatPercentItem(value);
};
void PercentItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString PercentFrom100sItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    double percent = value.toInt() /100.;
    // data is stored as 100th percent but the query compensatates that
    return qsl("%1%").arg(percent, 2, 'f', 2);
};
void PercentFrom100sItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString CurrencyFormatter::displayText(const QVariant& value, const QLocale& )const
{
    if( not value.isValid() or value.isNull())
        return QString();
    QVariant vv(value);
    if( not vv.convert(QMetaType(QMetaType::Double)))
        return value.toString();
    double w =value.toDouble();
    return qsl(" %1 ").arg(s_d2euro(w));
};
void CurrencyFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignRight|Qt::AlignVCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

void BookingAmountItemFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignRight|Qt::AlignVCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString BookingAmountItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    if( not value.isValid() or value.isNull())
        return QString();
    double w = r2(value.toDouble()/100.);
    if( w <= 0)
        return "[" + s_d2euro(-1 *w) + " "  + "]";
    else
        return s_d2euro(w)+ " ";
};

QString bookingTypeFormatter::displayText(const QVariant &value, const QLocale &) const
{
    return bookingTypeDisplayString(bookingtypeFromInt(value.toInt()));
}
void bookingTypeFormatter::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if( not finalPayoutForTerminatedContracts
        or bookingtypeFromInt(index.data().toInt()) not_eq bookingType::payout)
        return;
    // only the payout on the termination date is final; the bookings view is ordered
    // by id DESC, so row 0 (column 2 = date) holds the contract's last booking
    const QDate lastBookingDate =index.model()->index(0, 2).data().toDate();
    if( index.siblingAtColumn(2).data().toDate() == lastBookingDate)
        option->text = qsl("Finale Auszahlung");
}
void bookingTypeFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}

QString interestModeFormatter::displayText(const QVariant &value, const QLocale &) const
{
    if( value.toString().toLower() == qsl("all"))
        return qsl("Alle");
    int index =value.toInt();
    if( index <0 or index >= toInt(interestModel::maxId)){
        qCritical() << "invalid interest model index: " << index;
        return qsl("Fehler");
    } else
        return interestModelDisplayString( interestModelFromInt(index));
}
void interestModeFormatter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem alignedOption(option);
    alignedOption.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, alignedOption, index);
}
