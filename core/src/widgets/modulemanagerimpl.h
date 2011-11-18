/****************************************************************************
**
** qutIM instant messenger
**
** Copyright (C) 2011 Ruslan Nigmatullin <euroelessar@ya.ru>
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

#ifndef MODULEMANAGERIMPL_H
#define MODULEMANAGERIMPL_H

#include <qutim/modulemanager.h>
#include <QWizard>

using namespace qutim_sdk_0_3;

namespace Core
{
	class ModuleManagerImpl : public ModuleManager
	{
		Q_OBJECT
	public:
		ModuleManagerImpl();
		virtual ExtensionInfoList coreExtensions() const;
	private slots:
		virtual void initExtensions();
		void submitStatistics();
	private:
		QWizard *wizard;
	};
}

#endif // MODULEMANAGERIMPL_H
