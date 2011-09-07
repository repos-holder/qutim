/****************************************************************************
**
** qutIM instant messenger
**
** Copyright (C) 2011 Evgeniy Degtyarev <degtep@gmail.com>
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

#ifndef QUICKPASSWORDDIALOGMANAGER_H
#define QUICKPASSWORDDIALOGMANAGER_H

#include <QtCore/QObject>
namespace MeegoIntegration
{

class QuickPasswordDialogManager : public QObject {
	Q_OBJECT

public:
	static void init();

private:
	QuickPasswordDialogManager();


};
}

#endif /* PASSWORDDIALOGMANAGER_H */
