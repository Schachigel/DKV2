#ifndef IBANVALIDATOR_H
#define IBANVALIDATOR_H

#include "pch.h"

bool checkIban(QString iban);

// taken from https://github.com/Al-/IbanValidator
class IbanValidator : public QRegExpValidator
{
    Q_OBJECT
public:
    explicit IbanValidator(QObject* parent =nullptr);
    virtual void fixup (QString& input) const override;
    virtual State validate (QString& input, int& pos) const override;
    static IbanValidator* getValidator() {
        if( !globalValidator)
            globalValidator =new IbanValidator();
        return globalValidator;
    }
    static unsigned int mod97(const QString& input);
private:
    static IbanValidator* globalValidator;
};

#endif // IBANVALIDATOR_H
