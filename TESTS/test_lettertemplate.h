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
    const QString filename = "..\\..\\data\\testdb.sqlite";
    const QString testCon = "test_dkdb_connection";
    QSqlDatabase testDb() {return QSqlDatabase::database(testCon);}

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void test_save_letter_template();
    void test_load_letter_template();
    void test_applyPlaceholders();
};

#endif // TEST_LETTERTEMPLATE_H
