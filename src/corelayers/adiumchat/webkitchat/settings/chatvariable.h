#ifndef CHATVARIABLE_H
#define CHATVARIABLE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QFontDialog>
#include <QColorDialog>
#include <QColor>
#include <QDoubleSpinBox>
#include <QCheckBox>

namespace Core
{
namespace AdiumChat
{

struct CustomChatStyle
{
	QString selector;
	QString parameter;
	QString value;
};

class ChatVariable
{
public:
	virtual const CustomChatStyle &chatStyle() = 0;
	virtual ~ChatVariable() {}
};

}
}
Q_DECLARE_INTERFACE(Core::AdiumChat::ChatVariable, "org.qutim.core.ChatVariable")

namespace Core
{
namespace AdiumChat
{

class ChatFont : public QWidget, ChatVariable
{
	Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatVariable)
public:
	ChatFont(const CustomChatStyle &style, QWidget *parent = 0);
	const CustomChatStyle &chatStyle();
signals:
	void changeValue();
private slots:
	void changeCurrentFont();
private:
	QLabel *fontLabel;
	QToolButton *changeButton;
	CustomChatStyle m_style;
};

class ChatColor : public QToolButton, ChatVariable
{
	Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatVariable)
public:
	ChatColor(const CustomChatStyle &style, QWidget *parent = 0);
	const CustomChatStyle &chatStyle();
signals:
	void changeValue();
private slots:
	void changeCurrentColor();
private:
	QColor color;
	CustomChatStyle m_style;
};

class ChatNumeric : public QDoubleSpinBox, ChatVariable
{
	Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatVariable)
public:
	ChatNumeric(const CustomChatStyle &style,
				double min, double max, double step, QWidget *parent = 0);
	const CustomChatStyle &chatStyle();
signals:
	void changeValue();
private slots:
	void onChangeValue();
private:
	CustomChatStyle m_style;
};

class ChatBoolean : public QCheckBox, ChatVariable
{
	Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatVariable)
public:
	ChatBoolean(const CustomChatStyle &style,
				const QString &trueValue, const QString &falseValue,  QWidget *parent = 0);
	const CustomChatStyle &chatStyle();
signals:
	void changeValue();
private slots:
	void onChangeValue();
private:
	CustomChatStyle m_style;
	QString m_trueValue;
	QString m_falseValue;
};

}
}

#endif // CHATVARIABLE_H
