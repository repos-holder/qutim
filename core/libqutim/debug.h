/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright (C) 2011 Ruslan Nigmatullin <euroelessar@yandex.ru>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

#ifndef DEBUG_H
#define DEBUG_H

// Include QDebug, because we want to redefine some symbols
#include <QDebug>
#include "libqutim_global.h"

//#if !(defined(LIBQUTIM_LIBRARY) || defined(QUTIM_CORE))
//extern qptrdiff qutim_plugin_id();
//#endif

namespace qutim_sdk_0_3
{
	enum DebugLevel
	{
		DebugInfo = 0,
		DebugVerbose,
		DebugVeryVerbose
#if 1
		,
		Info = DebugInfo,
		Verbose = DebugVerbose,
		VeryVerbose = DebugVeryVerbose
#endif
	};

	LIBQUTIM_EXPORT QDebug debug_helper(quint64, DebugLevel, QtMsgType);
//	LIBQUTIM_EXPORT qptrdiff debug_area_helper(const char *str);
	LIBQUTIM_EXPORT void debugClearConfig();
    
//#ifndef QUTIM_DEBUG_AREA
//#if defined (LIBQUTIM_LIBRARY)
//# define QUTIM_DEBUG_AREA "libqutim"
//#elif defined (QUTIM_CORE)
//# define QUTIM_DEBUG_AREA "core"
//#else
//# define QUTIM_DEBUG_AREA ""
//#endif
//	
//	inline qptrdiff debug_area(const char *str)
//	{
//		static qptrdiff area = debug_area_helper(str);
//		return area;
//	}

#if defined(LIBQUTIM_LIBRARY) || defined(QUTIM_CORE) || 1
	inline QDebug debug(DebugLevel level = Info)
	{ return debug_helper(0, level, QtDebugMsg); }
	inline QDebug warning(DebugLevel level = Info)
	{ return debug_helper(0, level, QtWarningMsg); }
	inline QDebug critical(DebugLevel level = Info)
	{ return debug_helper(0, level, QtCriticalMsg); }
	inline QDebug fatal(DebugLevel level = Info)
	{ return debug_helper(0, level, QtFatalMsg); }
#else
	inline quint64 qutim_plugin_id()
	{ return Q_UINT64_C(QUTIM_PLUGIN_ID); }
	inline QDebug debug(DebugLevel level = Info)
	{ return debug_helper(qutim_plugin_id(), level, QtDebugMsg); }
	inline QDebug warning(DebugLevel level = Info)
	{ return debug_helper(qutim_plugin_id(), level, QtWarningMsg); }
	inline QDebug critical(DebugLevel level = Info)
	{ return debug_helper(qutim_plugin_id(), level, QtCriticalMsg); }
	inline QDebug fatal(DebugLevel level = Info)
	{ return debug_helper(qutim_plugin_id(), level, QtFatalMsg); }
#endif
}
#endif // DEBUG_H

