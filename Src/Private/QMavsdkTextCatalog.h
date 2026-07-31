#ifndef QMAVSDKTEXTCATALOG_H
#define QMAVSDKTEXTCATALOG_H

#include <QString>
#include <QStringView>

class QMavsdkTextCatalog
{
public:
    static QString text(QStringView section, int value);
    static QString text(QStringView section, QStringView key);
};

#endif // QMAVSDKTEXTCATALOG_H
