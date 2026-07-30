#ifndef QMAVSDKTEXTCATALOG_H
#define QMAVSDKTEXTCATALOG_H

#include <QString>
#include <QStringView>

class QMavsdkTextCatalog
{
public:
    static QString text(QStringView section, int value);
};

#endif // QMAVSDKTEXTCATALOG_H
