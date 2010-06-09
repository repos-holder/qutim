#ifndef EDITABLEWIDGETS_H
#define EDITABLEWIDGETS_H

#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <libqutim/dataforms.h>

namespace Core
{

using namespace qutim_sdk_0_3;

static LocalizedString notSpecifiedStr = QT_TRANSLATE_NOOP("DataForms", "Not specified");

class CheckBox : public QCheckBox, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	CheckBox(const DataItem &item);
	virtual DataItem item() const;
};

class ComboBox : public QComboBox, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	ComboBox(const QString &value, const LocalizedStringList &alternatives, const DataItem &item);
	virtual DataItem item() const;
};

class DateTimeEdit : public QDateTimeEdit, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	DateTimeEdit(const DataItem &item);
	virtual DataItem item() const;
};

class DateEdit : public QDateEdit, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	DateEdit(const DataItem &item);
	virtual DataItem item() const;
};

class TextEdit : public QTextEdit, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	TextEdit(const DataItem &item);
	virtual DataItem item() const;
};

class LineEdit : public QLineEdit, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	LineEdit(const DataItem &item);
	virtual DataItem item() const;
};

class SpinBox : public QSpinBox, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	SpinBox(const DataItem &item);
	virtual DataItem item() const;
};

class DoubleSpinBox : public QDoubleSpinBox, public AbstractDataWidget
{
	Q_OBJECT
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	DoubleSpinBox(const DataItem &item);
	virtual DataItem item() const;
};

}

#endif // EDITABLEWIDGETS_H
