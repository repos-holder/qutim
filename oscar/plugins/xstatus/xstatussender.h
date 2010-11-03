#ifndef XSTATUSSENDER_H
#define XSTATUSSENDER_H


#include <QTimer>
#include <icqcontact.h>

namespace qutim_sdk_0_3 {

namespace oscar {

class XStatusSender;

class XStatusSenderList : public QObject
{
	Q_OBJECT
public:
	XStatusSenderList();
	XStatusSender *getSender(IcqAccount *account);
private slots:
	void accountDestroyed(QObject *obj);
private:
	QHash<IcqAccount*, XStatusSender*> m_senders;
};

class XStatusSender : public QObject
{
	Q_OBJECT
public:
	static void sendXStatus(IcqContact *contact, quint64 cookie);
private slots:
	void sendXStatus();
private:
	friend class XStatusSenderList;
	XStatusSender();
	void sendXStatusImpl(IcqContact *contact, quint64 cookie);
	QList<QPointer<IcqContact> > m_contacts;
	QTimer m_timer;
};

} } // namespace qutim_sdk_0_3::oscar

#endif // XSTATUSSENDER_H
