#ifndef OSCAR_INFOREQUEST_H
#define OSCAR_INFOREQUEST_H

#include "qutim/inforequest.h"
#include "metainfo.h"
#include <QPointer>

namespace qutim_sdk_0_3 {

namespace oscar {

class IcqContact;

class IcqInfoRequest : public InfoRequest
{
	Q_OBJECT
public:
	IcqInfoRequest(IcqAccount *account);
	IcqInfoRequest(IcqContact *contact);
	virtual ~IcqInfoRequest();
	virtual InfoItem item(const QString &name = QString()) const;
	virtual State state() const;
	static MetaInfoValuesHash itemToMetaInfoValuesHash(const InfoItem &item);
private slots:
	void onDone(bool ok);
private:
	void addItem(const MetaInfoField &field, InfoItem &group) const;
	void init();
	QPointer<FullInfoMetaRequest> m_metaReq;
	MetaInfoValuesHash m_values;
	State m_state;
	bool m_accountInfo;
	union {
		IcqAccount *m_account;
		IcqContact *m_contact;
	};
};

} } // namespace qutim_sdk_0_3::oscar

#endif // OSCAR_INFOREQUEST_H
