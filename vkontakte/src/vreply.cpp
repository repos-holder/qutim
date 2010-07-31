/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#include "vreply.h"
#include <qutim/json.h>

VReply::VReply(QNetworkReply *parent) :
    QObject(parent)
{
	connect(parent, SIGNAL(finished()), this, SLOT(onRequestFinished()));
}

void VReply::onRequestFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if (reply->error() != QNetworkReply::NoError) {
		QVariantMap error;
		error.insert("error_code", 1);
		error.insert("error_msg", reply->errorString());
		emit resultReady(error, true);
		return;
	}
	QVariantMap map = qutim_sdk_0_3::Json::parse(reply->readAll()).toMap();
	if (map.contains("error")) {
		QVariantMap error = map.value("error").toMap();
		if (error.value("error_code").toInt() == CaptchaNeeded) {
			QString sid = error.value("captcha_sid").toString();
			QString url = error.value("captcha_img").toString();
			QNetworkAccessManager *manager = reply->manager();
			qDebug("%s %s %s", Q_FUNC_INFO, qPrintable(sid), qPrintable(url));
#if 0
			QVariantMap params;
			QVariantList list = error.value("request_params").toList();
			for (int i = 0; i < list.size(); i++) {
				QVariantMap map = list.at(i).toMap();
				params.insert(map.value(QLatin1String("key")), map.value(QLatin1String("value")));
			}
			VCaptchaRequest *request = new VCaptchaRequest(params, sid, url, manager);
			setParent(request);
#else
			Q_UNUSED(manager);
#endif
		} else {
			emit resultReady(error, true);
		}
	} else {
		emit resultReady(map.value("response"), false);
	}
}
