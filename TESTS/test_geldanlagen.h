#ifndef TEST_GELDANLAGEN_H
#define TEST_GELDANLAGEN_H

#include "qobject.h"
#include "qobjectdefs.h"
#include <QTest>
#include <QObject>

class test_geldanlagen : public QObject
{
    Q_OBJECT
public:
    explicit test_geldanlagen();
private slots:
    void init();
    void cleanup();
    void  test_ohneAnlagen();
    void test_kontinuierlicheAnlagen_data();
    void test_kontinuierlicheAnlagen();
    // void test_zeitlBegrenzteAnlagen();
};

#endif // TEST_GELDANLAGEN_H
