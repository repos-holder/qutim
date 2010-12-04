/****************************************************************************
 *  joinpage.h
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

#ifndef JOINPAGE_H
#define JOINPAGE_H

#include <QScrollArea>
#include <QPointer>
#include "groupchatpage.h"

class QVBoxLayout;
class QCheckBox;
class QLineEdit;
namespace Core {

class JoinPage : public GroupChatPage
{
    Q_OBJECT
public:
	explicit JoinPage(QWidget *parent = 0);
public slots:
	void join();
	void updateDataForm();
protected:
	void showEvent(QShowEvent *);
signals:
	void joined();
private slots:
	void onCheckStateChanged(int state);
private:
	QLineEdit *m_bookmarksEdit;
	QPointer<QWidget> m_dataForm;
};

} // namespace Core

#endif // JOINPAGE_H
