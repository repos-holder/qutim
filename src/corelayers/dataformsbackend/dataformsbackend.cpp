#include "dataformsbackend.h"
#include "modifiablewidget.h"
#include "datalayout.h"
#include "widgetgenerator.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QKeyEvent>

#include <libqutim/icon.h>

Q_DECLARE_METATYPE(QList<QIcon>);

namespace Core
{

DefaultDataForm::DefaultDataForm(const DataItem &item, StandardButtons standartButtons,  const Buttons &buttons) :
	m_widget(0)
{
	DataLayout *dataLayout = 0;
	QVBoxLayout *layout = 0;
	setFrameStyle(NoFrame);
	setObjectName(item.name());
	setWindowTitle(item.title());
	if (item.isAllowedModifySubitems()) {
		layout = new QVBoxLayout(this);
		ModifiableWidget *w = new ModifiableWidget(item);
		m_widget = w;
		layout->addWidget(w);
		if (!w->isExpandable()) {
			QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			layout->addItem(spacer);
		}
	} else {
		dataLayout = new DataLayout(!item.isReadOnly(), this);
		m_widget = dataLayout;
		if (item.hasSubitems())
			dataLayout->addItems(item.subitems());
		else
			dataLayout->addItem(item);
		if (!dataLayout->isExpandable())
			dataLayout->addSpacer();
	}
	if (standartButtons != NoButton || !buttons.isEmpty()) {
		QDialogButtonBox *buttonsBox = new QDialogButtonBox(
				static_cast<QDialogButtonBox::StandardButton>(static_cast<int>(standartButtons)), Qt::Horizontal, this);
		foreach (const Button &button, buttons) {
			QPushButton *btn = buttonsBox->addButton(button.name, static_cast<QDialogButtonBox::ButtonRole>(button.role));
			btn->setObjectName(button.name.original());
		}
		connect(buttonsBox, SIGNAL(accepted()), SLOT(accept()));
		connect(buttonsBox, SIGNAL(rejected()), SLOT(reject()));
		connect(buttonsBox, SIGNAL(helpRequested()), SIGNAL(helpRequested()));
		connect(buttonsBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButtonClicked(QAbstractButton*)));
		connect(this, SIGNAL(accepted()), SLOT(close()));
		connect(this, SIGNAL(rejected()), SLOT(close()));
		if (dataLayout)
			dataLayout->addRow(buttonsBox);
		else
			layout->addWidget(buttonsBox);
	}
}

DataItem DefaultDataForm::item() const
{
	DataItem item;
	if (m_widget)
		item = m_widget->item();
	item.setName(objectName());
	item.setTitle(windowTitle());
	return item;
}

void DefaultDataForm::onButtonClicked(QAbstractButton *button)
{
	emit clicked(button->objectName());
}

void DefaultDataForm::keyPressEvent(QKeyEvent *e)
{
#ifdef Q_WS_MAC
	if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period) {
		e->accept();
		reject();
	} else
#endif
	if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return: {
			foreach (QPushButton *btn, qFindChildren<QPushButton*>(this)) {
				if (btn->isDefault() && btn->isVisible()) {
					if (btn->isEnabled())
						btn->click();
					e->accept();
					break;
				}
			}
			return;
		}
		case Qt::Key_Escape:
			e->accept();
			reject();
			break;
		}
	}
	AbstractDataForm::keyPressEvent(e);
}

DefaultDataFormsBackend::DefaultDataFormsBackend()
{
}

QWidget *DefaultDataFormsBackend::get(const DataItem &item, AbstractDataForm::StandardButtons standartButtons, const AbstractDataForm::Buttons &buttons)
{
	if (item.isNull())
		return 0;
	if (!item.hasSubitems() && !item.isAllowedModifySubitems() &&
		standartButtons == AbstractDataForm::NoButton && buttons.isEmpty())
	{
		return getWidget(item);
	} else {
		return new DefaultDataForm(item, standartButtons, buttons);
	}
}

}
