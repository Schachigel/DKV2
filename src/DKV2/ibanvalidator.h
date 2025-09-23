#ifndef IBANVALIDATOR_H
#define IBANVALIDATOR_H



bool checkIban(QString iban);

// taken from https://github.com/Al-/IbanValidator
class IbanValidator : public QRegularExpressionValidator
{
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
