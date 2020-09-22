#ifndef FINHELPER_H
#define FINHELPER_H

#include <QDate>
#include <QRegExpValidator>

inline double r2(const double d)
{
    return (qRound(d * 100.))/100.;
}
inline double r4(const double d)
{
    return (qRound(d * 10000.))/10000.;
}
inline double r6(const double d)
{
    return (qRound(d * 1000000.))/1000000.;
}

inline int ctFromEuro( const double d)
{
    if( d>=0)
        return int( d * 100. +.5);
    return int( d * 100. -.5);
}
inline double euroFromCt( const int i)
{
    return double (i)/100.;
}

inline QString i2s(int x)       {return QString::number(x);}
inline QString d2s_2d(double x) {return QString::number(x, 'f', 2);}
inline QString d2s_4d(double x) {return QString::number(x, 'f', 4);}
inline QString d2s_6d(double x) {return QString::number(x, 'f', 6);}
QString d2euro(double x);
inline QString d2percent(double x) {return QString::number(x, 'f', 2) +QStringLiteral (" %");}


int TageBisJahresende(const QDate& d);
int TageBisJahresende_lookup(const QDate& d);
int TageSeitJahresAnfang(const QDate& d);
int TageSeitJahresAnfang_lookup(const QDate& d);

double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa=true);

// taken from https://github.com/Al-/IbanValidator
class IbanValidator : public QRegExpValidator
{
public:
    explicit IbanValidator();
    virtual void fixup (QString& input) const;
    virtual State validate (QString& input, int& pos) const;
private:
    unsigned int mod97(const QString& input) const;
};


#endif // FINHELPER_H
