#ifndef TEST_BOOKING_H
#define TEST_BOOKING_H

class test_booking : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    // the actual tests
    void test_defaults();

    void test_bookDeposit();
    void test_bookPayout();
    void test_bookReInvest();
    void test_bookAnnualReInvest();
    void test_bookInterestActive();
    void test_getNbrOfBookings();
    void test_getBookings();
    void test_yearsWAnnualBookings();
    void test_changeBookingValue();
    void test_bookingTypeFunctions();
};

#endif // TEST_BOOKING_H
