#ifndef DATEITEMFORMATTER_H
#define DATEITEMFORMATTER_H

#include <QObject>
#include <qstring.h>
#include <QDate>
#include <qstyleditemdelegate.h>

class DateItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DateItemFormatter(QObject *parent = nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
};

class PercentItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PercentItemFormatter(QObject *parent = nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
};

class EuroItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    EuroItemFormatter(QObject *parent = nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
};

class ActivatedItemFormatter : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ActivatedItemFormatter(QObject *parent = nullptr) : QStyledItemDelegate(parent){}
    QString displayText(const QVariant& value, const QLocale& )const override;
};


#endif // DATEITEMFORMATTER_H
