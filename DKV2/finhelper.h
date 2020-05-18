#ifndef FINHELPER_H
#define FINHELPER_H
#include <QMap>
#include <QDate>
#include <QRegExpValidator>

double gerundet(const double d, const int stellen = 2);
double auf2Stellen(const double d);
double auf6Stellen(const double d);

int TageBisJahresende_a(const QDate& d);
int TageBisJahresende(const QDate& d);
int TageSeitJahresAnfang_a(const QDate& d);
int TageSeitJahresAnfang(const QDate& d);

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
