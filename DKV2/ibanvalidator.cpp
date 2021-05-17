
#include "helper.h"
#include "ibanvalidator.h"


IbanValidator::IbanValidator(QObject* parent) : QRegExpValidator(parent)
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
    if (result not_eq QValidator::Acceptable)
        return result;
    // qDebug() << "2. string passed reg exp validation and is forwarded to checksum calculation" << iban;
    QString first4(iban.left(4));
    iban.remove(0, 4);
    iban.append(first4);
    QString ibanNumeric;
    for (int i(0); i < iban.length(); ++i){
        QChar character(iban.at(i));
        Q_ASSERT_X(character.isDigit() or character.isUpper(), "illegal character survived QRegExp validation", QString(character).toLatin1());
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
