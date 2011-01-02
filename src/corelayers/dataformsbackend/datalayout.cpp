#include "datalayout.h"
#include "widgetgenerator.h"
#include <QSpacerItem>
#include <QIcon>
#include <QStyle>
#include <QApplication>

namespace Core
{

DataLayout::DataLayout(DefaultDataForm *dataForm, quint8 columns, QWidget *parent) :
	QGridLayout(parent),
	AbstractDataWidget(dataForm),
	m_style(0),
	m_row(0),
	m_expandable(false)
{
#ifndef QUTIM_MOBILE_UI
	m_columns = columns != 0 ? columns : 1;
	m_currentColumn = 0;
#else
	Q_UNUSED(columns)
#endif
}

DataLayout::~DataLayout()
{
}

DataItem DataLayout::item() const
{
	DataItem items;
	items.setName(objectName());
	foreach (const WidgetLine &line, m_widgets)
		items.addSubitem(getDataItem(line.title, line.data));
	return items;
}

void DataLayout::addItem(const DataItem &item)
{
	bool twoColumns;
	QWidget *widget = getWidget(dataForm(), item, parentWidget(), &twoColumns);
	QWidget *title = 0;
	twoColumns = twoColumns || item.property("hideTitle", false);
	if (!twoColumns)
		title = getTitle(dataForm(), item, Qt::AlignVCenter | labelAlignment(), parentWidget());
	widget->setObjectName(item.name());
	addRow(title, widget, !item.isReadOnly() ?  Qt::Alignment() : Qt::AlignLeft);
	m_widgets.push_back(WidgetLine(title, widget));
	if (!item.isReadOnly()) {
		QSizePolicy::Policy policy = widget->sizePolicy().verticalPolicy();
		if (!m_expandable)
			m_expandable = policy == QSizePolicy::MinimumExpanding || policy == QSizePolicy::Expanding;
	}
}

void DataLayout::addItems(const QList<DataItem> &items)
{
	foreach (const DataItem &item, items)
		addItem(item);
}

void DataLayout::addSpacer()
{
	QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
#ifndef QUTIM_MOBILE_UI
	if (m_currentColumn != 0)
		++m_row;
#endif
	QGridLayout::addItem(spacer, m_row++, 0);
}

void DataLayout::addRow(QWidget *title, QWidget *widget, Qt::Alignment widgetAligment)
{
#ifdef QUTIM_MOBILE_UI
	if (title) {
		QVBoxLayout *layout = new QVBoxLayout;
		layout->addWidget(title, 0, labelAlignment());
		layout->addWidget(widget, 0, widgetAligment);
		addLayout(layout, m_row++, 0);
	} else {
		addWidget(widget, m_row++, 0);
	}
#else
	if (m_columns > 1) {
		if (title) {
			QHBoxLayout *layout = new QHBoxLayout;
			layout->addWidget(title, 0, labelAlignment());
			layout->addWidget(widget, 0, widgetAligment);
			addLayout(layout, m_row, m_currentColumn);
		} else {
			addWidget(widget, m_row, m_currentColumn);
		}

		if (m_columns == ++m_currentColumn) {
			m_currentColumn = 0;
			++m_row;
		}
		Q_ASSERT(m_currentColumn < m_columns);
	} else {
		if (title) {
			addWidget(title, m_row, 0, labelAlignment());
			addWidget(widget, m_row++, 1, widgetAligment);
		} else {
			addWidget(widget, m_row++, 0, 1, 2);
		}
	}
#endif
}

void DataLayout::setHorizontalSpacing(int spacing)
{
#ifdef QUTIM_MOBILE_UI
	// do nothing
#else
	if (m_columns > 1)
		QGridLayout::setHorizontalSpacing(spacing);
#endif
}

inline Qt::Alignment DataLayout::labelAlignment()
{
	if (!m_style) {
		if (QWidget *parent = parentWidget())
			m_style = parent->style();
		else
			m_style = QApplication::style();
		m_labelAlignment = Qt::Alignment(m_style->styleHint(QStyle::SH_FormLayoutLabelAlignment));
	}
	return m_labelAlignment;
}

}
