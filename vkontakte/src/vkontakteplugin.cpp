/****************************************************************************
 *  vkontakteplugin.cpp
 *
 *  Copyright (c) 2010 by Sidorov Aleksey <sauron@citadelspb.com>
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

#include "vkontakteplugin.h"
#include <qutim/debug.h>
#include "vkontakteprotocol.h"
#include "ui/vaccountcreator.h"
#include <qutim/settingslayer.h>
#include <qutim/menucontroller.h>
#include <qutim/icon.h>
#include "vaccount.h"
#include "vproxymanager.h"

void VkontaktePlugin::init()
{
	debug() << Q_FUNC_INFO;
	setInfo(QT_TRANSLATE_NOOP("Plugin", "Vkontakte"),
			QT_TRANSLATE_NOOP("Plugin", "Simple implementation of vkontakte, based on userapi.ru"),
			PLUGIN_VERSION(0, 0, 1, 0));
	addAuthor(QT_TRANSLATE_NOOP("Author", "Sidorov Aleksey"),
			  QT_TRANSLATE_NOOP("Task", "Author"),
			  QLatin1String("sauron@citadelspb.com"));
// 	addAuthor(QT_TRANSLATE_NOOP("Author", "Rustam Chakin"),
// 			  QT_TRANSLATE_NOOP("Task", "Founder"),
// 			  QLatin1String(""));

	ExtensionIcon vicon = ExtensionIcon("im-vkontakte");

	addExtension(QT_TRANSLATE_NOOP("Plugin", "vkontakte"),
				 QT_TRANSLATE_NOOP("Plugin", "Simple implementation of vkontakte, based on desktop.api"),
				 new GeneralGenerator<VkontakteProtocol>(),
				 vicon
				 );
	addExtension(QT_TRANSLATE_NOOP("Plugin", "Vkontakte account creator"),
				 QT_TRANSLATE_NOOP("Plugin", "Account creator for Vkontakte"),
				 new GeneralGenerator<VAccountCreator>(),
				 vicon);
	addExtension(QT_TRANSLATE_NOOP("Plugin", "Proxy support"),
				 QT_TRANSLATE_NOOP("Plugin", "Proxy support for Vkontakte"),
				 new GeneralGenerator<VProxyManager, NetworkProxyManager>(),
				 vicon);
}
bool VkontaktePlugin::load()
{
	return true;
}
bool VkontaktePlugin::unload()
{
	return false;
}

QUTIM_EXPORT_PLUGIN(VkontaktePlugin)
