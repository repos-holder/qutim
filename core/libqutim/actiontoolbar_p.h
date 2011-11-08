/****************************************************************************
 *  actiontoolbar_p.h
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *  Copyright (c) 2010-2011 by Sidorov Aleksey <sauron@citadelspb.com> 
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*****************************************************************************/

#ifndef ACTIONTOOLBAR_P_H
#define ACTIONTOOLBAR_P_H
#include <QObject>
#include <QPointer>
#include <QPoint>
#include <QVariant>
#include "localizedstring.h"
#include "menucontroller_p.h"
#include <QSize>

class QActionGroup;
class QMenu;
class QAction;

namespace qutim_sdk_0_3
{
class ActionToolBar;
class ActionGenerator;

typedef QList<ActionGenerator*> SizeList;

class ActionToolBarPrivate
{
	Q_DECLARE_PUBLIC(ActionToolBar)
public:
	ActionToolBarPrivate();
	virtual ~ActionToolBarPrivate();
	QMenu *initContextMenu();
	QList<ActionValue::Ptr> actions;
	QList<ObjectGenerator::Ptr> holders;
	QVariant data;
	QPoint dragPos;
	bool moveHookEnabled;
	ActionToolBar *q_ptr;
	QActionGroup *fillMenu(QMenu *menu, SizeList *map, int current = 0);
	QSize size;
	int style;
	void _q_size_action_triggered(QAction*);
	void _q_style_action_triggered(QAction*);
};
}

#endif // ACTIONTOOLBAR_P_H
