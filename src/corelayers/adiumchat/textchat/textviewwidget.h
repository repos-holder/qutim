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

#ifndef TEXTVIEWWIDGET_H
#define TEXTVIEWWIDGET_H

#include <chatlayer/chatviewfactory.h>
#include <QFrame>
#include <QTextEdit>
#include <QTextDocument>

namespace Core
{
namespace AdiumChat
{
class TextViewWidget : public QTextEdit, public Core::AdiumChat::ChatViewWidget
{
	Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatViewWidget)
public:
    TextViewWidget();
	virtual void setViewController(QObject *controller);
private:
//	QTextEdit *m_textEdit;
};
}
}

#endif // TEXTVIEWWIDGET_H
