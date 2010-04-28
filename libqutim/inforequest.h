#ifndef INFOREQUEST_H
#define INFOREQUEST_H

#include "localizedstring.h"
#include <QSharedPointer>
#include <QVariant>
#include <QEvent>

namespace qutim_sdk_0_3
{
	class InfoItemPrivate;

	class LIBQUTIM_EXPORT InfoItem
	{
	public:
		InfoItem();
		InfoItem(const QString &name, const LocalizedString &title, const QVariant &data);
		InfoItem(const LocalizedString &title, const QVariant &data = QVariant());
		InfoItem(const InfoItem &item);
		~InfoItem();
		InfoItem &operator=(const InfoItem &item);
		QString name() const;
		void setName(const QString &name);
		LocalizedStringList group() const;
		void setGroup(const QList<LocalizedString> &group);
		LocalizedString title() const;
		void setTitle(const LocalizedString &title);
		QVariant data() const;
		void setData(const QVariant &data);
		bool isNull() const;
		QList<InfoItem> subitems() const;
		void addSubitem(const InfoItem &item);
		bool hasSubitems() const;
		void setMultiple(const InfoItem &defaultSubitem, int maxCount = -1);
		bool isMultiple() const;
		int maxCount() const;
		InfoItem defaultSubitem() const;
		QVariant property(const char *name, const QVariant &def = QVariant()) const;
		template<typename T>
		T property(const char *name, const T &def = T()) const
		{ return qVariantValue<T>(property(name, qVariantFromValue<T>(def))); }
		void setProperty(const char *name, const QVariant &value);
	protected:
		QSharedDataPointer<InfoItemPrivate> d;
	};

	class LIBQUTIM_EXPORT InfoRequest : public QObject
	{
		Q_OBJECT
		Q_DISABLE_COPY(InfoRequest)
	public:
		enum State {
			Request,
			Done,
			Cancel,
			Cache
		};
		InfoRequest();
		virtual ~InfoRequest();
		virtual InfoItem item(const QString &name = QString()) const = 0;
		virtual State state() const = 0;
		virtual void resend() const;
	signals:
		void stateChanged(InfoRequest::State state);
	protected:
		virtual void virtual_hook(int id, void *data);
	};

	class LIBQUTIM_EXPORT InfoRequestCheckSupportEvent : public QEvent
	{
	public:
		enum SupportType {
			NoSupport,
			Read,
			ReadWrite
		};
		InfoRequestCheckSupportEvent(SupportType type = NoSupport);
		static QEvent::Type eventType();
		SupportType supportType() { return m_supportType; }
		void setSupportType(SupportType supportType) { m_supportType = supportType; }
	private:
		SupportType m_supportType;
	};

	class LIBQUTIM_EXPORT InfoRequestEvent : public QEvent
	{
	public:
		InfoRequestEvent();
		static QEvent::Type eventType();
		InfoRequest *request() { return m_request; }
		void setRequest(InfoRequest *request) { Q_ASSERT(!m_request); m_request = request; }
	private:
		InfoRequest *m_request;
	};

	class LIBQUTIM_EXPORT InfoItemUpdatedEvent : public QEvent
	{
	public:
		InfoItemUpdatedEvent(const InfoItem &newInfoItem);
		static QEvent::Type eventType();
		InfoItem infoItem() { return m_info; };
	private:
		InfoItem m_info;
	};
}

#endif // INFOREQUEST_H
