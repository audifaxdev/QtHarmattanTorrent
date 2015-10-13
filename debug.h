#ifndef DEBUG_H
#define DEBUG_H
#include <QDebug>
#define dbg(x) qDebug() << __PRETTY_FUNCTION__ << x
#endif // DEBUG_H
