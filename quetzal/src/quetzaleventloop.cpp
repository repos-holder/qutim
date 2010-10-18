/****************************************************************************
 *  quetzaleventloop.cpp
 *
 *  Copyright (c) 2009 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#include "quetzaleventloop.h"
#include <qutim/debug.h>
#include <QTimerEvent>
#include <QThread>
#include <QCoreApplication>
#include <QVariant>
#include <QTimer>

using namespace qutim_sdk_0_3;

QuetzalTimer *QuetzalTimer::m_self = NULL;

QuetzalTimer::QuetzalTimer(QObject *parent):
		QObject(parent), m_socketId(0)
{
}

QuetzalTimer *QuetzalTimer::instance()
{
	if (!m_self) {
		m_self = new QuetzalTimer();
	}
	return m_self;
}

uint QuetzalTimer::addTimer(guint interval, GSourceFunc function, gpointer data)
{
	Q_ASSERT(QThread::currentThread() == qApp->thread());
	int id = startTimer(interval);
	m_timers.insert(id, new TimerInfo(function, data));
	return static_cast<uint>(id);
}

gboolean QuetzalTimer::removeTimer(guint handle)
{
	Q_ASSERT(QThread::currentThread() == qApp->thread());
	int id = static_cast<int>(handle);
	QMap<int, TimerInfo *>::iterator it = m_timers.find(id);
	if (it == m_timers.end())
		return FALSE;
	killTimer(id);
	delete it.value();
	m_timers.erase(it);
	return TRUE;
}

void QuetzalTimer::timerEvent(QTimerEvent *event)
{
	QMap<int, TimerInfo *>::iterator it = m_timers.find(event->timerId());
	if (it == m_timers.end())
		return;
	TimerInfo *info = it.value();
	gboolean result = ( *info->function)(info->data);
	qDebug() << it.key() << event->timerId() << !!result;
	if (!result) {
		killTimer(it.key());
		delete it.value();
		m_timers.erase(it);
	}
}

guint QuetzalTimer::addIO(int fd, PurpleInputCondition cond, PurpleInputFunction func, gpointer user_data)
{
	Q_ASSERT(QThread::currentThread() == qApp->thread());
	if (fd < 0) {
		warning() << "Invalid file descriptor" << fd << "return id" << m_socketId;
		return m_socketId++;
	}

	QSocketNotifier::Type type;
	if (cond & PURPLE_INPUT_READ)
		type = QSocketNotifier::Read;
	else
		type = QSocketNotifier::Write;

	QSocketNotifier *socket = new QSocketNotifier(fd, type, this);
	socket->setProperty("quetzal_id", m_socketId);
	connect(socket, SIGNAL(activated(int)), this, SLOT(onSocket(int)));

	m_files.insert(m_socketId, new FileInfo(fd, socket, cond, func, user_data));
	socket->setEnabled(true);
	return m_socketId++;
}

gboolean QuetzalTimer::removeIO(guint handle)
{
	Q_ASSERT(QThread::currentThread() == qApp->thread());
	QMap<uint, FileInfo *>::iterator it = m_files.find(handle);
	if (it == m_files.end())
		return FALSE;
	FileInfo *info = it.value();
	QTimer::singleShot(0, info->socket, SLOT(deleteLater()));
	// Don't know exactly why yet, but it causes segfault at some cases
//	info->socket->deleteLater();
	m_files.erase(it);
	delete info;
	return TRUE;
}

int QuetzalTimer::getIOError(int fd, int *error)
{
	return 0;
}

void QuetzalTimer::onSocket(int fd)
{
	QSocketNotifier *socket = qobject_cast<QSocketNotifier *>(sender());
	guint id = socket->property("quetzal_id").toUInt();
	QMap<uint, FileInfo *>::iterator it = m_files.find(id);
	if (it != m_files.end()) {
		FileInfo *info = it.value();
		socket->setEnabled(false);
		(*info->func)(info->data, fd, info->cond);
		socket->setEnabled(true);
	}
}

static guint quetzal_timeout_add(guint interval, GSourceFunc function, gpointer data)
{
	return QuetzalTimer::instance()->addTimer(interval, function, data);
}

static gboolean quetzal_timeout_remove(guint handle)
{
	return QuetzalTimer::instance()->removeTimer(handle);
}

static guint quetzal_input_add(int fd, PurpleInputCondition cond, PurpleInputFunction func, gpointer user_data)
{
	return QuetzalTimer::instance()->addIO(fd, cond, func, user_data);
}

static gboolean quetzal_input_remove(guint handle)
{
	return QuetzalTimer::instance()->removeIO(handle);
}

//static int quetzal_input_get_error(int fd, int *error)
//{
//	return QuetzalTimer::instance()->getIOError(fd, error);
//}

static guint quetzal_timeout_add_seconds(guint interval, GSourceFunc function, gpointer data)
{
	return quetzal_timeout_add(interval * 1000, function, data);
}

PurpleEventLoopUiOps quetzal_eventloop_uiops =
{
	quetzal_timeout_add,
	quetzal_timeout_remove,
	quetzal_input_add,
	quetzal_input_remove,
	NULL /*quetzal_input_get_error*/,
	quetzal_timeout_add_seconds,

	/* padding */
	NULL,
	NULL,
	NULL
};
