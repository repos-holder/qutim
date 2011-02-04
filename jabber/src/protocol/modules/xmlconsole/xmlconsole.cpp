#include "xmlconsole.h"
#include "ui_xmlconsole.h"
#include <qutim/account.h>
#include <qutim/icon.h>
#include <qutim/debug.h>
#include <QMenu>
#include <QActionGroup>
#include <QStringBuilder>
#include <QDebug>
#include <QTextLayout>
#include <QPlainTextDocumentLayout>
#include <qutim/systemintegration.h>
//#include <QElapsedTimer>

using namespace jreen;
using namespace qutim_sdk_0_3;

namespace Jabber
{
XmlConsole::XmlConsole(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::XmlConsole),
	m_client(0),
	//	    m_bracketsColor(0),
	m_filter(0x1f)
{
	m_ui->setupUi(this);
	qDebug("%s", Q_FUNC_INFO);
	//		m_incoming.depth = 0;
	//		m_outgoing.depth = 0;
	QPalette pal = palette();
	pal.setColor(QPalette::Base, Qt::black);
	pal.setColor(QPalette::Text, Qt::white);
	m_ui->xmlBrowser->viewport()->setPalette(pal);
	QTextDocument *doc = m_ui->xmlBrowser->document();
	doc->setDocumentLayout(new QPlainTextDocumentLayout(doc));
	doc->clear();
	//		doc->rootFrame()->set
	//		doc->setHtml(QLatin1String("<body bgcolor='#000000'></body>"));
	//		QTextCursor cur(doc);
	//		cur.movePosition(QTextCursor::Start);
	QTextFrameFormat format = doc->rootFrame()->frameFormat();
	format.setBackground(QColor(Qt::black));
	format.setMargin(0);
	doc->rootFrame()->setFrameFormat(format);
	QMenu *menu = new QMenu(m_ui->filterButton);
	menu->setSeparatorsCollapsible(false);
	menu->addSeparator()->setText(tr("Filter"));
	QActionGroup *group = new QActionGroup(menu);
	QAction *disabled = group->addAction(menu->addAction(tr("Disabled")));
	disabled->setCheckable(true);
	disabled->setData(Disabled);
	QAction *jid = group->addAction(menu->addAction(tr("By JID")));
	jid->setCheckable(true);
	jid->setData(ByJid);
	QAction *xmlns = group->addAction(menu->addAction(tr("By namespace uri")));
	xmlns->setCheckable(true);
	xmlns->setData(ByXmlns);
	QAction *attrb = group->addAction(menu->addAction(tr("By all attributes")));
	attrb->setCheckable(true);
	attrb->setData(ByAllAttributes);
	disabled->setChecked(true);
	connect(group, SIGNAL(triggered(QAction*)), this, SLOT(onActionGroupTriggered(QAction*)));
	menu->addSeparator()->setText(tr("Visible stanzas"));
	group = new QActionGroup(menu);
	group->setExclusive(false);
	QAction *iq = group->addAction(menu->addAction(tr("Information query")));
	iq->setCheckable(true);
	iq->setData(XmlNode::Iq);
	iq->setChecked(true);
	QAction *message = group->addAction(menu->addAction(tr("Message")));
	message->setCheckable(true);
	message->setData(XmlNode::Message);
	message->setChecked(true);
	QAction *presence = group->addAction(menu->addAction(tr("Presence")));
	presence->setCheckable(true);
	presence->setData(XmlNode::Presence);
	presence->setChecked(true);
	QAction *custom = group->addAction(menu->addAction(tr("Custom")));
	custom->setCheckable(true);
	custom->setData(XmlNode::Custom);
	custom->setChecked(true);
	connect(group, SIGNAL(triggered(QAction*)), this, SLOT(onActionGroupTriggered(QAction*)));
	m_ui->filterButton->setMenu(menu);
	//		m_bracketsColor = QLatin1String("#666666");
	//		m_incoming.bodyColor = QLatin1String("#bb66bb");
	//		m_incoming.tagColor = QLatin1String("#006666");
	//		m_incoming.attributeColor = QLatin1String("#009933");
	//		m_incoming.paramColor = QLatin1String("#cc0000");
	//		m_outgoing.bodyColor = QLatin1String("#999999");
	//		m_outgoing.tagColor = QLatin1String("#22aa22");
	//		m_outgoing.attributeColor = QLatin1String("#ffff33");
	//		m_outgoing.paramColor = QLatin1String("#dd8811");
	//		doc->setDocumentMargin(0);
	m_stackBracketsColor = QColor(0x666666);
	m_stackIncoming.bodyColor = QColor(0xbb66bb);
	m_stackIncoming.tagColor = QColor(0x006666);
	m_stackIncoming.attributeColor = QColor(0x009933);
	m_stackIncoming.paramColor = QColor(0xcc0000);
	m_stackOutgoing.bodyColor = QColor(0x999999);
	m_stackOutgoing.tagColor = QColor(0x22aa22);
	m_stackOutgoing.attributeColor = QColor(0xffff33);
	m_stackOutgoing.paramColor = QColor(0xdd8811);

	QAction *action = new QAction(tr("Close"),this);
	action->setSoftKeyRole(QAction::NegativeSoftKey);
	connect(action, SIGNAL(triggered()), SLOT(close()));
	addAction(action);
}

XmlConsole::~XmlConsole()
{
	delete m_ui;
}

void XmlConsole::init(Account *account, const JabberParams &params)
{
	qDebug("%s", Q_FUNC_INFO);
	m_client = params.item<Client>();
	account->addAction(new ActionGenerator(Icon("utilities-terminal"),
										   QT_TRANSLATE_NOOP("Jabber", "Xml console"),
										   this, SLOT(show())),
					   "Additional");
	m_client->addXmlStreamHandler(this);
}

void XmlConsole::show()
{
	SystemIntegration::show(this);
}

void XmlConsole::handleStreamBegin()
{
	//		m_incoming.reader.clear();
	//		m_outgoing.reader.clear();
	//		m_incoming.depth = 0;
	//		m_outgoing.depth = 0;
	//		m_incoming.xmlns.clear();
	//		m_outgoing.xmlns.clear();

	m_stackIncoming.reader.clear();
	m_stackOutgoing.reader.clear();
	m_stackIncoming.depth = 0;
	m_stackOutgoing.depth = 0;
	qDeleteAll(m_stackIncoming.tokens);
	qDeleteAll(m_stackOutgoing.tokens);
	m_stackIncoming.tokens.clear();
	m_stackOutgoing.tokens.clear();
}

void XmlConsole::handleStreamEnd()
{
	//		m_incoming.reader.clear();
	//		m_outgoing.reader.clear();
	//		m_incoming.depth = 0;
	//		m_outgoing.depth = 0;

	m_stackIncoming.reader.clear();
	m_stackOutgoing.reader.clear();
	m_stackIncoming.depth = 0;
	m_stackOutgoing.depth = 0;
	qDeleteAll(m_stackIncoming.tokens);
	qDeleteAll(m_stackOutgoing.tokens);
	m_stackIncoming.tokens.clear();
	m_stackOutgoing.tokens.clear();
}

void XmlConsole::handleIncomingData(const char *data, qint64 size)
{
	stackProcess(QByteArray::fromRawData(data, size), true);
	//		process(QByteArray::fromRawData(data, size), true);
}

void XmlConsole::handleOutgoingData(const char *data, qint64 size)
{
	stackProcess(QByteArray::fromRawData(data, size), false);
	//		process(QByteArray::fromRawData(data, size), false);
}

//	QString generate_space(int depth)
//	{
//		depth *= 2;
//		QString space;
//		while (depth--)
//			space += QLatin1String("&nbsp;");
//		return space;
//	}

QString generate_stacked_space(int depth)
{
	return QString(depth * 2, QLatin1Char(' '));
}

//	void XmlConsole::Environment::appendText(const QString &text, const QLatin1String &color)
//	{
//		html.append(QLatin1String("<font color="));
//		html.append(color);
//		html.append(QLatin1String(">"));
//		html.append(text);
//		html.append(QLatin1String("</font>"));
//	}

//	void XmlConsole::Environment::appendAttribute(const QString &name, const QStringRef &value)
//	{
//		html.append(QLatin1String(" <font color='"));
//		html.append(attributeColor);
//		html.append(QLatin1String("'>"));
//		html.append(Qt::escape(name));
//		html.append(QLatin1String("</font>=<font color='"));
//		html.append(paramColor);
//		html.append(QLatin1String("'>'"));
//		html.append(Qt::escape(value.toString()));
//		html.append(QLatin1String("'</font>"));
//	}

//	struct MethodLiveCounter
//	{
//		MethodLiveCounter(const char *m, qint64 &t) : method(m), time(t) { timer.start(); }
//		~MethodLiveCounter() { time += timer.elapsed(); debug() << method << "worked" << time << "ms"; }
//		QElapsedTimer timer;
//		const char *method;
//		qint64 &time;
//	};

void XmlConsole::stackProcess(const QByteArray &data, bool incoming)
{
	//		static qint64 t = 0;
	//		MethodLiveCounter counter(Q_FUNC_INFO, t);
	StackEnvironment *d = &(incoming ? m_stackIncoming : m_stackOutgoing);
	d->reader.addData(data);
	StackToken *token;
	while (d->reader.readNext() > QXmlStreamReader::Invalid) {
		//			debug() << d->depth << d->reader.tokenString();
		switch(d->reader.tokenType()) {
		case QXmlStreamReader::StartElement:
			d->depth++;
			if (d->depth > 1) {
				if (!d->tokens.isEmpty() && d->tokens.last()->type == QXmlStreamReader::Characters)
					delete d->tokens.takeLast();
				d->tokens << new StackToken(d->reader);
			}
			break;
		case QXmlStreamReader::EndElement:
			if (d->tokens.isEmpty())
				break;
			token = d->tokens.last();
			if (token->type == QXmlStreamReader::StartElement && !token->startTag.empty)
				token->startTag.empty = true;
			else if (d->depth > 1)
				d->tokens << new StackToken(d->reader);
			//				debug() << __LINE__ << d->depth << d->tokens.size();
			if (d->depth == 2) {
				QTextCursor cursor(m_ui->xmlBrowser->document());
				cursor.movePosition(QTextCursor::End);
				cursor.beginEditBlock();
				QTextCharFormat zeroFormat = cursor.charFormat();
				zeroFormat.setForeground(QColor(Qt::white));
				QTextCharFormat bodyFormat = zeroFormat;
				bodyFormat.setForeground(d->bodyColor);
				QTextCharFormat tagFormat = zeroFormat;
				tagFormat.setForeground(d->tagColor);
				QTextCharFormat attributeFormat = zeroFormat;
				attributeFormat.setForeground(d->attributeColor);
				QTextCharFormat paramsFormat = zeroFormat;
				paramsFormat.setForeground(d->paramColor);
				QTextCharFormat bracketFormat = zeroFormat;
				bracketFormat.setForeground(m_stackBracketsColor);
				QString singleSpace = QLatin1String(" ");
				cursor.insertBlock();
				int depth = 0;
				QString currentXmlns;
				QXmlStreamReader::TokenType lastType = QXmlStreamReader::StartElement;
				for (int i = 0; i < d->tokens.size(); i++) {
					token = d->tokens.at(i);
					if (token->type == QXmlStreamReader::StartElement) {
						QString space = generate_stacked_space(depth);
						cursor.insertText(QLatin1String("\n"));
						cursor.insertText(space);
						cursor.insertText(QLatin1String("<"), bracketFormat);
						cursor.insertText(token->startTag.name->toString(), tagFormat);
						const QStringRef &xmlns = *token->startTag.xmlns;
						if (i == 0 || xmlns != currentXmlns) {
							currentXmlns = xmlns.toString();
							cursor.insertText(singleSpace);
							cursor.insertText(QLatin1String("xmlns"), attributeFormat);
							cursor.insertText(QLatin1String("="), zeroFormat);
							cursor.insertText(QLatin1String("'"), paramsFormat);
							cursor.insertText(currentXmlns, paramsFormat);
							cursor.insertText(QLatin1String("'"), paramsFormat);
						}
						QXmlStreamAttributes *attributes = token->startTag.attributes;
						for (int j = 0; j < attributes->count(); j++) {
							const QXmlStreamAttribute &attr = attributes->at(j);
							cursor.insertText(singleSpace);
							cursor.insertText(attr.name().toString(), attributeFormat);
							cursor.insertText(QLatin1String("="), zeroFormat);
							cursor.insertText(QLatin1String("'"), paramsFormat);
							cursor.insertText(attr.value().toString(), paramsFormat);
							cursor.insertText(QLatin1String("'"), paramsFormat);
						}
						if (token->startTag.empty) {
							cursor.insertText(QLatin1String("/>"), bracketFormat);
						} else {
							cursor.insertText(QLatin1String(">"), bracketFormat);
							depth++;
						}
					} else if (token->type == QXmlStreamReader::EndElement) {
						if (lastType == QXmlStreamReader::EndElement) {
							QString space = generate_stacked_space(depth - 1);
							cursor.insertText(QLatin1String("\n"));
							cursor.insertText(space);
						}
						cursor.insertText(QLatin1String("</"), bracketFormat);
						cursor.insertText(token->endTag.name->toString(), tagFormat);
						cursor.insertText(QLatin1String(">"), bracketFormat);
						depth--;
					} else if (token->type == QXmlStreamReader::Characters) {
						cursor.setCharFormat(bodyFormat);
						QString text = token->charachters.text->toString();
						if (text.contains(QLatin1Char('\n'))) {
							QString space = generate_stacked_space(depth);
							space.prepend(QLatin1Char('\n'));
							QStringList lines = text.split(QLatin1Char('\n'));
							for (int j = 0; j < lines.size(); j++) {
								cursor.insertText(space);
								cursor.insertText(lines.at(j));
							}
							space.chop(1);
							cursor.insertText(space);
						} else {
							cursor.insertText(text);
						}
					}
					lastType = token->type;
					if (lastType == QXmlStreamReader::StartElement && token->startTag.empty)
						lastType = QXmlStreamReader::EndElement;
					delete token;
				}
				cursor.endEditBlock();
				d->tokens.clear();
			}
			d->depth--;
			break;
		case QXmlStreamReader::Characters:
			token = d->tokens.isEmpty() ? 0 : d->tokens.last();
			if (token && token->type == QXmlStreamReader::StartElement && !token->startTag.empty)
				d->tokens << new StackToken(d->reader);
			break;
		default:
			break;
		}
	}
	if (!incoming && d->depth > 1) {
		qFatal("outgoing depth %d on\n\"%s\"", d->depth,
			   qPrintable(QString::fromUtf8(data, data.size())));
	}
}

//	void XmlConsole::process(const QByteArray &data, bool incoming)
//	{
//		static qint64 t = 0;
//		MethodLiveCounter counter(Q_FUNC_INFO, t);
//		Environment *d = &(incoming ? m_incoming : m_outgoing);
//		d->reader.addData(data);
//		while (d->reader.readNext() > QXmlStreamReader::Invalid) {
//			switch(d->reader.tokenType()) {
//			case QXmlStreamReader::StartElement:
//				if (d->last == QXmlStreamReader::StartElement)
//					d->appendText(QLatin1String("&gt;"), m_bracketsColor);
//				if (d->depth == 1) {
//					d->html.resize(0);
//					d->current.type = XmlNode::Custom;
//					if (d->reader.name() == QLatin1String("iq"))
//						d->current.type = XmlNode::Iq;
//					else if (d->reader.name() == QLatin1String("presence"))
//						d->current.type = XmlNode::Presence;
//					else if (d->reader.name() == QLatin1String("message"))
//						d->current.type = XmlNode::Message;
//					d->current.incoming = incoming;
//					d->current.time = QDateTime::currentDateTime();
//					d->current.xmlns.clear();
//					d->current.xmlns << d->reader.namespaceUri().toString();
//					d->current.attributes.clear();
//					foreach (const QXmlStreamAttribute &attrb, d->reader.attributes())
//						d->current.attributes << attrb.value().toString();
//					QLatin1String jid = QLatin1String(incoming ? "from" : "to");
//					d->current.jid = d->reader.attributes().value(jid).toString();
//				}
//				if (!d->html.isEmpty())
//					d->html.append(QLatin1String("<br/>"));
//				d->html.append(generate_space(d->depth));
//				d->appendText(QLatin1String("&lt;"), m_bracketsColor);
//				d->appendText(Qt::escape(d->reader.name().toString()), d->tagColor);
//				if (d->xmlns != d->reader.namespaceUri() || d->depth == 1) {
//					d->xmlns = d->reader.namespaceUri().toString();
//					d->appendAttribute(QLatin1String("xmlns"), d->reader.namespaceUri());
//				}
//				foreach (const QXmlStreamAttribute &attr, d->reader.attributes())
//					d->appendAttribute(attr.name().toString(), attr.value());
//				d->depth++;
//				break;
//			case QXmlStreamReader::EndElement:
//				d->depth--;
//				if (d->last == QXmlStreamReader::StartElement) {
//					// &#47; is equal for '/'
//					d->appendText(QLatin1String("&#47;&gt;"), m_bracketsColor);
//				} else {
//					if (d->last != QXmlStreamReader::Characters) {
//						d->html.append("<br/>");
//						d->html.append(generate_space(d->depth));
//					}
//					d->appendText(QLatin1String("&lt;&#47;"), m_bracketsColor);
//					d->appendText(Qt::escape(d->reader.name().toString()), d->tagColor);
//					d->appendText(QLatin1String("&gt;"), m_bracketsColor);
//				}
//				if (d->depth == 1) {
//					QTextDocument *doc = m_ui->xmlBrowser->document();
//					QTextCursor cur(doc);
//					cur.beginEditBlock();
//					cur.movePosition(QTextCursor::End);
////					if (!m_nodes.isEmpty())
//						cur.insertBlock();

//					d->current.block = cur.block();
//					d->current.lineCount = cur.block().lineCount();
//					d->html.append(QLatin1String("<br/><br/>"));
//					cur.insertHtml(d->html);
//					cur.endEditBlock();

//					m_nodes << d->current;
//				}
//				break;
//			case QXmlStreamReader::Characters: {
//				if (d->last == QXmlStreamReader::EndElement)
//					break;
//				if (d->last == QXmlStreamReader::StartElement)
//					d->appendText(QLatin1String("&gt;"), m_bracketsColor);
//				QString text = Qt::escape(d->reader.text().toString());
//				if (text.contains(QLatin1Char('\n'))) {
//					QString space = generate_space(d->depth);
//					space.prepend("<br/>");
//					d->html.append(space);
//					text.replace(QLatin1String("\n"), space);
//					text.append("<br/>");
//					text.append(generate_space(d->depth - 1));
//				}
//				d->appendText(text, d->bodyColor);
//				break; }
//			default:
//				break;
//			}
//			if (d->last != QXmlStreamReader::EndElement
//			        || d->reader.tokenType() != QXmlStreamReader::Characters) {
//				d->last = d->reader.tokenType();
//			}
//		}
//		if (!incoming && d->depth > 1) {
//			qFatal("outgoing depth %d on\n\"%s\"", d->depth,
//				   qPrintable(QString::fromUtf8(data, data.size())));
//		}
//	}

void XmlConsole::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
}

void Jabber::XmlConsole::onActionGroupTriggered(QAction *action)
{
	int type = action->data().toInt();
	if (type >= 0x10) {
		m_filter = (m_filter & 0xf) | type;
		m_ui->lineEdit->setEnabled(type != 0x10);
	} else {
		m_filter = m_filter ^ type;
	}
	on_lineEdit_textChanged(m_ui->lineEdit->text());
}

void Jabber::XmlConsole::on_lineEdit_textChanged(const QString &text)
{
	qDebug() << Q_FUNC_INFO;
	int filterType = m_filter & 0xf0;
	JID filterJid = (filterType == ByJid) ? text : QString();
	qDebug() << ("0x" + QString::number(filterType, 16));
    for (int i = 0; i < m_nodes.size(); i++) {
		XmlNode &node = m_nodes[i];
		bool ok = true;
		switch (filterType) {
		case ByXmlns:
			ok = node.xmlns.contains(text);
			break;
		case ByJid:
			ok = node.jid.full() == filterJid.full() || node.jid.bare() == filterJid.full();
			break;
		case ByAllAttributes:
			ok = node.attributes.contains(text);
			break;
		default:
			break;
		}
		ok &= bool(node.type & m_filter);
		node.block.setVisible(ok);
		node.block.setLineCount(ok ? node.lineCount : 0);
		//		qDebug() << node.block.lineCount();
	}
	QAbstractTextDocumentLayout *layout = m_ui->xmlBrowser->document()->documentLayout();
	Q_ASSERT(qobject_cast<QPlainTextDocumentLayout*>(layout));
	qobject_cast<QPlainTextDocumentLayout*>(layout)->requestUpdate();
}
