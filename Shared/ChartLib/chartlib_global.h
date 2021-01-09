#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(CHARTLIB_LIB)
#  define CHARTLIB_EXPORT Q_DECL_EXPORT
# else
#  define CHARTLIB_EXPORT Q_DECL_IMPORT
# endif
#else
# define CHARTLIB_EXPORT
#endif
