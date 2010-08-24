#include "jsonhistory.h"
#include <qutim/chatunit.h>
#include <qutim/account.h>
#include <qutim/protocol.h>
#include <qutim/systeminfo.h>
#include <qutim/json.h>
#include <QStringBuilder>
#include "historywindow.h"
#include <qutim/icon.h>

namespace Core
{
	void init()
	{
		History *history = History::instance();
		ActionGenerator *gen = new ActionGenerator(Icon("view-history"),
											QT_TRANSLATE_NOOP("Chat", "View History"),
											history,
											SLOT(onHistoryActionTriggered(QObject*)));
		gen->setType(ActionTypeChatButton|ActionTypeContactList);
		gen->setPriority(512);
		MenuController::addAction<ChatUnit>(gen);
	}
	
	JsonHistory::JsonHistory()
	{
		static bool inited = false;
		if (!inited) {
			inited = true;
			init();
		}
	}

	JsonHistory::~JsonHistory()
	{
	}

	uint JsonHistory::findEnd(QFile &file)
	{
		int len = file.size();
		QByteArray data;
		uchar *fmap = file.map(0, file.size());
		if(!fmap)
		{
			data = file.readAll();
			fmap = (uchar *)data.constData();
		}
		uint end = file.size();
		const uchar *s = Json::skipBlanks(fmap, &len);
		uchar qch = *s;
		if(!s || (qch != '[' && qch != '{'))
		{
			if(data.isEmpty())
				file.unmap(fmap);
			return end;
		}
		qch = (qch == '{' ? '}' : ']');
		s++;
		len--;
		bool first = true;
		while(s)
		{
			s = Json::skipBlanks(s, &len);
			if(len < 2 || (s && *s == qch))
			{
				if(*(s-1) == '\n')
					s--;
				end = (uint)(s - fmap);
				break;
			}
			if(!s)
				break;
			if((!first && *s != ',') || (first && *s == ','))
				break;
			first = false;
			if(*s == ',')
			{
				s++;
				len--;
			}
			if(!(s = Json::skipRecord(s, &len)))
				break;
		}
		if(data.isEmpty())
			file.unmap(fmap);
		return end;
	}

	void JsonHistory::store(const Message &message)
	{
		if(!message.chatUnit())
			return;
		QFile file(getAccountDir(message.chatUnit()->getHistoryUnit()).filePath(getFileName(message)));
		bool new_file = !file.exists();
		if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
			return;
		if(new_file)
		{
			file.write("[\n");
		}
		else
		{
			uint end = findEnd(file);
			file.resize(end);
			file.seek(end);
			file.write(",\n");
		}
		file.write(" {\n");
		foreach(const QByteArray &name, message.dynamicPropertyNames())
		{
			QByteArray data;
			if(!Json::generate(data, message.property(name), 2))
				continue;
			file.write("  ");
			file.write(Json::quote(QString::fromUtf8(name)).toUtf8());
			file.write(": ");
			file.write(data);
			file.write(",\n");
		}
		file.write("  \"datetime\": \"");
		QDateTime time = message.time();
		if(!time.isValid())
			time = QDateTime::currentDateTime();
		file.write(time.toString(Qt::ISODate).toLatin1());
		file.write("\",\n  \"in\": ");
		file.write(message.isIncoming() ? "true" : "false");
		file.write(",\n  \"text\": ");
		file.write(Json::quote(message.text()).toUtf8());
		file.write("\n }\n]");
		file.close();
	//	It will produce something like this:
	//	{
	//	 "datetime": "2009-06-20T01:42:22",
	//	 "type": 1,
	//	 "in": true,
	//	 "text": "some cool text"
	//	}
	}

	MessageList JsonHistory::read(const ChatUnit *unit, const QDateTime &from, const QDateTime &to, int max_num)
	{
		MessageList items;
		if(!unit)
			return items;

		const ChatUnit *u = unit->getHistoryUnit();

		QDir dir = getAccountDir(u);
		QString filter = quote(u->id());
		filter += ".*.json";
		QStringList files = dir.entryList(QStringList() << filter, QDir::Readable | QDir::Files | QDir::NoDotAndDotDot,QDir::Name);
		if(files.isEmpty())
			return items;
		for(int i=files.size()-1; i>=0; i--)
		{
			QList<const uchar *> pointers;
			QFile file(dir.filePath(files[i]));
			if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
				continue;
			int len = file.size();
			QByteArray data;
			const uchar *fmap = file.map(0, file.size());
			if(!fmap)
			{
				data = file.readAll();
				fmap = (uchar *)data.constData();
			}
			const uchar *s = Json::skipBlanks(fmap, &len);
			uchar qch = *s;
			if(!s || (qch != '[' && qch != '{'))
				continue;
			qch = (qch == '{' ? '}' : ']');
			s++;
			len--;
			bool first = true;
			while(s)
			{
				s = Json::skipBlanks(s, &len);
				if(len < 2 || (s && *s == qch))
					break;
				if((!first && *s != ',') || (first && *s == ','))
					break;
				first = false;
				if(*s == ',')
				{
					s++;
					len--;
				}
				pointers.prepend(s);
				if(!(s = Json::skipRecord(s, &len)))
				{
					pointers.removeFirst();
					break;
				}
			}
			QVariant value;
			for(int i=0; i<pointers.size(); i++)
			{
				value.clear();
				s = pointers[i];
				len = file.size() + 1 - (s - fmap);
				Json::parseRecord(value, s, &len);
				QVariantMap message = value.toMap();
				Message item;
				QVariantMap::iterator it = message.begin();
				for(; it != message.end(); it++)
				{
					QString key = it.key();
					if(key == QLatin1String("datetime"))
						item.setTime(QDateTime::fromString(it.value().toString(), Qt::ISODate));
					else
						item.setProperty(key.toUtf8(), it.value());
				}
				if(item.time() >= to)
					continue;
				if(item.time() < from)
					return items;
				items.prepend(item);
				if((items.size() >= max_num) && (max_num != -1))
					return items;
			}
		}
		return items;
	}

	void JsonHistory::showHistory(const ChatUnit *unit)
	{
		unit = unit->getHistoryUnit();
		if (m_history_window) {
			m_history_window->setUnit(unit);
			m_history_window->raise();
		}
		else {
			m_history_window = new Core::HistoryWindow(unit);
			m_history_window->show();
		}
	}

	QString JsonHistory::quote(const QString &str)
	{
		const static bool true_chars[128] =
		{// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
	/* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2 */ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	/* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,
	/* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0,
	/* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0
		};
		QString result;
		result.reserve(str.size() * 2); // I hope it's enough in most cases
		const QChar *s = str.data();
		while(!s->isNull())
		{
			if(s->unicode() < 0xff && true_chars[s->unicode()])
				result += *s;
			else
			{
				result += '%';
				if(s->unicode() < 0x1000)
					result += '0';
				if(s->unicode() < 0x100)
					result += '0';
				if(s->unicode() < 0x10)
					result += '0';
				result += QString::number(s->unicode(), 16);
			}
			s++;
		}
		return result;
	}

	QString JsonHistory::unquote(const QString &str)
	{
		QString result;
		bool ok = false;
		result.reserve(str.size()); // the worst variant
		const QChar *s = str.data();
		while(!s->isNull())
		{
			if(s->unicode() == L'%')
			{
				result += QChar(QString::fromRawData(++s, 4).toUShort(&ok, 16));
				s += 3;
			}
			else
				result += *s;
			s++;
		}
		return result;
	}

	inline QString protocol(const ChatUnit *unit)
	{
		const Account *acc = unit ? unit->account() : 0;
		const Protocol *proto = acc ? acc->protocol() : 0;
		return proto ? proto->id() : QLatin1String("unknown");
	}

	QString JsonHistory::getFileName(const Message &message) const
	{
		QDateTime time = message.time().isValid() ? message.time() : QDateTime::currentDateTime();
		return quote(message.chatUnit()->getHistoryUnit()->id()) % time.toString(".yyyyMM.'json'");
	}

	QDir JsonHistory::getAccountDir(const ChatUnit *unit) const
	{
		QDir history_dir = SystemInfo::getDir(SystemInfo::HistoryDir);
		QString path = quote(protocol(unit));
		path += QLatin1Char('.');
		path += unit->account() ? quote(unit->account()->id()) : QLatin1String("unknown");
		if(!history_dir.exists(path))
			history_dir.mkpath(path);
		return history_dir.filePath(path);
	}
	
	void JsonHistory::onHistoryActionTriggered(QObject* object)
	{
		ChatUnit *unit = qobject_cast<ChatUnit*>(object);
		showHistory(unit);
	}

}
