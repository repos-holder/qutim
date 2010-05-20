#include "editabledatalayout.h"
#include "readonlydatalayout.h"
#include "libqutim/icon.h"
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include "editablewidgets.h"
#include "libqutim/debug.h"

namespace Core
{

static ComboBox *getComboBox(const QString &value, const LocalizedStringList &alt)
{
	ComboBox *d = new ComboBox();
	int current = -1;
	int i = 0;
	d->addItem(notSpecifiedStr);
	foreach (const LocalizedString &str, alt) {
		if (value == str)
			current = i;
		d->addItem(str);
		++i;
	}
	d->setCurrentIndex(current + 1);
	return d;
}

static QWidget *getTitle(const DataItem &item)
{
	LocalizedStringList alt = item.property("titleAlternatives", LocalizedStringList());
	if (item.property("readOnly", false) || alt.isEmpty()) {
		QLabel *title = new QLabel(item.title() + ":");
		title->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
		QFont font;
		font.setBold(true);
		title->setFont(font);
		return title;
	} else {
		return getComboBox(item.title(), alt);
	}
}

static QWidget *getWidgetHelper(const DataItem &item, bool *twoColumn, QSizePolicy::Policy &verticalPolicy)
{
	verticalPolicy = QSizePolicy::Maximum;
	if (twoColumn)
		*twoColumn = false;
	QVariant::Type type = item.data().type();
	if (item.property("readOnly", false)) {
		return ReadOnlyDataLayout::getReadOnlyWidget(item, twoColumn);
	} else if (type == QVariant::StringList) {
		return new StringListGroup(item);
	} else if (item.isMultiple()) {
		if (twoColumn)
			*twoColumn = true;
		return new DataListGroup(item);
	} else if (item.hasSubitems()) {
		if (twoColumn)
			*twoColumn = true;
		return new DataGroup(item);
	} else if (type == QVariant::Bool) {
		CheckBox *d = new CheckBox();
		d->setText(item.title());
		d->setChecked(item.data().toBool());
		if (twoColumn)
			*twoColumn = true;
		return d;
	} else if (type == QVariant::Date) {
		DateEdit *d = new DateEdit();
		d->setDate(item.data().toDate());
		return d;
	} else if (type == QVariant::DateTime) {
		DateTimeEdit *d = new DateTimeEdit();
		d->setDateTime(item.data().toDateTime());
		return d;
	} else if (type == QVariant::Int || type == QVariant::LongLong || type == QVariant::UInt) {
		SpinBox *d = new SpinBox();
		d->setValue(item.data().toInt());
		return d;
	} else if (type == QVariant::Double) {
		DoubleSpinBox *d = new DoubleSpinBox();
		d->setValue(item.data().toDouble());
		return d;
	} else if (type == QVariant::Date) {
		DateEdit *d = new DateEdit();
		d->setDate(item.data().toDate());
		return d;
	} else if (type == QVariant::DateTime) {
		DateTimeEdit *d = new DateTimeEdit();
		d->setDateTime(item.data().toDateTime());
		return d;
	}
	QString str;
	if (item.data().canConvert<LocalizedString>())
		str = item.data().value<LocalizedString>();
	else
		str = item.data().toString();
	if (item.property("readOnly", false)) {
		return new QLabel(str);
	} else {
		LocalizedStringList alt = item.property("alternatives", LocalizedStringList());
		if (!alt.isEmpty()) {
			return getComboBox(str, alt);
		} if (!item.property("multiline", false)) {
			LineEdit *d = new LineEdit();
			d->setText(str);
			return d;
		} else {
			verticalPolicy = QSizePolicy::MinimumExpanding;
			TextEdit *d = new TextEdit();
			d->setText(str);
			return d;
		}
	}
}

static QWidget *getWidget(const DataItem &item, bool *twoColumn = 0)
{
	QSizePolicy::Policy vertPolicy;
	QWidget *widget = getWidgetHelper(item, twoColumn, vertPolicy);
	widget->setSizePolicy(QSizePolicy::Expanding, vertPolicy);
	return widget;
}

static QString getTitle(QWidget *title)
{
	QLabel *label = qobject_cast<QLabel*>(title);
	if (label)
		return label->text();
	QComboBox *box = qobject_cast<QComboBox*>(title);
	if (box) {
		QString text = box->currentText();
		if (!text.isEmpty() && text != notSpecifiedStr)
			return text;
	}
	return QString();
}

static DataItem getDataItem(QWidget *title, QWidget *data)
{
	QString titleStr = getTitle(title);
	AbstractDataWidget *dataGroup = qobject_cast<AbstractDataWidget*>(data);
	if (dataGroup) {
		DataItem item = dataGroup->item();
		item.setTitle(titleStr);
		return item;
	}
	return DataItem(data->objectName(), titleStr, QVariant());
}

DataListWidget::DataListWidget(QWidget *parent) :
	QWidget(parent)
{
	init();
}

DataListWidget::DataListWidget(const DataItem &def, QWidget *parent) :
	QWidget(parent), m_def(def)
{
	init();
}

void DataListWidget::init()
{
	m_max = -1;
	m_layout = new QGridLayout(this);
	m_addButton = new QPushButton(this);
	m_addButton->setIcon(Icon("list-add"));
	connect(m_addButton, SIGNAL(clicked()), SLOT(onAddRow()));
	m_layout->addWidget(m_addButton, 0, 1, 1, 1);
}

DataListWidget::~DataListWidget()
{
}

void DataListWidget::addRow(QWidget *data, QWidget *title)
{
	data->setParent(this);
	if (title)
		title->setParent(this);
	QPushButton *deleteButton = new QPushButton(this);
	deleteButton->setIcon(Icon("list-remove"));
	connect(deleteButton, SIGNAL(clicked()), SLOT(onRemoveRow()));
	deleteButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	int row = m_widgets.size();
	WidgetLine line(deleteButton, data, title);
	m_widgets.push_back(line);
	m_layout->removeWidget(m_addButton);
	setRow(line, row);
	m_layout->addWidget(m_addButton, ++row, 2, 1, 1);
	m_addButton->setVisible(m_max < 0 || m_max > row);
}

void DataListWidget::addRow(const DataItem &item)
{
	QWidget *title = 0;
	bool twoColumn;
	QWidget *data = getWidget(item, &twoColumn);
	data->setObjectName(item.name());
	if (!twoColumn && !item.property("hideTitle", false))
		title = getTitle(item);
	addRow(data, title);
}

DataItem DataListWidget::item() const
{
	DataItem items;
	items.setName(objectName());
	items.setMultiple(m_def, m_max);
	foreach (const WidgetLine &line, m_widgets)
		items.addSubitem(getDataItem(line.title, line.data));
	return items;
}

void DataListWidget::onAddRow()
{
	addRow(m_def);
}

void DataListWidget::onRemoveRow()
{
	QWidget *deleteButton = reinterpret_cast<QWidget*>(sender());
	Q_ASSERT(deleteButton);
	WidgetList::iterator itr = m_widgets.begin();
	int row = 0;
	while (itr->deleteButton != deleteButton) {
		Q_ASSERT(itr != m_widgets.end());
		++itr;
		++row;
	}
	deleteButton->deleteLater();
	itr->data->deleteLater();
	if (itr->title)
		itr->title->deleteLater();
	itr = m_widgets.erase(itr);
	while (itr != m_widgets.end()) {
		m_layout->removeWidget(itr->deleteButton);
		m_layout->removeWidget(itr->data);
		if (itr->title)
			m_layout->removeWidget(itr->title);
		setRow(*itr, row++);
		itr++;
	}
	if (m_max < 0 || m_max > row)
		m_addButton->setVisible(true);
}

void DataListWidget::setRow(const WidgetLine &line, int row)
{
	if (line.title) {
		m_layout->addWidget(line.title, row, 0);
		m_layout->addWidget(line.data, row, 1);
	} else {
		m_layout->addWidget(line.data, row, 0, 1, 2);
	}
	m_layout->addWidget(line.deleteButton, row, 2);
}

DataListGroup::DataListGroup(const DataItem &item, QWidget *parent) :
	QGroupBox(parent)
{
	setObjectName(item.name());
	setTitle(item.title());
	setFlat(true);
	QVBoxLayout *layout = new QVBoxLayout(this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_widget = new DataListWidget(item.defaultSubitem());
	m_widget->setMaxItemsCount(item.maxCount());
	foreach (const DataItem &subitem, item.subitems())
		m_widget->addRow(subitem);
	layout->addWidget(m_widget);
}

DataItem DataListGroup::item() const
{
	DataItem item = m_widget->item();
	item.setName(objectName());
	return item;
}

DataGroup::DataGroup(const DataItem &items, QWidget *parent) :
	QGroupBox(parent)
{
	setTitle(items.title());
	setFlat(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_layout = new EditableDataLayout(this);
	m_layout->addItems(items.subitems());
}

DataItem DataGroup::item() const
{
	DataItem item = m_layout->item();
	item.setName(objectName());
	return item;
}

StringListGroup::StringListGroup(const DataItem &item, QWidget *parent) :
	DataListWidget(parent)
{
	m_max = item.maxCount() == 1 ? -1 : item.maxCount();
	m_def = item;
	m_def.setData(QVariant(QVariant::String));
	m_def.setProperty("hideTitle", true);
	m_def.setMultiple(DataItem(), 1);
	LocalizedStringList alt = item.property("alternatives", LocalizedStringList());
	foreach (const QString &str, item.data().toStringList()) {
		if (!alt.isEmpty())
			addRow(getComboBox(str, alt));
		else
			addRow(new QLineEdit(str));
	}
}

DataItem StringListGroup::item() const
{
	QStringList list;
	foreach (const WidgetLine &line, m_widgets)
		list << getDataItem(line.title, line.data).data().toString();
	DataItem item;
	item.setName(objectName());
	item.setData(list);
	return item;
}

EditableDataLayout::EditableDataLayout(QWidget *parent) :
	AbstractDataLayout(parent)
{
}

bool EditableDataLayout::addItem(const DataItem &item)
{
	bool twoColumns;
	QWidget *widget = getWidget(item, &twoColumns);
	QWidget *title = 0;
	twoColumns = twoColumns || item.property("hideTitle", false);
	if (!twoColumns) {
		title = getTitle(item);
		addWidget(title, m_row, 0, 1, 1, Qt::AlignRight | Qt::AlignTop);
	}
	widget->setParent(parentWidget());
	widget->setObjectName(item.name());
	if (!twoColumns)
		addWidget(widget, m_row++, 1, 1, 1);
	else
		addWidget(widget, m_row++, 0, 1, 2);
	m_widgets.push_back(WidgetLine(title, widget));
	QSizePolicy::Policy policy = widget->sizePolicy().verticalPolicy();
	return policy == QSizePolicy::MinimumExpanding || policy == QSizePolicy::Expanding;
}

DataItem EditableDataLayout::item() const
{
	DataItem items;
	items.setName(objectName());
	foreach (const WidgetLine &line, m_widgets)
		items.addSubitem(getDataItem(line.title, line.data));
	return items;
}

QWidget *EditableDataLayout::getEditableWidget(const DataItem &item)
{
	return getWidget(item);
}

}
