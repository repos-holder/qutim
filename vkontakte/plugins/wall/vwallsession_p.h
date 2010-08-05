#ifndef VWALLSESSION_P_H
#define VWALLSESSION_P_H
#include <QObject>
#include <QTimer>
#include <QVariantMap>

namespace qutim_sdk_0_3 {
	class Buddy;
	class Contact;
	class Message;
}

class VAccount;
class VContact;
class VWallSession;
class VWallSessionPrivate : public QObject
{
	Q_DECLARE_PUBLIC(VWallSession)
	Q_OBJECT
public:
    VWallSessionPrivate() : 
		QObject(0),q_ptr(0),id(QString()),me(0),owner(0),historyCount(0),timeStamp(0)
	{
		
	}
    virtual ~VWallSessionPrivate() {}
    VWallSession *q_ptr;
	QString id;
	VAccount *account();
	VContact *me;
	VContact *owner;
	int historyCount;
	QTimer updateTimer;
	int timeStamp;
	void processMultimediaMessage(qutim_sdk_0_3::Message &mess, const QVariantMap &data);
public slots:
	void getHistory();
	void onGetHistoryFinished();
};

#endif // VWALLSESSION_P_H
