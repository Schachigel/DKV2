

#include "helper.h"
#include "helperfin.h"
#include "ibanvalidator.h"

IbanValidator* IbanValidator::globalValidator =nullptr;

namespace {

const QString expression {
qsl("(AL)\\d\\d[0-9]{8}[A-Z0-9]{16}|(AD)\\d\\d[0-9]{8}[A-Z0-9]{12}|(AT)\\d\\d[0-9]{16}|(BE)\\d\\d[0-9]{12}|(BH)\\\
d\\d[A-Z]{4}[A-Z0-9]{14}|(BA)\\d\\d[0-9]{16}|(BG)\\d\\d[A-Z]{4}[0-9]{6}[A-Z0-9]{8}|(HR)\\d\\d[0-9]{17}|(CY\
)\\d\\d[0-9]{8}[A-Z0-9]{16}|(CZ)\\d\\d[0-9]{20}|(DK)\\d\\d[0-9]{14}|(DO)\\d\\d[A-Z]{4}[0-9]{20}|(EE)\\d\\d[0-\
9]{16}|(FO)\\d\\d[0-9]{14}|(FI)\\d\\d[0-9]{14}|(FR|TF|PF|YT|NC|PM|WF)\\d\\d[0-9]{10}[A-Z0-9]{11}[0-9]{2}|\
(GE)\\d\\d[A-Z0-9]{2}[0-9]{16}|(DE)\\d\\d[0-9]{18}|(GI)\\d\\d[A-Z]{4}[A-Z0-9]{15}|(GR)\\d\\d[0-9]{7}[A-Z0-9\
]{16}|(GL)\\d\\d[0-9]{14}|(HU)\\d\\d[0-9]{24}|(IS)\\d\\d[0-9]{22}|(IE)\\d\\d[A-Z0-9]{4}[0-9]{14}|(IL)\\d\\d[0\
-9]{19}|(IT)\\d\\d[A-Z]{1}[0-9]{10}[A-Z0-9]{12}|(KZ)\\d\\d[0-9]{3}[A-Z0-9]{3}[0-9]{10}|(KW)\\d\\d[A-Z]{4}\
[0-9]{22}|(LV)\\d\\d[A-Z]{4}[A-Z0-9]{13}|(LB)\\d\\d[0-9]{4}[A-Z0-9]{20}|(LI)\\d\\d[0-9]{5}[A-Z0-9]{12}|(L\
T)\\d\\d[0-9]{16}|(LU)\\d\\d[0-9]{3}[A-Z0-9]{13}|(MK)\\d\\d[0-9]{3}[A-Z0-9]{10}[0-9]{2}|(MT)\\d\\d[A-Z]{4}[\
0-9]{5}[A-Z0-9]{18}|(MR)\\d\\d[0-9]{23}|(MU)\\d\\d[A-Z]{4}[0-9]{19}[A-Z]{3}|(MC)\\d\\d[0-9]{10}[A-Z0-9]{1\
1}[0-9]{2}|(ME)\\d\\d[0-9]{18}|(NL)\\d\\d[A-Z]{4}[0-9]{10}|(NO)\\d\\d[0-9]{11}|(PL)\\d\\d[0-9]{24}|(PT)\\d\\d\
[0-9]{21}|(RO)\\d\\d[A-Z]{4}[A-Z0-9]{16}|(SM)\\d\\d[A-Z]{1}[0-9]{10}[A-Z0-9]{12}|(SA)\\d\\d[0-9]{2}[A-Z0-\
9]{18}|(RS)\\d\\d[0-9]{18}|(SK)\\d\\d[0-9]{20}|(SI)\\d\\d[0-9]{15}|(ES)\\d\\d[0-9]{20}|(SE)\\d\\d[0-9]{20}|(C\
H)\\d\\d[0-9]{5}[A-Z0-9]{12}|(TN)\\d\\d[0-9]{20}|(TR)\\d\\d[0-9]{5}[A-Z0-9]{17}|(AE)\\d\\d[0-9]{3}[0-9]{16}\
|(GB)\\d\\d[A-Z]{4}[0-9]{14}")};

}

bool checkIban(QString iban)
{
    if( iban.isEmpty ())
        return false;
    iban.remove(QLatin1Char(' '));    // ignore spaces
    iban = iban.toUpper ();
    static QRegularExpression re(expression);

    if( not re.match(iban).hasMatch()) {
        return false;
    }
    QString start {iban.left(4)};
    iban.remove(0,4);
    iban.append(start);
    QString ibanNumeric;
    for (int i(0); i < iban.length(); ++i){
        QChar character(iban.at(i));
        Q_ASSERT_X(character.isDigit() or character.isUpper(), "illegal character survived QRegExp validation", QString(character).toLatin1());
        if (character.isDigit())
            ibanNumeric.append(character);
        else
            ibanNumeric.append(i2s(character.toLatin1() - QChar(QLatin1Char('A')).toLatin1() + 10));}
    if (IbanValidator::mod97(ibanNumeric) == 1)
        return true;
    else
        return false;
}

IbanValidator::IbanValidator(QObject* parent) : QRegularExpressionValidator(parent)
{
    static const QRegularExpression ibanRegEx(expression);
    setRegularExpression(ibanRegEx);

    Q_ASSERT_X(regularExpression().isValid(), "invalid regExp", regularExpression().pattern().toLatin1());
}

IbanValidator::State IbanValidator::validate(QString& input, int& pos) const
{
    // qInfo() << "IbanValidator::validate" << "validate IBAN string";
    QString iban(input);
    // qInfo() << "1. basic check by regular expression (parent class) of" << iban;
    iban.remove(QLatin1Char(' '));    // generously ignore spaces
    iban = iban.toUpper();            // generously accept non-capitalized letters
    State result = QRegularExpressionValidator::validate(iban, pos);
    if (result not_eq QValidator::Acceptable)
        return result;
    // qInfo() << "2. string passed reg exp validation and is forwarded to checksum calculation" << iban;
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
            ibanNumeric.append(i2s(character.toLatin1() - QChar(QLatin1Char('A')).toLatin1() + 10));}
    if (mod97(ibanNumeric) == 1)
        return QValidator::Acceptable;
    else
        return QValidator::Invalid;
}

void IbanValidator::fixup(QString& input) const
{
    // qInfo() << "IbanValidator::fixup" << "nicely format IBAN";
    // qInfo() << "1. capitalize all letters in" << input;
    input = input.toUpper();
    // qInfo() << "2. remove all spaces in" << input;
    input.remove(QLatin1Char(' '));
    // qInfo() << "3. place spaces every four symbols in" << input;
    if (input.length() > 4)
        for (qsizetype i(input.length() / 4l * 4l); i > 0l; i = i - 4l)
            input.insert(i, QLatin1String(" "));
    // qInfo() << "4. trim possibly added space at end of" << input;
    input = input.trimmed();
}

unsigned int IbanValidator::mod97(const QString& input)
{
    // qInfo() << "IbanValidator::mod97" << "calculate module 97 of" << input;
    int a[30] = {1, 10, 3, 30, 9, 90, 27, 76, 81, 34, 49, 5, 50, 15, 53, 45, 62, 38, 89, 17, 73, 51, 25, 56, 75, 71, 31, 19, 93, 57};
    int ad(0);
    qsizetype len = input.length();
    for (qsizetype i(0); i < len; ++i)
        ad += a[i] * (input.at(len - 1- i).toLatin1() - QChar(QLatin1Char('0')).toLatin1());
    return ad % 97;
}
