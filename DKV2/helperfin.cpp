
#include <QStringLiteral>
#define qsl(x) QStringLiteral(x)
#include <QFile>
#include <math.h>
#include <QDebug>

#include "helperfin.h"

int TageZwischen(const QDate& von, const QDate& bis)
{   // finanzmathematischer Abstand zwischen zwei Daten im selben Jahr
    if( bis == von) return 0;
    Q_ASSERT( von.year() == bis.year());
    Q_ASSERT(bis > von);
    if( bis.day() == 31)
        return TageZwischen(von, QDate(bis.year(), bis.month(), 30));
    if( von.day() == 31)
        return TageZwischen(QDate(von.year(), von.month(), 30), bis);
    if( von.month() == bis.month())
        return bis.day() -von.day();

    int tageErsterMonat = 30-von.day();
    int tageVolleMonate = 30* (bis.month() -von.month() -1);
    int tageLetzterMonat = bis.day();
    qDebug() << "TageZwischen (" << von << ") und (" << bis << "): "
             << tageErsterMonat << " + " << tageVolleMonate << " + " << tageLetzterMonat
             << " =" << tageErsterMonat+tageVolleMonate+tageLetzterMonat;
    return tageErsterMonat + tageVolleMonate + tageLetzterMonat;
}

int TageBisJahresende(const QDate& d)
{
    if( d.day()==d.daysInMonth()) // letzter Tag des Monats
    {
        if( d.month()==2)
            // Feb hat 30 Tage (da Vertrag über Feb hinaus geht)
            return 30* (12-d.month()) +30-d.day();
        else
            // der 31. wird behandelt wie der 30.
            // alle restlichen Monate des Jahres
            return 30* (12-d.month());
    }
    int month = 12 - d.month();
    int days { 30 -d.day()};
    return 30*month +days;
}

int TageSeitJahresAnfang(const QDate& d)
{
    return 30* (d.month()-1) + ((d.day() == 31) ? 30 : d.day());
}

double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool thesa)
{
    qDebug().noquote() << "\n\nZinsberechnung von " << von << " bis " << bis << QString((thesa)? " thesaurierend\n" : ("ausschüttend\n"));
    if( !(von.isValid() && bis.isValid()) || ( von > bis)) {
        qCritical() << "Zinseszins kann nicht berechnet werden - ungültige Parameter";
        return -1.;
    }
    if( wert <= 0.) return 0.;

    if( von.year() == bis.year()) {
        return r2(TageZwischen(von, bis)/360. *zins/100. *wert);
    }
    int TageImErstenJahr = TageBisJahresende(von); // first day no intrest
    double ZinsImErstenJahr = TageImErstenJahr/360. *zins/100. *wert;
    double gesamtZins (ZinsImErstenJahr);

    double zwischenWert = (thesa) ? (wert+ZinsImErstenJahr) : (wert);
    double ZinsVolleJahre = 0.;
    int jahre(0);
    for( jahre=0; jahre < bis.year()-von.year()-1; jahre++) {
        double JahresZins = zwischenWert *zins/100.;
        gesamtZins += JahresZins; ZinsVolleJahre += JahresZins;
        zwischenWert = (thesa) ? (zwischenWert+JahresZins) : zwischenWert;
    }
    int TageImLetztenJahr = TageSeitJahresAnfang(bis);
    double ZinsRestjahr = TageImLetztenJahr/360. *zins/100. *zwischenWert;
    gesamtZins += ZinsRestjahr;
    gesamtZins = r2(gesamtZins);
    qDebug().noquote()
        << "\nErstes Jahr : " << ZinsImErstenJahr << "(" << TageImErstenJahr << " Tage)"
        << "\nVolle Jahre : " << ZinsVolleJahre << "(" << jahre << " Jahre)"
        << "\nLetztes Jahr: " << ZinsRestjahr << "(" << TageImLetztenJahr << " Tage)"
        << "\nGesamtzins  : " << gesamtZins << Qt::endl;
    return r2(gesamtZins);
}

IbanValidator::IbanValidator() : QRegExpValidator()
{
    QFile f (qsl(":/res/IbanRegExp.txt"));
    f.open(QIODevice::ReadOnly);
    QString exp =f.readAll();
    setRegExp(QRegExp(exp));

    Q_ASSERT_X(regExp().isValid(), "invalid regExp", regExp().pattern().toLatin1());
}

IbanValidator::State IbanValidator::validate(QString& input, int& pos) const
{
    // qDebug() << "IbanValidator::validate" << "validate IBAN string";
    QString iban(input);
    // qDebug() << "1. basic check by regular expression (parent class) of" << iban;
    iban.remove(QLatin1Char(' '));    // generously ignore spaces
    iban = iban.toUpper();            // generously accept non-capitalized letters
    State result = QRegExpValidator::validate(iban, pos);
    if (result != QValidator::Acceptable)
        return result;
    // qDebug() << "2. string passed reg exp validation and is forwarded to checksum calculation" << iban;
    QString first4(iban.left(4));
    iban.remove(0, 4);
    iban.append(first4);
    QString ibanNumeric;
    for (int i(0); i < iban.length(); ++i){
        QChar character(iban.at(i));
        Q_ASSERT_X(character.isDigit() || character.isUpper(), "illegal character survived QRegExp validation", QString(character).toLatin1());
        if (character.isDigit())
            ibanNumeric.append(character);
        else
            ibanNumeric.append(QString::number(character.toLatin1() - QChar(QLatin1Char('A')).toLatin1() + 10));}
    if (mod97(ibanNumeric) == 1)
        return QValidator::Acceptable;
    else
        return QValidator::Invalid;
}

void IbanValidator::fixup(QString& input) const
{
    // qDebug() << "IbanValidator::fixup" << "nicely format IBAN";
    // qDebug() << "1. capitalize all letters in" << input;
    input = input.toUpper();
    // qDebug() << "2. remove all spaces in" << input;
    input.remove(QLatin1Char(' '));
    // qDebug() << "3. place spaces every four symbols in" << input;
    if (input.length() > 4) for (int i(input.length() / 4 * 4); i > 0; i = i - 4)
            input.insert(i, QLatin1String(" "));
    // qDebug() << "4. trim possibly added space at end of" << input;
    input = input.trimmed();
}

unsigned int IbanValidator::mod97(const QString& input) const
{
    // qDebug() << "IbanValidator::mod97" << "calculate module 97 of" << input;
    int a[30] = {1, 10, 3, 30, 9, 90, 27, 76, 81, 34, 49, 5, 50, 15, 53, 45, 62, 38, 89, 17, 73, 51, 25, 56, 75, 71, 31, 19, 93, 57};
    int ad(0);
    int len = input.length();
    for (int i(0); i < len; ++i)
        ad += a[i] * (input.at(len - 1- i).toLatin1() - QChar(QLatin1Char('0')).toLatin1());
    return ad % 97;
}
