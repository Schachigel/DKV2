#include "helper.h"
#include "customtoolbutton.h"

CustomToolButton::CustomToolButton(QWidget *parent) :
    QToolButton(parent)
{   LOG_CALL;
    setPopupMode(QToolButton::MenuButtonPopup);
    QObject::connect(this, SIGNAL(triggered(QAction*)),
                     this, SLOT(setDefaultAction(QAction*)));
}
