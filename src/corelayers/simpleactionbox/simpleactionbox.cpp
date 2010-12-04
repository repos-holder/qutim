#include "simpleactionbox.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <libqutim/debug.h>
#include <QHBoxLayout>

namespace Core
{

SimpleActionBoxModule::SimpleActionBoxModule()
{
	m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
}

void SimpleActionBoxModule::addAction(QAction *action)
{
	debug() << "SimpleActionBox Added action" << action;
	if (actions().contains(action))
		return;

	QToolButton *button = new QToolButton(this);
	button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	button->setDefaultAction(action);
	button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	button->setVisible(action->isVisible());
	m_buttons.insert(action,button);

	connect(action,SIGNAL(changed()),SLOT(onChanged()));
	connect(button,SIGNAL(destroyed(QObject*)),SLOT(onButtonDestroyed(QObject*)));
	connect(action,SIGNAL(destroyed()),button,SLOT(deleteLater()));
	if (action->softKeyRole() == QAction::NegativeSoftKey)
		m_layout->insertWidget(0,button);
	else
		m_layout->addWidget(button);
	QWidget::addAction(action);
}

void SimpleActionBoxModule::removeAction(QAction *action)
{
	m_buttons.take(action)->deleteLater();
	QWidget::removeAction(action);
}

void SimpleActionBoxModule::onButtonDestroyed(QObject *obj)
{
		QToolButton *button = reinterpret_cast<QToolButton*>(obj);
		QAction *action = m_buttons.key(button);
		m_buttons.remove(action);
}

void SimpleActionBoxModule::onChanged()
{
		QAction *action = qobject_cast<QAction*>(sender());
		Q_ASSERT(action);
		QToolButton *button = m_buttons.value(action);
		Q_ASSERT(button);
		button->setVisible(action->isVisible());
}

}
