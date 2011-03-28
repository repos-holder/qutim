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

#ifndef ABOUTKDEDIALOG_H
#define ABOUTKDEDIALOG_H

#include <qutim/startupmodule.h>

class AboutKdeDialog : public QObject, public qutim_sdk_0_3::StartupModule
{
    Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::StartupModule)
public:
    AboutKdeDialog();
};

#endif // ABOUTKDEDIALOG_H
