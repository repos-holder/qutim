/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2011 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This file is part of free software; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************
 ****************************************************************************/

#ifndef SimpleAboutCreator_H
#define SimpleAboutCreator_H

#include <qutim/startupmodule.h>
#include <QWidget>
#include <QPointer>

namespace Core
{
class SimpleAboutCreator : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("Service", "AboutDialog")
	Q_CLASSINFO("Uses", "ContactList")
public:
    SimpleAboutCreator();
    ~SimpleAboutCreator();
public slots:
	void showWidget();
private:
	QPointer<QWidget> m_widget;
};
}

#endif // SimpleAboutCreator_H
