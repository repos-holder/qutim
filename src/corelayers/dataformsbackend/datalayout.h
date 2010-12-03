#ifndef DATALAYOUT_H
#define DATALAYOUT_H

#include <QWidget>
#include <QGridLayout>
#include <QHostAddress>
#include <QPointer>
#include <qutim/dataforms.h>

class QLabel;
Q_DECLARE_METATYPE(QHostAddress);

namespace Core
{
using namespace qutim_sdk_0_3;

class DataLayout : public QGridLayout, public AbstractDataWidget
{
	Q_INTERFACES(qutim_sdk_0_3::AbstractDataWidget)
public:
	DataLayout(QWidget *parent = 0);
	~DataLayout();
	DataItem item() const;
	void addItem(const DataItem &item);
	void addItems(const QList<DataItem> &items);
	void addSpacer();
	void addRow(QWidget *widget) { addRow(0, widget); }
	void addRow(QWidget *title, QWidget *widget, Qt::Alignment widgetAligment = 0);
	bool isExpandable() { return m_expandable; }
protected:
	Qt::Alignment labelAlignment();
private:
	mutable Qt::Alignment m_labelAlignment;
	mutable QPointer<QStyle> m_style;
	int m_row;
	struct WidgetLine {
		WidgetLine(QWidget *t, QWidget *d) :
			title(t), data(d)
		{}
		QWidget *title;
		QWidget *data;
	};
	QList<WidgetLine> m_widgets;
	bool m_expandable;
};

}

#endif // DATALAYOUT_H
