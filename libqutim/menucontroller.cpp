/****************************************************************************
 *  menucontroller.cpp
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

#include "menucontroller_p.h"
#include "actiongenerator_p.h"
#include <QMap>
#include <QDebug>
#include <QMetaMethod>
#include "debug.h"

namespace qutim_sdk_0_3
{

	typedef QMap<const QMetaObject *, ActionInfo> MenuActionMap;

	Q_GLOBAL_STATIC(MenuActionMap, globalActions)

	MenuController::MenuController(QObject *parent) : QObject(parent), d_ptr(new MenuControllerPrivate(this))
	{
	}

	MenuController::MenuController(MenuControllerPrivate &mup, QObject *parent) : QObject(parent), d_ptr(&mup)
	{
	}

	MenuController::~MenuController()
	{
	}

	bool actionLessThan(const ActionInfo &a, const ActionInfo &b)
	{
		int cmp = a.menu.size() - b.menu.size();
		if (cmp == 0) {
			for (int i = 0; i < a.menu.size() && !cmp; i++)
				cmp = a.hash.at(i) - b.hash.at(i); //qstrcmp(a.menu.at(i), b.menu.at(i));
			if (cmp != 0)
				return cmp < 0;
		} else {
			return cmp < 0;
		}

		// Items are inside one menu
		if (a.gen->type() == b.gen->type()) {
			if (a.gen->priority() == b.gen->priority())
				return a.gen->text() < b.gen->text();
			else
				return a.gen->priority() > b.gen->priority();
		} else {
			return a.gen->type() < b.gen->type();
		}
	}

	inline bool isEqualMenu(const QList<uint> &a, const QList<uint> &b)
	{
		if (a.size() != b.size())
			return false;
		for (int i = 0; i < a.size(); i++) {
			if (a[i] != b[i])
				return false;
		}
		return true;
	}

	inline quint64 pack_helper(quint32 a, quint32 b)
	{
		return (quint64(a) << 32) | quint64(b);
	}
	
	inline void unpack_helper(quint64 h, quint32 *a, quint32 *b)
	{
		*b = h & Q_INT64_C(0xffffffff);
		*a = (h - *b) >> 32;
	}

	ActionEntry *DynamicMenu::findEntry(ActionEntry &entries, const ActionInfo &info, bool legacy)
	{
		ActionEntry *current = &entries;
		for (int i = 0; i < info.menu.size(); i++) {
			QMap<uint, ActionEntry>::iterator it = current->entries.find(info.hash.at(i));
			if (it == current->entries.end()) {
				QMenu *menu = current->menu->addMenu(QString::fromUtf8(info.menu.at(i), info.menu.at(i).size()));
				QAction *action = menu->menuAction();
				action->setData(pack_helper(info.hash.at(i), info.hash.size()));
				if (legacy)
					m_temporary << action;
				connect(this, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));
				it = current->entries.insert(info.hash.at(i), ActionEntry(menu));
			}
			current = &it.value();
		}
		return current;
	}

	// Move method somewhere outside the DynamicMenu
	QList<ActionInfo> DynamicMenu::allActions(bool legacy) const
	{
		const MenuController *owner = m_d->q_func();
		QList<ActionInfo> actions;
		if (legacy) {
			foreach (const MenuController::Action &act, m_d->dynamicActions()) {
				actions << ActionInfo(act.first, ActionGeneratorPrivate::get(act.first), act.second);
				m_d->temporary << act.first;
				m_owners.insert(act.first, const_cast<MenuController*>(owner));
			}
		}
		QSet<const QMetaObject *> metaObjects;
		while (owner) {
			foreach (const ActionInfo &info, m_d->actions) {
				if ((info.gen_p->connectionType == ActionConnectionLegacy) == legacy)
					actions << info;
			}
			const QMetaObject *meta = owner->metaObject();
			while (meta) {
				if (metaObjects.contains(meta))
					break;
				foreach (const ActionInfo &info, globalActions()->values(meta)) {
					if ((info.gen_p->connectionType == ActionConnectionLegacy) == legacy) {
						actions << info;
						m_owners.insert(info.gen, const_cast<MenuController*>(owner));
					}
				}
				metaObjects.insert(meta);
				meta = meta->superClass();
			}
			owner = MenuControllerPrivate::get(owner)->owner;
		}
		return actions;
	}

	DynamicMenu::DynamicMenu(const MenuControllerPrivate *d) :
			m_d(d), m_showed(false), m_group(new QActionGroup(this)), m_entry(this)
	{
		connect(this, SIGNAL(aboutToShow()), this, SLOT(onAboutToShow()));
		connect(this, SIGNAL(aboutToHide()), this, SLOT(onAboutToHide()));
		connect(m_group, SIGNAL(triggered(QAction*)), this, SLOT(onActionTriggered(QAction*)));
		QList<ActionInfo> actions = allActions(false);
		if (actions.isEmpty()) {
			return;
		}
		// Stable actions, they use new api and are always at memory
		qSort(actions.begin(), actions.end(), actionLessThan);
		int lastType = actions[0].gen->type();
		QList<uint> lastMenu;
		ActionEntry *currentEntry = &m_entry;
		for (int i = 0; i < actions.size(); i++) {
			const ActionInfo &act = actions[i];
			if (!isEqualMenu(lastMenu, act.hash)) {
				lastType = act.gen->type();
				lastMenu = act.hash;
				currentEntry = findEntry(m_entry, act, false);
			} else if (lastType != act.gen->type()) {
				lastType = act.gen->type();
				currentEntry->menu->addSeparator();
			}
			const_cast<ActionGenerator *>(act.gen)->setMenuController(const_cast<MenuController *>(m_d->q_ptr));
			// TODO: create static global QMap<ActionGenerator*, QAction*>, 
			// use it as cashe and doesn't assign parent to QAction's created there
			if (QAction *action = act.gen->generate<QAction>()) {
				action->setParent(currentEntry->menu);
				currentEntry->menu->addAction(action);
				m_group->addAction(action);
			}
		}
	}
	
	DynamicMenu::~DynamicMenu()
	{
		foreach (QAction *action, actions())
			action->deleteLater();
	}

	void DynamicMenu::onAboutToShow()
	{
		QList<ActionInfo> actions = allActions(true);
		if (actions.isEmpty() || m_showed)
			return;
		qSort(actions.begin(), actions.end(), actionLessThan);
		int lastType = actions[0].gen->type();
		QList<uint> lastMenu;
		ActionEntry *currentEntry = &m_entry;
		for (int i = 0; i < actions.size(); i++) {
			const ActionInfo &act = actions[i];
			if (!isEqualMenu(lastMenu, act.hash)) {
				lastType = act.gen->type();
				lastMenu = act.hash;
				currentEntry = findEntry(m_entry, act, true);
			} else if (lastType != act.gen->type()) {
				lastType = act.gen->type();
				m_temporary << currentEntry->menu->addSeparator();
			}
			const_cast<ActionGenerator *>(act.gen)->setMenuController(const_cast<MenuController *>(m_d->q_ptr));
			if (QAction *action = act.gen->generate<QAction>()) {
				action->setParent(currentEntry->menu);
				currentEntry->menu->addAction(action);
				m_temporary << action;
			}
		}
		m_showed = true;
	}

	void DynamicMenu::onAboutToHide()
	{
		if (!m_showed)
			return;
		qDeleteAll(m_d->temporary);
		m_d->temporary.clear();
		foreach (QAction *action, m_temporary) {
			if (QMenu *menu = action->menu()) {
				quint32 size;
				quint32 current;
				QList<uint> values;
				do {
					unpack_helper(menu->menuAction()->data().toLongLong(), &current, &size);
					values.prepend(current);
					menu = qobject_cast<QMenu*>(menu->parent());
				} while (size > 1);
				ActionEntry *entry = &m_entry;
				for (int i = 0; i < values.size() - 1; i++) {
					QMap<uint, ActionEntry>::iterator it = entry->entries.find(values.at(i));
					if (it == entry->entries.end()) {
						entry = 0;
						break;
					}
					entry = &it.value();
				}
				if (entry)
					entry->entries.remove(values.last());
			}
			action->deleteLater();
			removeAction(action);
		}
		m_temporary.clear();
		m_showed = false;
	}
	
	void DynamicMenu::onActionTriggered(QAction *action)
	{
		const ActionGenerator *gen = action->data().value<ActionGenerator*>();
		if (!gen) {
			qWarning("DynamicMenu::onActionTriggered: Invalid ActionGenerator");
			return;
		}
		const ActionGeneratorPrivate *d = ActionGeneratorPrivate::get(gen);
		QObject *controller = m_owners.value(gen);
		QObject *obj = d->receiver ? d->receiver.data() : controller;
		const QMetaObject *meta = obj->metaObject();
		int index = meta->indexOfMethod(d->member.constData() + 1);
		if (index == -1) {
			qWarning("DynamicMenu::onActionTriggered: No such method %s::%s",
					 meta->className(), d->member.constData() + 1);
			return;
		}
		QMetaMethod method = meta->method(index);
		qDebug("DynamicMenu::onActionTriggered: Trying %s::%s",
			   meta->className(), d->member.constData() + 1);		
		switch (d->connectionType) {
		case ActionConnectionObjectOnly:
			method.invoke(obj, Q_ARG(QObject*, controller));
			break;
		case ActionConnectionActionOnly:
			method.invoke(obj, Q_ARG(QAction*, action));
			break;
		case ActionConnectionFull:
			method.invoke(obj, Q_ARG(QAction*, action), Q_ARG(QObject*, controller));
			break;
		case ActionConnectionSimple:
			method.invoke(obj);
			break;
		default:
			break;
		}
	}

	QMenu *MenuController::menu(bool deleteOnClose) const
	{
		QMenu *menu = new DynamicMenu(d_func());
		menu->setAttribute(Qt::WA_DeleteOnClose, deleteOnClose);
		return menu;
	}

	void MenuController::showMenu(const QPoint &pos)
	{
		menu(true)->popup(pos);
	}

	void MenuController::addAction(const ActionGenerator *gen, const QList<QByteArray> &menu)
	{
		Q_ASSERT(gen);
		d_func()->actions.append(ActionInfo(gen, gen->d_func(), menu));
	}

	bool MenuController::removeAction(const ActionGenerator *gen)
	{
		Q_D(MenuController);
		for (int i = 0; i < d->actions.size(); i++) {
			if (d->actions.at(i).gen == gen) {
				d->actions.removeAt(i);
				return true;
			}
		}
		return false;
	}

	void MenuController::addAction(const ActionGenerator *gen, const QMetaObject *meta, const QList<QByteArray> &menu)
	{
		Q_ASSERT(gen && meta);
		globalActions()->insertMulti(meta, ActionInfo(gen, gen->d_func(), menu));
	}

	void MenuController::setMenuOwner(MenuController *controller)
	{
		Q_D(MenuController);
		d->owner = controller;
	}

	QList<MenuController::Action> MenuController::dynamicActions() const
	{
		return QList<Action>();
	}

	void MenuController::virtual_hook(int id, void *data)
	{
		Q_UNUSED(id);
		Q_UNUSED(data);
	}
}
