//#include "finhelper.h"
#include <QDate>

#include "finhelper.h"
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
    double percent = round2(value.toDouble());
    return QString::number(percent) + "%";
};

QString EuroItemFormatter::displayText(const QVariant& value, const QLocale& locale)const
{
    double w = round2(value.toDouble());
    if( w <= 0)
        return "[" + locale.toCurrencyString(-1 *w) + "] offen";
    else
        return locale.toCurrencyString(w);
};

QString KFristItemFormatter ::displayText(const QVariant &value, const QLocale &) const
{
    int v = value.toInt();
    if( v == -1)
        return "(festes Laufzeitende)";
    else
        return QString("%L1 Monate").arg(v);
}

QString thesaItemFormatter :: displayText(const QVariant &value, const QLocale &) const
{
    if( value.toBool())
        return "thesaurierend";
    else
        return "auszahlend";
}
