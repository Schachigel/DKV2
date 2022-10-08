#ifndef BUSYCURSOR_H
#define BUSYCURSOR_H

// #include "pch.h"

struct busycursor
{
    busycursor() {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    };
    busycursor(const busycursor&) =delete;
    ~busycursor() {
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
