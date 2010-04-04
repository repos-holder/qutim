#ifndef CONTACTINFO_H
#define CONTACTINFO_H

#include "QWidget"
#include "QGroupBox"
#include "QScrollArea"
#include <libqutim/buddy.h>
#include <libqutim/inforequest.h>

class QLabel;
class QGridLayout;
class QVBoxLayout;

namespace Core
{
using namespace qutim_sdk_0_3;

class InfoGroup : public QGroupBox
{
	Q_OBJECT
public:
	InfoGroup(QWidget *parent = 0);
	void addItems(const QList<InfoItem> &items);
	void addItem(const InfoItem &item);
private:
	void addDataWidget(QWidget *widget);
	QLabel *addLabel(const QString &data);
	QGridLayout *m_layout;
	int m_row;
};

class MainWindow : public QScrollArea
{
	Q_OBJECT
public:
	MainWindow();
	void setBuddy(Buddy *buddy, InfoRequest *request);
private slots:
	void onRequestStateChanged(InfoRequest::State state);
private:
	void addItems(const InfoItem &items);
private:
	QVBoxLayout *m_layout;
	InfoRequest *request;
};

class ContactInfo : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("Service", "ContactInfo")
	Q_CLASSINFO("Uses", "IconLoader")
public:
	ContactInfo();
public slots:
	void show(Buddy *buddy);
private slots:
	void onShow();
private:
	QPointer<MainWindow> info;
};

}

#endif // CONTACTINFO_H
