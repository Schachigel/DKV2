#ifndef BUSYCURSOR_H
#define BUSYCURSOR_H



struct busyCursor
{
    busyCursor() {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    };
    busyCursor(const busyCursor&) =delete;
    ~busyCursor() {
        QGuiApplication::restoreOverrideCursor();
    }
    void finish() {
        QGuiApplication::restoreOverrideCursor();
    }
    void set() {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
};

#endif // BUSYCURSOR_H
