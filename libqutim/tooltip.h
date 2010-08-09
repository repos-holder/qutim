/****************************************************************************
 *  tooltip.h
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include "libqutim_global.h"
#include "localizedstring.h"
#include "extensionicon.h"
#include <QPoint>
#include <QEvent>
#include <QVariant>

namespace qutim_sdk_0_3
{
	class Buddy;
	class ToolTipEventPrivate;

	class LIBQUTIM_EXPORT ToolTipEvent : public QEvent
	{
	public:
		ToolTipEvent(bool generateLayout = true);
		~ToolTipEvent();
		void addHtml(const QString &html, quint8 priority = 60);
		void addField(const LocalizedString &title,
					  const LocalizedString &data = LocalizedString(),
					  quint8 priority = 60);
		void addField(const LocalizedString &title,
					  const LocalizedString &data,
					  const QString &icon,
					  quint8 priority = 60);
		void addField(const LocalizedString &title,
					  const LocalizedString &data,
					  const ExtensionIcon &icon,
					  quint8 priority = 60);
		bool generateLayout() const;
		QString html() const;
		static QEvent::Type eventType();
	protected:
		friend class ToolTip;
		QScopedPointer<ToolTipEventPrivate> d;
	};

	class LIBQUTIM_EXPORT ToolTip : public QObject
	{
		Q_OBJECT
	public:
		static ToolTip *instance();
		virtual void showText(const QPoint &pos, QObject *obj, QWidget *w = 0);
		inline void hideText() { showText(QPoint(), 0); }
	protected:
		ToolTip(QObject *parent = 0);
		bool eventFilter(QObject *, QEvent *);
	};
}

#endif // TOOLTIPMANAGER_H
