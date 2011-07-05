/****************************************************************************
 *  macwidget.cpp
 *
 *  Copyright (c) 2011 by Denis Daschenko <daschenko@gmail.com>
 *  Sidorov Aleksey <sauron@citadelspb.com>
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



#include "macwidget.h"
#include <qutim/simplecontactlist/abstractcontactmodel.h>
#include <qutim/simplecontactlist/simplestatusdialog.h>
#include <qutim/account.h>
#include <qutim/actiongenerator.h>
#include <qutim/actiontoolbar.h>
#include <qutim/config.h>
#include <qutim/contact.h>
#include <qutim/icon.h>
#include <qutim/messagesession.h>
#include <qutim/metacontact.h>
#include <qutim/protocol.h>
#include <qutim/qtwin.h>
#include <qutim/servicemanager.h>
#include <qutim/shortcut.h>
#include <qutim/systemintegration.h>
#include <qutim/utils.h>
#include <QAbstractItemDelegate>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QTimer>
#include <QKeyEvent>

namespace Core {
namespace SimpleContactList {
struct MacWidgetPrivate
{
	TreeView *view;
	AbstractContactModel *model;
	QLineEdit *searchBar;
	QAction *statusTextAction;
	QVector<MenuController *> controllers;
	QVector<QMenu *> menus;
	QHash<Account *, QAction *> accountActions;
	QHash<ChatSession *, QAction *> aliveSessions;
	QMenuBar *menuBar;
	QString pressedKeys;
};

MacWidget::MacWidget() : d_ptr(new MacWidgetPrivate())
{
	Q_D(MacWidget);
	d->controllers.resize(MacMenuSize);
	d->menus.resize(MacMenuSize);
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
	setWindowIcon(Icon("qutim"));

	resize(150,0);//hack
	setAttribute(Qt::WA_AlwaysShowToolTips);
	loadGeometry();

	QWidget *w = new QWidget(this);
	setCentralWidget(w);
	setUnifiedTitleAndToolBarOnMac(true);

	QVBoxLayout *layout = new QVBoxLayout(w);
	layout->setMargin(1);
	layout->setSpacing(0);

	if (QtWin::isCompositionEnabled()) {
		QtWin::extendFrameIntoClientArea(this);
		setContentsMargins(0, 0, 0, 0);
	}

	Config cfg;
	cfg.beginGroup("contactlist");

	d->model = ServiceManager::getByName<AbstractContactModel *>("ContactModel");
	d->view = new TreeView(d->model, this);
	layout->addWidget(d->view);
	d->view->setItemDelegate(ServiceManager::getByName<QAbstractItemDelegate *>("ContactDelegate"));
	d->view->setAlternatingRowColors(cfg.value("alternatingRowColors", false));
	d->view->setFrameShape(QFrame::NoFrame);
	d->view->setFrameShadow(QFrame::Plain);
	d->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	d->searchBar = new QLineEdit(this);
	layout->insertWidget(0, d->searchBar);
	connect(d->searchBar, SIGNAL(textChanged(QString)), d->model, SLOT(filterList(QString)));
	connect(d->searchBar, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
	d->searchBar->setVisible(false);
	d->view->installEventFilter(this);
	d->searchBar->installEventFilter(this);

	qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
#ifdef Q_OS_MAC
	d->menuBar = new QMenuBar(0);
#else
	d->menuBar = new QMenuBar(this);
#endif
	addMenu(tr("Accounts"), MacMenuAccounts);
	addMenu(tr("Chats"), MacMenuChats);
	addMenu(tr("Roster"), MacMenuRoster);
	//if (MenuController *contactList = qobject_cast<MenuController *>(ServiceManager::getByName("ContactList")))
	//	d->menu.value(MacMenuFile)->setMenuOwner(contactList);
	d->statusTextAction = d->menus[MacMenuAccounts]->addAction(Icon("im-status-message-edit"), tr("Set Status Text"),
																	   this, SLOT(showStatusDialog()));
	QString lastStatus = Config().group("contactList").value("lastStatus", QString());
	d->statusTextAction->setData(lastStatus);
	d->menus[MacMenuAccounts]->addSeparator();
	foreach(Protocol *protocol, Protocol::all())
		connect(protocol, SIGNAL(accountCreated(qutim_sdk_0_3::Account *)), this, SLOT(onAccountCreated(qutim_sdk_0_3::Account *)));
	QTimer timer;
	timer.singleShot(0, this, SLOT(initMenu()));
}

MacWidget::~MacWidget()
{
	Q_D(MacWidget);
	qDeleteAll(d->menus);
	Config config;
	config.beginGroup("contactList");
	config.setValue("geometry", saveGeometry());
}

class MacMenuFileController : public MenuController
{
	public:
		MacMenuFileController(QObject *parent) : MenuController(parent)
		{
			if (MenuController *contactList = ServiceManager::getByName<MenuController *>("ContactList"))
				setMenuOwner(contactList);
		}
};

void MacWidget::addMenu(const QString &title, MacMenuId id)
{
	Q_D(MacWidget);
	MenuController *controller = 0;
	if (id == MacMenuFile)
		controller = new MacMenuFileController(this);
	else
		controller = new MenuController(this);
	QMenu *menu = controller->menu(false);
	menu->setTitle(title);
	d->menus[id] = menu;
	d->controllers[id] = controller;
}

void MacWidget::addButton(ActionGenerator *generator)
{
	d_func()->controllers[MacMenuRoster]->addAction(generator);
}

void MacWidget::removeButton(ActionGenerator *generator)
{
	d_func()->controllers[MacMenuRoster]->removeAction(generator);
}

void MacWidget::loadGeometry()
{
	QByteArray geom = Config().group("contactList").value("geometry", QByteArray());
	if (geom.isNull()) {
		QRect rect = QApplication::desktop()->availableGeometry(QCursor::pos());
		//black magic
		int width = size().width();
		int x = rect.width() - width;
		int y = 0;
		int height = rect.height();
		QRect geometry(x, y, width, height);
		setGeometry(geometry);
	} else {
		restoreGeometry(geom);
	}
}

void MacWidget::showStatusDialog()
{
	QString text = d_func()->statusTextAction->data().toString();
	SimpleStatusDialog *dialog = new SimpleStatusDialog(text, this);
	connect(dialog, SIGNAL(accepted()), SLOT(changeStatusTextAccepted()));
	centerizeWidget(dialog);
	SystemIntegration::show(dialog);
}

void MacWidget::changeStatusTextAccepted()
{
	SimpleStatusDialog *dialog = qobject_cast<SimpleStatusDialog *>(sender());
	Q_ASSERT(dialog);
	QString text = dialog->statusText();
	d_func()->statusTextAction->setData(text);
	foreach(Protocol *proto, Protocol::all()) {
		foreach(Account *account, proto->accounts()) {
			Status status = account->status();
			status.setText(text);
			account->setStatus(status);
		}
	}
	Config config = Config().group("contactList");
	config.setValue("lastStatus",text);
	config.sync();
}

void MacWidget::onAccountCreated(qutim_sdk_0_3::Account *account)
{
	Q_D(MacWidget);
	QAction *action = new QAction(account->status().icon(), account->id(), this);
	action->setIconVisibleInMenu(true);
	connect(account, SIGNAL(statusChanged(qutim_sdk_0_3::Status,qutim_sdk_0_3::Status)),
			this, SLOT(onAccountStatusChanged(qutim_sdk_0_3::Status)));
	connect(account, SIGNAL(destroyed(QObject *)),SLOT(onAccountDestroyed(QObject *)));
	d->accountActions.insert(account, action);
	action->setMenu(account->menu());
	d->menus[MacMenuAccounts]->addAction(action);
	QString text = d->statusTextAction->data().toString();
	if (!text.isEmpty()) {
		Status status = account->status();
		status.setText(text);
		account->setStatus(status);
	}
}

void MacWidget::onAccountStatusChanged(const qutim_sdk_0_3::Status &status)
{
	Q_D(MacWidget);
	Account *account = qobject_cast<Account *>(sender());
	Q_ASSERT(account);
	QAction *action = d->accountActions.value(account);
	Q_ASSERT(action);
	action->setIcon(status.icon());
}

void MacWidget::onAccountDestroyed(QObject *obj)
{
	Account *account = reinterpret_cast<Account *>(obj);
	d_func()->accountActions.take(account)->deleteLater();
}

void MacWidget::onSessionCreated(qutim_sdk_0_3::ChatSession *session)
{
	Q_D(MacWidget);
	QAction *action = new QAction(session->getUnit()->title(), d->menus[MacMenuChats]);
	action->setCheckable(true);
	connect(action, SIGNAL(triggered()), session, SLOT(activate()));
	d->menus[MacMenuChats]->addAction(action);
	d->aliveSessions.insert(session, action);
	connect(session, SIGNAL(activated(bool)), this, SLOT(onActivatedSession(bool)));
	connect(session, SIGNAL(destroyed()), SLOT(onSessionDestroyed()));
}

void MacWidget::onSessionDestroyed()
{
	ChatSession *session = static_cast<ChatSession *>(sender());
	delete d_func()->aliveSessions.take(session);
}

void MacWidget::onActivatedSession(bool state)
{
	ChatSession *session = static_cast<ChatSession *>(sender());
	d_func()->aliveSessions.value(session)->setChecked(state);
}

void MacWidget::initMenu()
{
	Q_D(MacWidget);
	addMenu(tr("File"), MacMenuFile);
	d->menuBar->addMenu(d->menus[MacMenuFile]);
	d->menuBar->addMenu(d->menus[MacMenuAccounts]);
	d->menuBar->addMenu(d->menus[MacMenuChats]);
	d->menuBar->addMenu(d->menus[MacMenuRoster]);
	connect(ChatLayer::instance(), SIGNAL(sessionCreated(qutim_sdk_0_3::ChatSession *)),
			this, SLOT(onSessionCreated(qutim_sdk_0_3::ChatSession *)));
}

bool MacWidget::eventFilter(QObject *obj, QEvent *ev)
{
	Q_D(MacWidget);
	if (obj == d->view) {
		if (ev->type() == QEvent::KeyPress) {
			QKeyEvent *event = static_cast<QKeyEvent*>(ev);
			if (d->view->hasFocus() && d->searchBar->isHidden())
				d->pressedKeys.append(event->text());

			if (d->pressedKeys.count() > 1) {
				d->searchBar->show();
				d->searchBar->setText(d->pressedKeys);
				d->searchBar->setFocus();
			}
			ev->accept();
		} else if (ev->type() == QEvent::FocusOut && d->searchBar->isHidden()) {
			d->pressedKeys.clear();
		}
	} else if (obj == d->searchBar) {
		if (ev->type() == QEvent::FocusOut) {
			d->pressedKeys.clear();
			//d->searchBar->clear();
			//d->searchBar->hide();
		} else if (ev->type() == QEvent::FocusIn)
			d->pressedKeys.clear();
	}
	return QMainWindow::eventFilter(obj, ev);
}

void MacWidget::onTextChanged(const QString &text)
{
	d_func()->searchBar->setVisible(!text.isEmpty());
}

} // namespace SimpleContactList
} // namespace Core
