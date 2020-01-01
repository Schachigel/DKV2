#include <qdebug.h>
#include "itemformatter.h"

QString DateItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    QDate date= QDate::fromString(value.toString(), Qt::ISODate);
    if( date == QDate() || date > QDate(9990, 12, 31))
        return "";
    else
        return date.toString("dd.MM.yyyy");
};

QString PercentItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return value.toString()+"%";
};

QString EuroItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    return QString("%L1").arg(value.toDouble())+" Euro";
};

QString WertEuroItemFormatter::displayText(const QVariant& value, const QLocale& )const
{
    double d = value.toDouble();
    if( d != 0.)
        return QString("%L1").arg(value.toDouble())+" Euro";
    else
        return QString("(auszahlend)");
};

QString KFristItemFormatter ::displayText(const QVariant &value, const QLocale &) const
{
    int v = value.toInt();
    if( v == -1)
        return "(festes Laufzeitende)";
    else
        return QString("%L1 Monate").arg(v);
}

QString ActivatedItemFormatter::displayText(const QVariant &value, const QLocale &) const
{
    // the view delivers strings like "true" and "false" for boolean values
    // let the variant resolve this ...
    return value.toBool() ? " aktiv " : "- INAKTIV -";
}
