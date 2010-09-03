/****************************************************************************
*  popupwidget.cpp
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

#include "popupwidget.h"
#include <QVBoxLayout>
#include "manager.h"
#include <QMouseEvent>
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QDesktopWidget>
#include <qutim/qtwin.h>
#include <qutim/emoticons.h>
#include <qutim/chatunit.h>
#include <qutim/messagesession.h>
#include <qutim/message.h>

namespace Core
{
namespace KineticPopups
{
PopupWidget::PopupWidget ()
{
	ThemeHelper::PopupSettings popupSettings = Manager::self()->popupSettings;
	init(popupSettings);
}

PopupWidget::PopupWidget (const ThemeHelper::PopupSettings &popupSettings)
{
	init(popupSettings);
}

void PopupWidget::init (const ThemeHelper::PopupSettings &popupSettings)
{
	m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), this, SIGNAL(activated()));
	//init browser
	setTheme(popupSettings);
	if (popupSettings.popupFlags & ThemeHelper::Preview) {
		setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	}
	else {
		setWindowFlags(popup_settings.widgetFlags);
		setAttribute(Qt::WA_DeleteOnClose);
		if (popupSettings.popupFlags & ThemeHelper::Transparent) {
			setAttribute(Qt::WA_NoSystemBackground);
			setAttribute(Qt::WA_TranslucentBackground);
			viewport()->setAutoFillBackground(false);
		}
		if (popupSettings.popupFlags & ThemeHelper::AeroThemeIntegration) {
			//init aero integration for win
			if (QtWin::isCompositionEnabled()) {
				QtWin::extendFrameIntoClientArea(this);
				setContentsMargins(0, 0, 0, 0);
			}
		}

	}
	setFrameShape ( QFrame::NoFrame );
	setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
	setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	setContextMenuPolicy(Qt::NoContextMenu);
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff);
}

void PopupWidget::setData ( const QString& title, const QString& body, QObject *sender,const QVariant &data)
{
	Manager *manager = Manager::self();

	m_sender = sender;
	m_data = data;

	QString popup_title = title;

	Contact *c = qobject_cast<Contact *>(sender);
	QString image_path = c ? c->avatar() : QString();
	if(image_path.isEmpty())
		image_path = QLatin1String(":/icons/qutim_64");
	QString popup_data = popup_settings.content;
	QString text;
	if (manager->parseEmoticons)
		text = Emoticons::theme().parseEmoticons(text);
	else
		text = body;
	if (text.length() > manager->maxTextLength) {
		text.truncate(manager->maxTextLength);
		text.append("...");
	}
	popup_data.replace ( "{title}", popup_title );
	popup_data.replace ( "{body}", text);
	popup_data.replace ( "{imagepath}",image_path);
	document()->setTextWidth(popup_settings.defaultSize.width());
	document()->setHtml(popup_data);
	emit sizeChanged(sizeHint());

	if (manager->timeout > 0) {
		if (m_timer.isActive())
			m_timer.stop();
		m_timer.start(manager->timeout);
	}
}


void PopupWidget::setTheme (const ThemeHelper::PopupSettings &popupSettings )
{
	popup_settings = popupSettings;
	this->setStyleSheet (popup_settings.styleSheet);
}


void PopupWidget::mouseReleaseEvent ( QMouseEvent* ev )
{
	//TODO
	if (ev->button() == Qt::LeftButton) {
		onAction1Triggered();
	}
	else if (ev->button() == Qt::RightButton)
		onAction2Triggered();
	else
		return;
	emit activated();
}

PopupWidget::~PopupWidget()
{
}


void KineticPopups::PopupWidget::onAction1Triggered()
{
	if (ChatUnit *unit = qobject_cast<ChatUnit *>(m_sender)) {
		ChatUnit *metaContact = unit->metaContact();
		if (metaContact)
			unit = metaContact;
		ChatLayer::get(unit,true)->activate();
	}
	else if (QWidget *widget = qobject_cast<QWidget *>(m_sender)) {
		widget->raise();
	}
}

void KineticPopups::PopupWidget::onAction2Triggered()
{
	ChatUnit *unit = qobject_cast<ChatUnit *>(m_sender);
	ChatSession *sess;
	if (unit && (sess = ChatLayer::get(unit,false))) {

		if (m_data.canConvert<Message>())
			sess->markRead(m_data.value<Message>().id());
	}
}

void PopupWidget::timerEvent(QTimerEvent* ev)
{
	emit activated();
	QTextEdit::timerEvent(ev);
}

QSize PopupWidget::sizeHint() const
{
	int width = popup_settings.defaultSize.width();
	int height = static_cast<int>(document()->size().height());
	return QSize(width,height);
}

}
}




