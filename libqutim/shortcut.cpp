/****************************************************************************
 *  shortcut.cpp
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

#include "shortcut.h"
#include "configbase.h"
#include "dglobalhotkey_p.h"
#include <QPointer>

namespace qutim_sdk_0_3
{
	class GeneralShortcutInfo
	{
	public:
		LocalizedString name;
		LocalizedString group;
		QKeySequence key;
		bool global;
		bool inited;
	protected:
		GeneralShortcutInfo(bool g) : global(g), inited(false) {}
	};

	class ShortcutInfo : public GeneralShortcutInfo
	{
	public:
		ShortcutInfo() : GeneralShortcutInfo(false), context(Qt::WindowShortcut) {}
		Qt::ShortcutContext context;
		QList<QPointer<Shortcut> > shortcuts;
	};

	class GlobalShortcutInfo : public GeneralShortcutInfo
	{
	public:
		GlobalShortcutInfo() : GeneralShortcutInfo(true) {}
		QList<int> contexts;
		QList<QPointer<GlobalShortcut> > shortcuts;
	};

	typedef QHash<QString, GeneralShortcutInfo *> ShortcutInfoHash;

	struct ShortcutSelf
	{
		ShortcutInfoHash hash;
		void init();
	};

	void ShortcutSelf::init()
	{
		struct _Info
		{
			const char *id;
			LocalizedString name;
			QKeySequence::StandardKey key;
			Qt::ShortcutContext context;
		};
		typedef struct _Info Info;
		Info infos [] = {
			//		{ "helpContents", QT_TRANSLATE_NOOP("Shortcut", "HelpContents"), QKeySequence::HelpContents, Qt::WindowShortcut },
			//		{ "whatsThis", QT_TRANSLATE_NOOP("Shortcut", "WhatsThis"), QKeySequence::WhatsThis, Qt::WindowShortcut },
			//		{ "open", QT_TRANSLATE_NOOP("Shortcut", "Open"), QKeySequence::Open, Qt::WindowShortcut },
			{ "close", QT_TRANSLATE_NOOP("Shortcut", "Close"), QKeySequence::Close, Qt::WindowShortcut },
			//		{ "save", QT_TRANSLATE_NOOP("Shortcut", "Save"), QKeySequence::Save, Qt::WindowShortcut },
			//		{ "new", QT_TRANSLATE_NOOP("Shortcut", "New"), QKeySequence::New, Qt::WindowShortcut },
			//		{ "delete", QT_TRANSLATE_NOOP("Shortcut", "Delete"), QKeySequence::Delete, Qt::WindowShortcut },
			//		{ "cut", QT_TRANSLATE_NOOP("Shortcut", "Cut"), QKeySequence::Cut, Qt::WindowShortcut },
			//		{ "copy", QT_TRANSLATE_NOOP("Shortcut", "Copy"), QKeySequence::Copy, Qt::WindowShortcut },
			//		{ "paste", QT_TRANSLATE_NOOP("Shortcut", "Paste"), QKeySequence::Paste, Qt::WindowShortcut },
			//		{ "undo", QT_TRANSLATE_NOOP("Shortcut", "Undo"), QKeySequence::Undo, Qt::WindowShortcut },
			//		{ "redo", QT_TRANSLATE_NOOP("Shortcut", "Redo"), QKeySequence::Redo, Qt::WindowShortcut },
			//		{ "back", QT_TRANSLATE_NOOP("Shortcut", "Back"), QKeySequence::Back, Qt::WindowShortcut },
			//		{ "forward", QT_TRANSLATE_NOOP("Shortcut", "Forward"), QKeySequence::Forward, Qt::WindowShortcut },
			//		{ "refresh", QT_TRANSLATE_NOOP("Shortcut", "Refresh"), QKeySequence::Refresh, Qt::WindowShortcut },
			//		{ "zoomIn", QT_TRANSLATE_NOOP("Shortcut", "ZoomIn"), QKeySequence::ZoomIn, Qt::WindowShortcut },
			//		{ "zoomOut", QT_TRANSLATE_NOOP("Shortcut", "ZoomOut"), QKeySequence::ZoomOut, Qt::WindowShortcut },
			//		{ "print", QT_TRANSLATE_NOOP("Shortcut", "Print"), QKeySequence::Print, Qt::WindowShortcut },
			//		{ "addTab", QT_TRANSLATE_NOOP("Shortcut", "AddTab"), QKeySequence::AddTab, Qt::WindowShortcut },
			{ "nextChild", QT_TRANSLATE_NOOP("Shortcut", "Next child"), QKeySequence::NextChild, Qt::WindowShortcut },
			{ "previousChild", QT_TRANSLATE_NOOP("Shortcut", "Previous child"), QKeySequence::PreviousChild, Qt::WindowShortcut },
			{ "find", QT_TRANSLATE_NOOP("Shortcut", "Find"), QKeySequence::Find, Qt::WindowShortcut },
			{ "findNext", QT_TRANSLATE_NOOP("Shortcut", "Find next"), QKeySequence::FindNext, Qt::WindowShortcut },
			{ "findPrevious", QT_TRANSLATE_NOOP("Shortcut", "Find previous"), QKeySequence::FindPrevious, Qt::WindowShortcut },
			//		{ "replace", QT_TRANSLATE_NOOP("Shortcut", "Replace"), QKeySequence::Replace, Qt::WindowShortcut },
			//		{ "selectAll", QT_TRANSLATE_NOOP("Shortcut", "SelectAll"), QKeySequence::SelectAll, Qt::WindowShortcut },
			//		{ "bold", QT_TRANSLATE_NOOP("Shortcut", "Bold"), QKeySequence::Bold, Qt::WindowShortcut },
			//		{ "italic", QT_TRANSLATE_NOOP("Shortcut", "Italic"), QKeySequence::Italic, Qt::WindowShortcut },
			//		{ "underline", QT_TRANSLATE_NOOP("Shortcut", "Underline"), QKeySequence::Underline, Qt::WindowShortcut },
			//		{ "moveToNextChar", QT_TRANSLATE_NOOP("Shortcut", "MoveToNextChar"), QKeySequence::MoveToNextChar, Qt::WindowShortcut },
			//		{ "moveToPreviousChar", QT_TRANSLATE_NOOP("Shortcut", "MoveToPreviousChar"), QKeySequence::MoveToPreviousChar, Qt::WindowShortcut },
			//		{ "moveToNextWord", QT_TRANSLATE_NOOP("Shortcut", "MoveToNextWord"), QKeySequence::MoveToNextWord, Qt::WindowShortcut },
			//		{ "moveToPreviousWord", QT_TRANSLATE_NOOP("Shortcut", "MoveToPreviousWord"), QKeySequence::MoveToPreviousWord, Qt::WindowShortcut },
			//		{ "moveToNextLine", QT_TRANSLATE_NOOP("Shortcut", "MoveToNextLine"), QKeySequence::MoveToNextLine, Qt::WindowShortcut },
			//		{ "moveToPreviousLine", QT_TRANSLATE_NOOP("Shortcut", "MoveToPreviousLine"), QKeySequence::MoveToPreviousLine, Qt::WindowShortcut },
			//		{ "moveToNextPage", QT_TRANSLATE_NOOP("Shortcut", "MoveToNextPage"), QKeySequence::MoveToNextPage, Qt::WindowShortcut },
			//		{ "moveToPreviousPage", QT_TRANSLATE_NOOP("Shortcut", "MoveToPreviousPage"), QKeySequence::MoveToPreviousPage, Qt::WindowShortcut },
			//		{ "moveToStartOfLine", QT_TRANSLATE_NOOP("Shortcut", "MoveToStartOfLine"), QKeySequence::MoveToStartOfLine, Qt::WindowShortcut },
			//		{ "moveToEndOfLine", QT_TRANSLATE_NOOP("Shortcut", "MoveToEndOfLine"), QKeySequence::MoveToEndOfLine, Qt::WindowShortcut },
			//		{ "moveToStartOfBlock", QT_TRANSLATE_NOOP("Shortcut", "MoveToStartOfBlock"), QKeySequence::MoveToStartOfBlock, Qt::WindowShortcut },
			//		{ "moveToEndOfBlock", QT_TRANSLATE_NOOP("Shortcut", "MoveToEndOfBlock"), QKeySequence::MoveToEndOfBlock, Qt::WindowShortcut },
			//		{ "moveToStartOfDocument", QT_TRANSLATE_NOOP("Shortcut", "MoveToStartOfDocument"), QKeySequence::MoveToStartOfDocument, Qt::WindowShortcut },
			//		{ "moveToEndOfDocument", QT_TRANSLATE_NOOP("Shortcut", "MoveToEndOfDocument"), QKeySequence::MoveToEndOfDocument, Qt::WindowShortcut },
			//		{ "selectNextChar", QT_TRANSLATE_NOOP("Shortcut", "SelectNextChar"), QKeySequence::SelectNextChar, Qt::WindowShortcut },
			//		{ "selectPreviousChar", QT_TRANSLATE_NOOP("Shortcut", "SelectPreviousChar"), QKeySequence::SelectPreviousChar, Qt::WindowShortcut },
			//		{ "selectNextWord", QT_TRANSLATE_NOOP("Shortcut", "SelectNextWord"), QKeySequence::SelectNextWord, Qt::WindowShortcut },
			//		{ "selectPreviousWord", QT_TRANSLATE_NOOP("Shortcut", "SelectPreviousWord"), QKeySequence::SelectPreviousWord, Qt::WindowShortcut },
			//		{ "selectNextLine", QT_TRANSLATE_NOOP("Shortcut", "SelectNextLine"), QKeySequence::SelectNextLine, Qt::WindowShortcut },
			//		{ "selectPreviousLine", QT_TRANSLATE_NOOP("Shortcut", "SelectPreviousLine"), QKeySequence::SelectPreviousLine, Qt::WindowShortcut },
			//		{ "selectNextPage", QT_TRANSLATE_NOOP("Shortcut", "SelectNextPage"), QKeySequence::SelectNextPage, Qt::WindowShortcut },
			//		{ "selectPreviousPage", QT_TRANSLATE_NOOP("Shortcut", "SelectPreviousPage"), QKeySequence::SelectPreviousPage, Qt::WindowShortcut },
			//		{ "selectStartOfLine", QT_TRANSLATE_NOOP("Shortcut", "SelectStartOfLine"), QKeySequence::SelectStartOfLine, Qt::WindowShortcut },
			//		{ "selectEndOfLine", QT_TRANSLATE_NOOP("Shortcut", "SelectEndOfLine"), QKeySequence::SelectEndOfLine, Qt::WindowShortcut },
			//		{ "selectStartOfBlock", QT_TRANSLATE_NOOP("Shortcut", "SelectStartOfBlock"), QKeySequence::SelectStartOfBlock, Qt::WindowShortcut },
			//		{ "selectEndOfBlock", QT_TRANSLATE_NOOP("Shortcut", "SelectEndOfBlock"), QKeySequence::SelectEndOfBlock, Qt::WindowShortcut },
			//		{ "selectStartOfDocument", QT_TRANSLATE_NOOP("Shortcut", "SelectStartOfDocument"), QKeySequence::SelectStartOfDocument, Qt::WindowShortcut },
			//		{ "selectEndOfDocument", QT_TRANSLATE_NOOP("Shortcut", "SelectEndOfDocument"), QKeySequence::SelectEndOfDocument, Qt::WindowShortcut },
			//		{ "deleteStartOfWord", QT_TRANSLATE_NOOP("Shortcut", "DeleteStartOfWord"), QKeySequence::DeleteStartOfWord, Qt::WindowShortcut },
			//		{ "deleteEndOfWord", QT_TRANSLATE_NOOP("Shortcut", "DeleteEndOfWord"), QKeySequence::DeleteEndOfWord, Qt::WindowShortcut },
			//		{ "deleteEndOfLine", QT_TRANSLATE_NOOP("Shortcut", "DeleteEndOfLine"), QKeySequence::DeleteEndOfLine, Qt::WindowShortcut },
			//		{ "insertParagraphSeparator", QT_TRANSLATE_NOOP("Shortcut", "InsertParagraphSeparator"), QKeySequence::InsertParagraphSeparator, Qt::WindowShortcut },
			//		{ "insertLineSeparator", QT_TRANSLATE_NOOP("Shortcut", "InsertLineSeparator"), QKeySequence::InsertLineSeparator, Qt::WindowShortcut },
			//		{ "saveAs", QT_TRANSLATE_NOOP("Shortcut", "SaveAs"), QKeySequence::SaveAs, Qt::WindowShortcut },
			{ "preferences", QT_TRANSLATE_NOOP("Shortcut", "Preferences"), QKeySequence::Preferences, Qt::ApplicationShortcut },
			{ "quit", QT_TRANSLATE_NOOP("Shortcut", "Quit"), QKeySequence::Quit, Qt::ApplicationShortcut }
		};
		LocalizedString group = QT_TRANSLATE_NOOP("Shortcut", "System");
		for (int i = 0, size = sizeof(infos)/sizeof(Info); i < size; i++) {
			const Info &it = infos[i];
			ShortcutInfo *info = new ShortcutInfo();
			info->name = it.name;
			info->group = group;
			info->context = it.context;
			info->key = QKeySequence(it.key);
			info->inited = true;
			hash.insert(QLatin1String(it.id), info);
		}
	}

	Q_GLOBAL_STATIC_WITH_INITIALIZER(ShortcutSelf, self, x->init())

	class ShortcutPrivate
	{
	public:
		ShortcutInfo *info;
	};

	class GlobalShortcutPrivate
	{
		Q_DECLARE_PUBLIC(GlobalShortcut)
	public:
				GlobalShortcutPrivate(GlobalShortcut *q) : info(0), q_ptr(q) {}
		GlobalShortcutInfo *info;
		GlobalShortcut *q_ptr;
	};

	Shortcut::Shortcut(const QString &id, QWidget *parent) :
			QShortcut(parent), d_ptr(new ShortcutPrivate)
	{
		Q_D(Shortcut);
		GeneralShortcutInfo *info = self()->hash.value(id);
		if (!info) {
			info = new ShortcutInfo();
			self()->hash.insert(id, info);
		}
		if (info && !info->global) {
			d->info = static_cast<ShortcutInfo*>(info);
			d->info->shortcuts.append(this);
			setKey(d->info->key);
			setContext(d->info->context);
		}
	}

	Shortcut::~Shortcut()
	{
	}

	bool Shortcut::registerSequence(const QString &id, const LocalizedString &name,
									const LocalizedString &group, const QKeySequence &key,
									Qt::ShortcutContext context)
	{
		ShortcutInfoHash &hash = self()->hash;
		ShortcutInfoHash::iterator it = hash.find(id);
		ShortcutInfo *info = 0;
		if (it != hash.end()) {
			GeneralShortcutInfo *i = it.value();
			if (i->global)
				return false;
			info = static_cast<ShortcutInfo*>(i);
		} else {
			info = new ShortcutInfo();
		}
		if (!info->inited) {
			info->inited = true;
			info->name = name;
			info->group = group;
			info->key = Config("profile").group("shortcuts").value<QKeySequence>(id,key);
			info->context = context;
			info->shortcuts.removeAll(NULL);
			foreach (Shortcut *shortcut, info->shortcuts) {
				shortcut->setKey(info->key);
				shortcut->setContext(info->context);
			}
			hash.insert(id, info);
			return true;
		}
		return false;
	}

	GlobalShortcut::GlobalShortcut(const QString &id, QObject *parent) :
			QObject(parent), d_ptr(new GlobalShortcutPrivate(this))
	{
		Q_D(GlobalShortcut);
		GeneralShortcutInfo *info = self()->hash.value(id);
		if (!info) {
			info = new GlobalShortcutInfo();
			self()->hash.insert(id, info);
		}
		if (info->global) {
			d->info = static_cast<GlobalShortcutInfo*>(info);
			d->info->shortcuts.append(this);
			connect(dGlobalHotKey::instance(), SIGNAL(hotKeyPressed(quint32)),
					this, SLOT(onHotKeyPressed(quint32)));
		}
	}

	GlobalShortcut::~GlobalShortcut()
	{
	}

	bool GlobalShortcut::registerSequence(const QString &id, const LocalizedString &name,
										  const LocalizedString &group, const QKeySequence &key)
	{
		ShortcutInfoHash &hash = self()->hash;
		ShortcutInfoHash::iterator it = hash.find(id);
		GlobalShortcutInfo *info = 0;
		if (it != hash.end()) {
			GeneralShortcutInfo *i = it.value();
			if (!i->global)
				return false;
			info = static_cast<GlobalShortcutInfo*>(i);
		} else {
			info = new GlobalShortcutInfo();
		}
		if (!info->inited) {
			info->inited = true;
			info->name = name;
			info->group = group;
			info->key = Config("profile").group("globalShortcuts").value<QKeySequence>(id,key);
			for (uint i = 0, count = key.count(); i < count; i++) {
				QString str = QKeySequence(key[i]).toString();
				int k = dGlobalHotKey::instance()->id(str);
				if (dGlobalHotKey::instance()->shortcut(str, true))
					info->contexts.append(k);
				else
					continue;
			}
			hash.insert(id, info);
			return true;
		}
		return false;
	}

	void GlobalShortcut::onHotKeyPressed(quint32 k)
	{
		Q_D(GlobalShortcut);
		if (d->info && d->info->contexts.contains(k))
			emit activated();
	}

	QStringList Shortcut::ids()
	{
		ShortcutInfoHash &hash = self()->hash;
		ShortcutInfoHash::const_iterator it;
		QStringList ids;
		for (it = hash.constBegin();it!=hash.constEnd();it++)
			if (!it.value()->global)
				ids << it.key();
		return ids;
	}

	KeySequence Shortcut::getSequence(const QString &id)
	{
		KeySequence sequence;
		GeneralShortcutInfo *info = self()->hash.value(id);
		if (!info->inited || info->global)
			return sequence;
		sequence.id = id;
		sequence.name = info->name;
		sequence.group = info->group;
		sequence.key = info->key;
		return sequence;
	}

	void Shortcut::setSequence(const QString &id, const QKeySequence &key)
	{
		ConfigGroup config = Config("profile").group("shortcuts");
		config.setValue(id,key);
		config.sync();
	}

	QStringList GlobalShortcut::ids()
	{
		ShortcutInfoHash &hash = self()->hash;
		ShortcutInfoHash::const_iterator it;
		QStringList ids;
		for (it = hash.constBegin();it!=hash.constEnd();it++)
			if (it.value()->global)
				ids << it.key();
		return ids;
	}

	KeySequence GlobalShortcut::getSequence(const QString &id)
	{
		KeySequence sequence;
		GeneralShortcutInfo *info = self()->hash.value(id);
		if (!info->inited || !info->global)
			return sequence;
		sequence.id = id;
		sequence.name = info->name;
		sequence.group = info->group;
		sequence.key = info->key;
		return sequence;
	}

	void GlobalShortcut::setSequence(const QString &id, const QKeySequence &key)
	{
		ConfigGroup config = Config("profile").group("globalShortcuts");
		config.setValue(id,key);
		config.sync();
	}
}
