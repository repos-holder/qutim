#ifndef JMUCUSER_H
#define JMUCUSER_H

#include "../roster/jcontactresource.h"

namespace Jabber
{
	class JAccount;
	class JMUCSession;
	class JMUCUserPrivate;

	class JMUCUser : public JContactResource
	{
		Q_OBJECT
		Q_DECLARE_PRIVATE(JMUCUser)
	public:
		JMUCUser(JMUCSession *muc, const QString &name);
		~JMUCUser();
		QString title() const;

	signals:

	public slots:

	};
}

#endif // JMUCUSER_H
