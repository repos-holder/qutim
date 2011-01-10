/****************************************************************************
 *  servicechooserwidget.cpp
 *
 *  Copyright (c) 2010 by Aleksey Sidorov <sauron@citadelspb.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*****************************************************************************/

#include "servicechooserwidget.h"
#include "ui_servicechooserwidget.h"
#include <QStandardItem>
#include <QModelIndex>
#include <qutim/extensioninfo.h>
#include <qutim/debug.h>
#include <qutim/icon.h>
#include "qutim/metaobjectbuilder.h"
#include <QStringBuilder>
#include "itemdelegate.h"
#include "serviceitem.h"
#include "servicechooser.h"
#include <qutim/configbase.h>
#include <qutim/notificationslayer.h>

namespace Core
{
ServiceChooserWidget::ServiceChooserWidget() :
	ui(new Ui::ServiceChooser),
	m_model(new QStandardItemModel)
{
	ui->setupUi(this);
	ui->treeView->setModel(m_model);
	ui->treeView->setItemDelegate(new ItemDelegate(ui->treeView));
	ui->treeView->setAnimated(false);
	ui->treeView->setExpandsOnDoubleClick(false);
	connect(ui->treeView,SIGNAL(clicked(QModelIndex)),SLOT(onItemClicked(QModelIndex)));

	connect(m_model,SIGNAL(itemChanged(QStandardItem*)),SLOT(onItemChanged(QStandardItem*)));
}
ServiceChooserWidget::~ServiceChooserWidget()
{
	delete ui;
}

const char *serviceIcon(const char *serviceName)
{
	if (!qstrcmp(serviceName,"TrayIcon"))
		return "user-desktop";
	if (!qstrcmp(serviceName,"ChatLayer"))
		return "view-list-text";
	if (!qstrcmp(serviceName,"ContactList"))
		return "view-list-details";
	return "applications-system";
}

void ServiceChooserWidget::loadImpl()
{
	clear();
	ConfigGroup group = Config().group("services");
	QVariantMap selected = group.value("list", QVariantMap());
	QStandardItem *parent_item = m_model->invisibleRootItem();

	ExtensionInfoList exts = extensionList();
	for (int i = 0; i < exts.size(); i++) {
		const ExtensionInfo &info = exts.at(i);
		const char *serviceName = MetaObjectBuilder::info(info.generator()->metaObject(), "Service");

		if (serviceName && *serviceName) {
			if (!m_service_items.contains(serviceName)) {
				QString localizedName = QT_TRANSLATE_NOOP("Service",serviceName).toString();
				ServiceItem *item = new ServiceItem(Icon(serviceIcon(serviceName)),localizedName);
				item->setData(true,ServiceItem::ExclusiveRole);
				parent_item->appendRow(item);
				m_service_items.insert(serviceName,item);
			}
			QIcon icon = !info.icon().name().isEmpty() ? info.icon() : Icon("applications-system");
			ServiceItem *item = new ServiceItem(icon,info.name());

			item->setToolTip(ServiceChooser::html(info));
			item->setCheckable(true);
			item->setData(info.description().toString(),DescriptionRole);
			if (selected.value(serviceName).toString() == ServiceChooser::className(info))
				item->setCheckState(Qt::Checked);
			item->setData(ServiceChooser::className(info),ServiceItem::ClassNameRole);

			m_service_items.value(serviceName)->appendRow(item);
		}
	}
}
void ServiceChooserWidget::cancelImpl()
{

}
void ServiceChooserWidget::saveImpl()
{
	ConfigGroup group = Config().group("services");
	QVariantMap selected;
	QHash<QByteArray, ServiceItem *>::const_iterator it;
	for (it = m_service_items.constBegin();it!=m_service_items.constEnd();it++) {
		QVariant service;
		for (int i =0;i!=it.value()->rowCount();i++) {
			Qt::CheckState state = static_cast<Qt::CheckState>(it.value()->child(i)->data(Qt::CheckStateRole).toInt());
			if (state == Qt::Checked) {
				service = it.value()->child(i)->data(ServiceItem::ClassNameRole);
				break;
			}
		}
		selected.insert(it.key(),service);
	}
	group.setValue("list", selected);
	group.sync();
	Notifications::send(tr("To take effect you must restart qutIM"));
}

void ServiceChooserWidget::clear()
{
	m_model->clear();
	m_service_items.clear();
}

void ServiceChooserWidget::onItemChanged(QStandardItem* )
{
	emit modifiedChanged(true);
}

void ServiceChooserWidget::onItemClicked(QModelIndex index)
{
	if (ui->treeView->isExpanded(index))
		ui->treeView->collapse(index);
	else
		ui->treeView->expand(index);
}

}
