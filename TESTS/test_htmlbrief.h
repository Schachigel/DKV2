#ifndef TEST_HTMLBRIEF_H
#define TEST_HTMLBRIEF_H

#include <QObject>
#include <QTest>

class test_htmlbrief : public QObject
{
    Q_OBJECT
public:
    explicit test_htmlbrief(QObject *p =nullptr);
    virtual ~test_htmlbrief(){}

private slots:
    void init(){}
    void cleanup(){}
    void test_parseEmptyString();
    void test_parseString_data();
    void test_parseString();

signals:
public slots:
};

#endif // TEST_HTMLBRIEF_H
