#include "helper.h"
#include "uicustomtoolbutton.h"

uiCustomToolButton::uiCustomToolButton(QWidget *parent) :
    QToolButton(parent)
{   LOG_CALL;
    setPopupMode(QToolButton::MenuButtonPopup);
    QObject::connect(this, SIGNAL(triggered(QAction*)),
                     this, SLOT(setDefaultAction(QAction*)));
}
