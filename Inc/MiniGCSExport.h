#ifndef MINIGCSEXPORT_H
#define MINIGCSEXPORT_H

#include <QtCore/qglobal.h>

#if defined(MINIGCS_LIBRARY)
#  define MINIGCS_EXPORT Q_DECL_EXPORT
#else
#  define MINIGCS_EXPORT Q_DECL_IMPORT
#endif

#endif // MINIGCSEXPORT_H

