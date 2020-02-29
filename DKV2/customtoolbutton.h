#ifndef CUSTOMTOOLBUTTON_H
#define CUSTOMTOOLBUTTON_H

#include <QToolButton>
#include <QDebug>

class CustomToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit CustomToolButton(QWidget *parent = 0);
};

#endif // CUSTOMTOOLBUTTON_H
