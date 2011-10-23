/****************************************************************************
 *  history.cpp
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

#include "history.h"
#include "objectgenerator.h"
#include "buddy.h"
#include "metacontact.h"

namespace qutim_sdk_0_3
{
	struct Private
	{
		QWeakPointer<History> self;
	};
	static Private *p = NULL;

	void ensurePrivate_helper()
	{
		p = new Private;
		GeneratorList gens = ObjectGenerator::module<History>();
		if(!gens.isEmpty())
		   p->self = gens.first()->generate<History>();
	}
	inline void ensurePrivate()
	{ if(!p) ensurePrivate_helper(); }

	History::History()
	{
		ensurePrivate();
	}

	History::~History()
	{
	}

	History *History::instance()
	{
		ensurePrivate();
		if(p->self.isNull() && ObjectGenerator::isInited())
			p->self = new History();
		return p->self.data();
	}

	void History::store(const Message &message)
	{
		if(p->self.isNull() || p->self.data() == this)
			return;
		p->self.data()->store(message);
	}

	MessageList History::read(const ChatUnit *unit, const QDateTime &from, const QDateTime &to, int max_num)
	{
		if(p->self.isNull() || p->self.data() == this)
			return MessageList();
		return p->self.data()->read(unit, from, to, max_num);
	}

	void History::showHistory(const ChatUnit *unit)
	{
		if(p->self.isNull() || p->self.data() == this)
			return;
		p->self.data()->showHistory(unit);
	}

	void History::virtual_hook(int id, void *data)
	{
		Q_UNUSED(id);
		Q_UNUSED(data);
	}
}
