#ifndef TEST_LETTERSNIPPET_H
#define TEST_LETTERSNIPPET_H


#include <QObject>
#include <QSqlDatabase>
#include <QTest>

class test_letterSnippet : public QObject
{
    Q_OBJECT
public:
    explicit test_letterSnippet(QObject *p = nullptr);
    virtual ~test_letterSnippet(){}

private:

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void test_write_read_snippet();
    void test_overwrite_snippet();
    void test_many_snippet_read_writes();
    void test_snippet_type_dep_read();
    void test_fallback();
};

#endif // TEST_LETTERSNIPPET_H
