#ifndef DATEITEMFORMATTER_H
#define DATEITEMFORMATTER_H

#include <QObject>
#include <QString>
#include <QStyledItemDelegate>

class centralAlignedTextFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    centralAlignedTextFormatter(QObject *parent =nullptr) : QStyledItemDelegate(parent){}
//    QString displayText(const QVariant& value, const QLocale& )const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

QString doFormatDateItem(const QVariant& v);
class DateItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DateItemFormatter(QObject *parent =nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

QString doFormatPercentItem(const QVariant& v);
class PercentItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PercentItemFormatter(QObject *parent =nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class PercentFrom100sItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PercentFrom100sItemFormatter(QObject *parent =nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class BookingAmountItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BookingAmountItemFormatter(QObject *parent =nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class CurrencyFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CurrencyFormatter(QObject *parent =nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class bookingTypeFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    bookingTypeFormatter(QObject* p =nullptr) : QStyledItemDelegate(p){}
    QString displayText(const QVariant &value, const QLocale &locale) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class interestModeFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    interestModeFormatter(QObject* p =nullptr) : QStyledItemDelegate(p){}
    QString displayText(const QVariant &value, const QLocale &locale) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DATEITEMFORMATTER_H
