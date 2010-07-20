#ifndef JMUCMANAGER_H
#define JMUCMANAGER_H

#include <QObject>
#include <qutim/chatunit.h>
#include <gloox/presence.h>

namespace qutim_sdk_0_3 
{
	class Conference;
}

namespace Jabber
{
	using namespace qutim_sdk_0_3;
	using namespace gloox;

	class JAccount;
	class JBookmarkManager;
	struct JMUCManagerPrivate;
	class JMUCSession;

	class JMUCManager : public QObject
	{
		Q_OBJECT
		public:
			JMUCManager(JAccount *account, QObject *parent = 0);
			~JMUCManager();
			ChatUnit *muc(const QString &jid);
			JBookmarkManager *bookmarkManager();
			void openJoinWindow(const QString &conference, const QString &nick, const QString &password,
					const QString &name = "");
			void openJoinWindow();
			void syncBookmarks();
			void join(const QString &conference, const QString &nick = QString(), const QString &password = QString());
			void setPresenceToRooms(Presence::PresenceType presence);
			void leave(const QString &room);
			bool event(QEvent *event);
			void appendMUCSession(JMUCSession *room);
		signals:
			void conferenceCreated(qutim_sdk_0_3::Conference*);
		private slots:
			void bookmarksChanged();
			void join(QObject*);
			void closeMUCSession();
			void saveToBookmarks(QObject*);
			void removeFromBookmarks(QObject*);
		protected:
			void createActions(JMUCSession *room);
			void closeMUCSession(JMUCSession *room);
		private:
			QScopedPointer<JMUCManagerPrivate> p;
	};
}

#endif // JMUCMANAGER_H
