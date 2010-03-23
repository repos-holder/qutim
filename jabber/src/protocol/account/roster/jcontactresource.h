#ifndef JCONTACTRESOURCE_H
#define JCONTACTRESOURCE_H

#include <qutim/buddy.h>
#include <gloox/presence.h>
#include <QSet>

namespace Jabber
{
	class JAccount;
	class JContact;
	class JContactResourcePrivate;

	class JContactResource : public qutim_sdk_0_3::Buddy
	{
		Q_PROPERTY(QSet<QString> features READ features WRITE setFeatures)
		Q_DECLARE_PRIVATE(JContactResource)
		Q_OBJECT
		public:
			JContactResource(qutim_sdk_0_3::ChatUnit *parent, const QString &name);
			JContactResource(qutim_sdk_0_3::ChatUnit *parent, JContactResourcePrivate &ptr);
			~JContactResource();
			QString id() const;
			QString title() const;
			void sendMessage(const qutim_sdk_0_3::Message &message);
			void setPriority(int priority);
			int priority();
			void setStatus(gloox::Presence::PresenceType presence, int priority);
			gloox::Presence::PresenceType status();
			virtual bool event(QEvent *ev);
			QSet<QString> features() const;
			void setFeatures(const QSet<QString> &features);
			bool checkFeature(const QLatin1String &feature) const;
			bool checkFeature(const QString &feature) const;
			bool checkFeature(const std::string &feature) const;
			ChatUnit *upperUnit();
			QString avatar() const;
		protected:
			QScopedPointer<JContactResourcePrivate> d_ptr;
	};
}

Q_DECLARE_METATYPE(QSet<QString>)

#endif // JCONTACTRESOURCE_H
