#ifndef TEST_LETTERTEMPLATE_H
#define TEST_LETTERTEMPLATE_H


#include <QObject>
#include <QSqlDatabase>
#include <QTest>

class test_letterTemplate : public QObject
{
    Q_OBJECT
public:
    explicit test_letterTemplate(QObject *p = nullptr);
    virtual ~test_letterTemplate(){}

private:

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void test_save_letter_template();
    void test_load_letter_template();
    void test_applyPlaceholders();
};

#endif // TEST_LETTERTEMPLATE_H
