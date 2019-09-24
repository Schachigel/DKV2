#include "itemformatter.h"

QString DateItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    QDate date= QDate::fromString(value.toString(), Qt::ISODate);
    return date.toString("dd.MM.yyyy");
};

QString PercentItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return value.toString()+"%";
};

QString EuroItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return value.toString()+" Euro";
};
