/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Ruslan Nigmatullin <euroelessar@yandex.ru>
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

#include "debug.h"
#include "config.h"
#include "objectgenerator.h"
#include <QTime>

namespace qutim_sdk_0_3
{
	struct DebugAreaData
	{
		inline DebugAreaData() : level(Info) {}
		QByteArray name;
		int level;
	};
	
	class NoDebugStream: public QIODevice
	{
	public:
		NoDebugStream() { open(WriteOnly); }
		bool isSequential() const { return true; }
		qint64 readData(char *, qint64) { return 0; /* eof */ }
		qint64 readLineData(char *, qint64) { return 0; /* eof */ }
		qint64 writeData(const char *, qint64 len) { return len; }
	};
	Q_GLOBAL_STATIC(NoDebugStream, devnull)
	
	typedef QMap<const QMetaObject*, DebugAreaData*> DebugMap;
	Q_GLOBAL_STATIC(DebugMap, debugAreaMap)
//	Q_GLOBAL_STATIC(QSet<qptrdiff>, debugAreaSet)
	
	void init_core_data(DebugAreaData *data)
	{
		data->name = "[Core]:";
		Config cfg;
		cfg.beginGroup("debug/core");
		data->level = cfg.value("level", DebugInfo);
//		debugAreaMap()->insert("core", data);
//		debugAreaSet()->insert(reinterpret_cast<qptrdiff>(data));
	}
	
//	void init_lib_data(DebugAreaData *data)
//	{
//		data->name = "[Library]:";
//		debugAreaMap()->insert("libqutim", data);
//		debugAreaSet()->insert(reinterpret_cast<qptrdiff>(data));
//	}

//	Q_GLOBAL_STATIC_WITH_INITIALIZER(DebugAreaData, coreData, init_core_data(x.data()))
//	Q_GLOBAL_STATIC_WITH_INITIALIZER(DebugAreaData, libData, init_lib_data(x))

	struct DebugData
	{
		DebugData() : inited(false), level(DebugInfo) {}
		bool inited;
		DebugLevel level;
	};
	
	Q_GLOBAL_STATIC(DebugData, debugData)
	
	QDebug debug_helper(quint64 ptr, DebugLevel level, QtMsgType type)
	{
		Q_UNUSED(ptr);
		DebugData *d = debugData();
		if (!d->inited && ObjectGenerator::isInited()) {
			d->inited = true;
			Config cfg;
			cfg.beginGroup(QLatin1String("debug"));
			d->level = cfg.value(QLatin1String("level"), DebugInfo);
		}
		if (d->level > level)
			return QDebug(devnull());
		
		return QDebug(type)
				<< qPrintable(QTime::currentTime().toString(QLatin1String("[hh:mm:ss]")));
//		const QMetaObject *meta = reinterpret_cast<const QMetaObject*>(ptr);
//		const DebugAreaData *data = meta ? debugAreaMap()->value(meta, 0) : coreData();
//		if (!data) {
//			DebugAreaData *d = new DebugAreaData();
//			Config cfg;
//			cfg.beginGroup("debug");
//			QString nameStr = QLatin1String(meta->className());
//			cfg.beginGroup(nameStr);
//			d->name = "[" + cfg.value("name", nameStr).toLocal8Bit() + "]:";
//			d->level = cfg.value("level", DebugInfo);
//			data = d;
//		}
		
//		if (data->level <= level)
//			return (QDebug(type) << data->name);
//		else
//			return QDebug(devnull());
	}
	
//	qptrdiff debug_area_helper(const char *str)
//	{
//		QByteArray name = str;
//		DebugAreaData *data = debugAreaMap()->value(name, 0);
//		if (data)
//			return reinterpret_cast<qptrdiff>(data);
//		data = new DebugAreaData();
//		Config cfg;
//		cfg.beginGroup("debug");
//		QString nameStr = QString::fromLatin1(name, name.size());
//		cfg.beginGroup(nameStr);
//		data->name = "[" + cfg.value("name", nameStr).toLocal8Bit() + "]:";
//		data->level = cfg.value("level", DebugInfo);
//		debugAreaSet()->insert(data);
//		return reinterpret_cast<qptrdiff>(data);
//	}
	
	void debugClearConfig()
	{
		DebugMap::iterator it = debugAreaMap()->begin();
		DebugMap::iterator end = debugAreaMap()->end();
		Config cfg;
		cfg.beginGroup("debug");
		for (; it != end; it++) {
			DebugAreaData *data = it.value();
			QString nameStr = QLatin1String(it.key()->className());
			cfg.beginGroup(nameStr);
			data->name = "[" + cfg.value("name", nameStr).toLocal8Bit() + "]:";
			data->level = cfg.value("level", DebugInfo);
			cfg.endGroup();
		}
	}
}

