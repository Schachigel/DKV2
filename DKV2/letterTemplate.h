#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>


class letterTemplate
{
public:
    enum sections{ dateFormat, projectAddress, projectUrl, Address, about, salutation,
                   mainText1, tableHeaderKennung, tableHeaderOldValue, tableHeaderInterest,
                   tableHeaderNewValue, mainText2, greeting, signee, maxSection
    };
    enum distances{ topmost= maxSection+1000, overProjectAddress, projectAddressHeight, logoWidth, overAbout,
                    overSalutation, overText, tableMargin, overGreeting, overSignee, maxDistance
    };
    enum templateId{ geldeingang, JA_thesa, JA_auszahlend, Kontoabschluss, maxTemplateId
    };

public:
    explicit letterTemplate(){};
    letterTemplate(templateId type);
    bool saveTemplate(const QString& con) const;
    bool loadTemplate(letterTemplate::templateId id, const QString& con);

    void init_geldeingang();
    void init_JA_thesa();
    void init_JA_auszahlend();
    void init_Kontoabschluss();

    static QString getNameFromId(templateId id) ;
    static templateId getIdFromName(QString n);
    bool operator ==(const letterTemplate &b) const;
private:
    templateId tid;
    QMap<int, QString> html;
    QMap<int, double>  length;
};

#endif // LETTER_H
