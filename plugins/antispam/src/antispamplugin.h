/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Ruslan Nigmatullin <euroelessar@yandex.ru>
** Copyright © 2012 Ruslan Nigmatullin <euroelessar@yandex.ru>
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

#ifndef ANTISPAMPLUGIN_H
#define ANTISPAMPLUGIN_H

#include <qutim/plugin.h>
#include <QPointer>

namespace Antispam {

using namespace qutim_sdk_0_3;

class Handler;
class AntispamPlugin : public Plugin
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qutim.Plugin")
	Q_CLASSINFO("DebugName", "Antispam")
	Q_CLASSINFO("Uses", "IconLoader")
public:
	virtual void init();
	virtual bool load();
	virtual bool unload();
private:
	QPointer<Handler> m_handler;
};

}

#endif // ANTISPAMPLUGIN_H

