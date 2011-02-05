#include "jprotocol.h"
#include "jmainsettings.h"
#include "account/jaccount.h"
#include <qutim/icon.h>
#include <qutim/servicemanager.h>
#include "account/roster/jresourceactiongenerator.h"
#include "account/roster/jcontact.h"
#include "account/dataform/jdataform.h"
#include <qutim/statusactiongenerator.h>
#include <qutim/settingslayer.h>
#include "account/muc/jmucuser.h"
#include "account/muc/jmucsession.h"
#include "account/muc/jmucmanager.h"
#include "account/muc/jbookmarkmanager.h"
#include "account/roster/jmessagesession.h"
#include "account/muc/jconferenceconfig.h"
#include <QInputDialog>
#include <qutim/debug.h>

namespace Jabber
{

enum JActionType
{
	JoinLeaveAction,
	SaveRemoveBookmarkAction,
	RoomConfigAction,
	RoomParticipantsAction,
	ChangeSubcriptionAction,
	KickAction,
	BanAction
};

class JProtocolPrivate
{
	Q_DECLARE_PUBLIC(JProtocol)
public:
	inline JProtocolPrivate(JProtocol *q) :
		accounts(new QHash<QString, JAccount *>),
		q_ptr(q)
	{
	}
	inline ~JProtocolPrivate() { delete accounts; }
	QHash<QString, JAccount *> *accounts;
	JProtocol *q_ptr;
	SettingsItem *mainSettings;
	QScopedPointer<ActionGenerator> subscribeGen;
	QScopedPointer<ActionGenerator> roomConfigGen;
	void checkSubscribe(JContact *c, QAction *a)
	{
		a->setEnabled(c->account()->status() != Status::Offline);
		LocalizedString str;
		switch(c->subscription()) {
		case jreen::AbstractRosterItem::Both:
		case jreen::AbstractRosterItem::To:
			str = QT_TRANSLATE_NOOP("Jabber", "Remove subscription");
			break;
		case jreen::AbstractRosterItem::From:
		case jreen::AbstractRosterItem::None:
		case jreen::AbstractRosterItem::Invalid:
			str = QT_TRANSLATE_NOOP("Jabber", "Request subscription");
			break;
		default:
			break;
		}
		a->setText(str);
	}
	void checkRoomConfig(JMUCSession *s, QAction *a)
	{
		a->setEnabled(s->enabledConfiguring());
	}
	void _q_status_changed(qutim_sdk_0_3::Status)
	{
		QMap<QObject*, QAction*> actions = subscribeGen->actions();
		QMap<QObject*, QAction*>::const_iterator it = actions.constBegin();
		for(;it != actions.constEnd(); it++) {
			//TODO may be possible use reinterpret_cast?
			JContact *c = qobject_cast<JContact*>(it.key());
			Q_ASSERT(c);
			checkSubscribe(c, it.value());
		}
		//actions = roomConfigGen->actions();
		//it = actions.begin();
		//for(;it != actions.constEnd(); it++) {
		//	//TODO may be possible use reinterpret_cast?
		//	JMUCSession *s = qobject_cast<JMUCSession*>(it.key());
		//	Q_ASSERT(s);
		//	checkRoomConfig(s, it.value());
		//}
	}
	void _q_conference_join_changed()
	{
		Q_Q(JProtocol);
		JMUCSession *s = qobject_cast<JMUCSession*>(q->sender());
		Q_ASSERT(s);
		foreach (QAction *a, roomConfigGen->actions(s))
			checkRoomConfig(s, a);
	}
	void _q_subscription_changed(jreen::AbstractRosterItem::SubscriptionType)
	{
		Q_Q(JProtocol);
		JContact *c = qobject_cast<JContact*>(c);
		Q_ASSERT(c);
		foreach (QAction *a, subscribeGen->actions(q->sender()))
			checkSubscribe(c,a);
	}
};

JProtocol *JProtocol::self = 0;

JProtocol::JProtocol() : d_ptr(new JProtocolPrivate(this))
{
	Q_ASSERT(!self);
	self = this;
}

JProtocol::~JProtocol()
{
	self = 0;
}

QList<Account *> JProtocol::accounts() const
{
	QList<Account *> accounts;
	foreach (JAccount *account, d_ptr->accounts->values())
		accounts.append(account);
	return accounts;
}

Account *JProtocol::account(const QString &id) const
{
	return d_func()->accounts->value(id);
}

void JProtocol::loadActions()
{
	Q_D(JProtocol);
	d->mainSettings = new GeneralSettingsItem<JMainSettings>(Settings::Protocol,
															 Icon("im-jabber"),
															 QT_TRANSLATE_NOOP("Settings", "Main settings"));

	Settings::registerItem<JAccount>(d->mainSettings);

	Settings::registerItem<JMUCSession>(new GeneralSettingsItem<JConferenceConfig>(
	                                        Settings::Protocol,
	                                        QIcon(),
	                                        QT_TRANSLATE_NOOP("Settings", "Room configuration")));

	ActionGenerator *generator  = new ActionGenerator(Icon("im-kick-user"),
	                                                  QT_TRANSLATE_NOOP("Conference", "Kick"),
	                                                  this, SLOT(onKickUser(QObject*)));
	generator->addHandler(ActionVisibilityChangedHandler, this);
	generator->addProperty("actionType", KickAction);
	MenuController::addAction<JMUCUser>(generator);
	
	generator  = new ActionGenerator(Icon("im-ban-user"), QT_TRANSLATE_NOOP("Conference", "Ban"),
	                                 this, SLOT(onBanUser(QObject*)));
	generator->addHandler(ActionVisibilityChangedHandler, this);
	generator->addProperty("actionType", BanAction);
	MenuController::addAction<JMUCUser>(generator);
	
	//MenuController::addAction<JMessageSession>(
	//			new ActionGenerator(QIcon(), QT_TRANSLATE_NOOP("Conference", "Convert to conference"),
	//								this, SLOT(onConvertToMuc(QObject*))));

	generator  = new ActionGenerator(QIcon(),QT_TRANSLATE_NOOP("Jabber", "Join conference"),
	                                 this, SLOT(onJoinLeave(QObject*)));
	generator->addHandler(ActionVisibilityChangedHandler,this);
	generator->setType(ActionTypeAdditional);
	generator->setPriority(3);
	generator->addProperty("actionType",JoinLeaveAction);
	MenuController::addAction<JMUCSession>(generator);

	d->roomConfigGen.reset(new ActionGenerator(Icon("preferences-other"), QT_TRANSLATE_NOOP("Jabber", "Room's configuration"),
											   this, SLOT(onShowConfigDialog(QObject*))));
	d->roomConfigGen->addHandler(ActionCreatedHandler, this);
	d->roomConfigGen->setType(ActionTypeChatButton);
	MenuController::addAction<JMUCSession>(d->roomConfigGen.data());

	generator = new ActionGenerator(QIcon(), QT_TRANSLATE_NOOP("Jabber", "Save to bookmarks") ,
									this, SLOT(onSaveRemoveBookmarks(QObject*)));
	generator->addHandler(ActionVisibilityChangedHandler,this);
	generator->setType(ActionTypeAdditional);
	generator->setPriority(0);
	generator->addProperty("actionType",SaveRemoveBookmarkAction);
	MenuController::addAction<JMUCSession>(generator);

	d->subscribeGen.reset(new ActionGenerator(QIcon(), QT_TRANSLATE_NOOP("Jabber", "Subscription") ,
											  this, SLOT(onChangeSubscription(QObject*))));
	d->subscribeGen->addHandler(ActionCreatedHandler,this);
	d->subscribeGen->setType(0);
	d->subscribeGen->setPriority(0);
	MenuController::addAction<JContact>(d->subscribeGen.data());

	QList<Status> statuses;
	statuses << Status(Status::Online)
			 << Status(Status::FreeChat)
			 << Status(Status::Away)
			 << Status(Status::NA)
			 << Status(Status::DND)
			 << Status(Status::Invisible)
			 << Status(Status::Offline);
	Status connectingStatus(Status::Connecting);
	connectingStatus.initIcon("jabber");
	Status::remember(connectingStatus, "jabber");

	foreach (Status status, statuses) {
		status.initIcon("jabber");
		Status::remember(status, "jabber");
		MenuController::addAction(new StatusActionGenerator(status), &JAccount::staticMetaObject);
	}


}

void JProtocol::onKickUser(QObject *obj)
{
	JMUCUser *user = qobject_cast<JMUCUser*>(obj);
	Q_ASSERT(user);
	JMUCSession *muc = static_cast<JMUCSession *>(user->upperUnit());
	QString reason = QInputDialog::getText(0, tr("Kick"), tr("Enter kick reason for %1").arg(user->name()));
	muc->room()->kick(user->name(), reason);
}

void JProtocol::onBanUser(QObject *obj)
{
	JMUCUser *user = qobject_cast<JMUCUser*>(obj);
	Q_ASSERT(user);
	JMUCSession *muc = static_cast<JMUCSession *>(user->upperUnit());
	QString reason = QInputDialog::getText(0, tr("Ban"), tr("Enter ban reason for %1").arg(user->name()));
	muc->room()->ban(user->name(), reason);
}

void JProtocol::onConvertToMuc(QObject *obj)
{
	//JMessageSession *session = qobject_cast<JMessageSession*>(obj);
	//Q_ASSERT(session);
	//session->convertToMuc();
}

void JProtocol::onJoinLeave(QObject* obj)
{
	JMUCSession *room = qobject_cast<JMUCSession*>(obj);
	debug() << Q_FUNC_INFO << obj;
	Q_ASSERT(room);
	if (!room->isJoined()) {
		//		JAccount *account = static_cast<JAccount*>(room->account());
		room->join();
		//		account->conferenceManager()->join(room->id());
	}
	else
		room->leave();
}

void JProtocol::onShowConfigDialog(QObject* obj)
{
	JMUCSession *room = qobject_cast<JMUCSession*>(obj);
	Q_ASSERT(room);
	if (!room->enabledConfiguring())
		return;
	SettingsLayer *layer = ServiceManager::getByName<SettingsLayer*>("SettingsLayer");
	layer->show(room);
	//	if (room->enabledConfiguring())
	//		room->showConfigDialog();
}

void JProtocol::onSaveRemoveBookmarks(QObject *obj)
{
	//TODO move to joingroupchat module
	JMUCSession *room = qobject_cast<JMUCSession*>(obj);
	Q_ASSERT(room);
	JAccount *account = static_cast<JAccount*>(room->account());
	JBookmarkManager *manager = account->conferenceManager()->bookmarkManager();
	if (!room->bookmark().isValid()) {
		QString name = room->id();
		manager->saveBookmark(-1,
							  name,
							  room->id(),
							  room->me()->name(),
							  QString());
	} else {
		manager->removeBookmark(room->bookmark());
		room->setBookmark(jreen::Bookmark::Conference());
	}
}

void JProtocol::onChangeSubscription(QObject *obj)
{
	JContact *contact = qobject_cast<JContact*>(obj);
	Q_ASSERT(contact);
	switch(contact->subscription()) {
	case jreen::AbstractRosterItem::Both:
	case jreen::AbstractRosterItem::To:
		contact->removeSubscription();
		break;
	case jreen::AbstractRosterItem::From:
	case jreen::AbstractRosterItem::None:
	case jreen::AbstractRosterItem::Invalid:
		contact->requestSubscription();
		break;
	default:
		break;
	}
}

void JProtocol::loadAccounts()
{
	loadActions();
	QStringList accounts = config("general").value("accounts", QStringList());
	foreach(const QString &id, accounts) {
		jreen::JID jid = id;
		addAccount(new JAccount(jid.bare()), true);
	}
}

void JProtocol::addAccount(JAccount *account, bool isEmit)
{
	Q_D(JProtocol);
	d->accounts->insert(account->id(), account);
	if(isEmit)
		emit accountCreated(account);

	connect(account, SIGNAL(destroyed(QObject*)),
			this, SLOT(removeAccount(QObject*)));
	d->mainSettings->connect(SIGNAL(saved()),
							 account, SLOT(loadSettings()));
}

QVariant JProtocol::data(DataType type)
{
	switch (type) {
	case ProtocolIdName:
		return QLatin1String("Jabber ID");
	case ProtocolContainsContacts:
		return true;
	default:
		return QVariant();
	}
}

jreen::Presence::Type JStatus::statusToPresence(const Status &status)
{
	jreen::Presence::Type presence;
	switch (status.type()) {
	case Status::Offline:
		presence = jreen::Presence::Unavailable;
		break;
	case Status::Online:
		presence = jreen::Presence::Available;
		break;
	case Status::Away:
		presence = jreen::Presence::Away;
		break;
	case Status::FreeChat:
		presence = jreen::Presence::Chat;
		break;
	case Status::DND:
		presence = jreen::Presence::DND;
		break;
	case Status::NA:
		presence = jreen::Presence::XA;
		break;
	case Status::Invisible:
		presence = jreen::Presence::XA;
		break;
	default:
		presence = jreen::Presence::Invalid;
	}
	return presence;
}

Status JStatus::presenceToStatus(jreen::Presence::Type presence)
{
	Status::Type status;
	switch (presence) {
	case jreen::Presence::Available:
		status = Status::Online;
		break;
	case jreen::Presence::Away:
		status = Status::Away;
		break;
	case jreen::Presence::Chat:
		status = Status::FreeChat;
		break;
	case jreen::Presence::DND:
		status = Status::DND;
		break;
	case jreen::Presence::XA:
		status = Status::NA;
		break;
	case jreen::Presence::Error:
	case jreen::Presence::Unavailable:
	default: //TODO probe,subscribe etc. isn't offline status
		status = Status::Offline;
	}
	return Status::instance(status, "jabber");
}

bool JProtocol::event(QEvent *ev)
{
	Q_D(JProtocol);
	if (ev->type() == ActionCreatedEvent::eventType()) {
		ActionCreatedEvent *event = static_cast<ActionCreatedEvent*>(ev);
		QAction *action = event->action();
		QObject *controller = event->controller();
		ChatUnit *u = qobject_cast<ChatUnit*>(controller);
		Q_ASSERT(u);
		if (event->generator() == d->subscribeGen.data()) {
			JContact *c = static_cast<JContact*>(u);
			d->checkSubscribe(c, action);
			connect(c, SIGNAL(subscriptionChanged(jreen::AbstractRosterItem::SubscriptionType)),
					this, SLOT(_q_subscription_changed(jreen::AbstractRosterItem::SubscriptionType)));
		} else if (event->generator() == d->roomConfigGen.data()) {
			JMUCSession *s = static_cast<JMUCSession*>(u);
			d->checkRoomConfig(s, action);
			connect(s, SIGNAL(joinedChanged(bool)),
					this, SLOT(_q_conference_join_changed()));
		}
		connect(u->account(), SIGNAL(statusChanged(qutim_sdk_0_3::Status,qutim_sdk_0_3::Status)),
				this, SLOT(_q_status_changed(qutim_sdk_0_3::Status)));
		return true;
	} else if (ev->type() == ActionVisibilityChangedEvent::eventType()) {
		ActionVisibilityChangedEvent *event = static_cast<ActionVisibilityChangedEvent*>(ev);
		QAction *action = event->action();		
		JActionType type = static_cast<JActionType>(action->property("actionType").toInt());
		if (event->isVisible()) {
			switch (type) {
			case JoinLeaveAction: {
				Conference *room = qobject_cast<JMUCSession*>(event->controller());
				if (!room->isJoined()) {
					action->setText(QT_TRANSLATE_NOOP("Jabber", "Join conference"));
					action->setIcon(Icon("im-user"));
				}
				else {
					action->setText(QT_TRANSLATE_NOOP("Jabber", "Leave conference"));
					action->setIcon(Icon("im-user-offline"));
				}
				break;
			}
			case RoomConfigAction: {
				break;
			}
			case SaveRemoveBookmarkAction: {
				JMUCSession *room = qobject_cast<JMUCSession*>(event->controller());
				if (!room->bookmark().isValid())
					action->setText(QT_TRANSLATE_NOOP("Jabber", "Save to bookmarks"));
				else
					action->setText(QT_TRANSLATE_NOOP("Jabber", "Remove from bookmarks"));
				break;
			}
			case ChangeSubcriptionAction: {
				break;
			}
			case KickAction: {
				JMUCUser *user = qobject_cast<JMUCUser*>(event->controller());
				action->setVisible(user->muc()->room()->canKick(user->name()));
				break;
			}
			case BanAction: {
				JMUCUser *user = qobject_cast<JMUCUser*>(event->controller());
				action->setVisible(user->muc()->room()->canBan(user->name()));
				break;
			}
			default:
				break;
			}
		}
	} else if (ev->type() == ExtendedInfosEvent::eventType()) {
		ExtendedInfosEvent *event = static_cast<ExtendedInfosEvent*>(ev);
		QVariantHash clientInfo;
		clientInfo.insert("id", "client");
		clientInfo.insert("name", tr("Possible client"));
		clientInfo.insert("settingsDescription", tr("Show client icon"));
		event->addInfo("client", clientInfo);
		return true;
	}
	return QObject::event(ev);
}

void JProtocol::removeAccount(QObject *obj)
{
	JAccount *acc = reinterpret_cast<JAccount*>(obj);
	d_ptr->accounts->remove(d_ptr->accounts->key(acc));
}
}

#include <jprotocol.moc>
