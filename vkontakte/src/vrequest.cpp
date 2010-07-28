/****************************************************************************
 *  vrequest.cpp
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

#include "vrequest.h"

VRequest::VRequest(const QUrl& url): QNetworkRequest(url)
{
    setRawHeader("User-Agent", "qutIM plugin VKontakte");
    setRawHeader("Accept-Charset", "utf-8");
    setRawHeader("Pragma", "no-cache");
    setRawHeader("Cache-control", "no-cache");
}
