/****************************************************************************
 *  emoticonssettings.cpp
 *
 *  Copyright (c) 2010 by Aleksey Sidorov <sauron@citadelspb.com>
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

#include "emoticonssettings.h"
#include <libqutim/libqutim_global.h>
#include "modulemanagerimpl.h"
#include "emoticonsselector.h"
#include <libqutim/settingslayer.h>
#include <libqutim/icon.h>

namespace Core
{
	static CoreModuleHelper<EmoticonsSettings, StartupModule> emoticons_settings_static(
			QT_TRANSLATE_NOOP("Plugin", "Emoticons settings"),
			QT_TRANSLATE_NOOP("Plugin", "Default settings widget set for emoticons")
			);

	EmoticonsSettings::EmoticonsSettings()
	{
		GeneralSettingsItem<EmoticonsSelector> *item = new GeneralSettingsItem<EmoticonsSelector>(Settings::Appearance, Icon("emoticon"), QT_TRANSLATE_NOOP("Settings","Emoticons"));
		Settings::registerItem(item);
		deleteLater();
	}

}
