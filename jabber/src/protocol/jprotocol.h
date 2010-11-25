#ifndef JPROTOCOL_H
#define JPROTOCOL_H

#include <qutim/protocol.h>
#include <qutim/status.h>
#include <gloox/presence.h>

namespace Jabber
{

using namespace qutim_sdk_0_3;
using namespace gloox;

struct JProtocolPrivate;
class JAccount;

class JProtocol : public Protocol
{
	Q_OBJECT
	Q_CLASSINFO("Protocol", "jabber")
	Q_CLASSINFO("Uses", "AuthorizationService")
public:
	JProtocol();
	virtual ~JProtocol();
	static inline JProtocol *instance() {
		if(!self) qWarning("JProtocol isn't created");
		return self;
	}
	virtual QList<Account *> accounts() const;
	virtual Account *account(const QString &id) const;
	void addAccount(JAccount *account, bool isEmit = false);
	static Presence::PresenceType statusToPresence(const Status &status);
	static Status presenceToStatus(Presence::PresenceType presence);
	virtual QVariant data(DataType type);
	bool event(QEvent *ev);
private slots:
	void onKickUser(QObject* obj);
	void onBanUser(QObject* obj);
	void onConvertToMuc(QObject* obj);
	void onJoinLeave(QObject *obj);
	void onShowConfigDialog(QObject *obj);
	void onSaveRemoveBookmarks(QObject *obj);
	void removeAccount(QObject *obj);
private:
	void loadActions();
	virtual void loadAccounts();
	static JProtocol *self;
	QScopedPointer<JProtocolPrivate> p;
};

} // Jabber namespace

#endif // JPROTOCOL_H
