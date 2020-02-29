#include "customtoolbutton.h"

CustomToolButton::CustomToolButton(QWidget *parent) :
    QToolButton(parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
    QObject::connect(this, SIGNAL(triggered(QAction*)),
                     this, SLOT(setDefaultAction(QAction*)));
}
