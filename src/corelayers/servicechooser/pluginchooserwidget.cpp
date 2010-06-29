/****************************************************************************
 *  pluginchooserwidget.cpp
 *
 *  Copyright (c) 2010 by Aleksey Sidorov <sauron@citadelspb.com>
 *  Copyright (c) 2010 by Nikita Belov <null@deltaz.org>
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

#include "pluginchooserwidget.h"
#include "ui_servicechooserwidget.h"
#include <QStandardItem>
#include <libqutim/extensioninfo.h>
#include <libqutim/debug.h>
#include <libqutim/icon.h>
#include <QStringBuilder>
#include "servicedelegate.h"
#include "serviceitem.h"
#include "servicechooser.h"
#include <libqutim/configbase.h>
#include <libqutim/notificationslayer.h>
#include <libqutim/plugin.h>
#include <libqutim/protocol.h>

namespace Core
{
	PluginChooserWidget::PluginChooserWidget() :
	ui(new Ui::ServiceChooser),
	m_model(new QStandardItemModel)
	{
		ui->setupUi(this);
		ui->toolButton->hide();
		ui->serviceInfo->hide();
		ui->treeView->setModel(m_model);
		ui->treeView->setItemDelegate(new ServiceDelegate(this));
		ui->treeView->setIndentation(0);

		connect(m_model,SIGNAL(itemChanged(QStandardItem*)),SLOT(onItemChanged(QStandardItem*)));
	}
	PluginChooserWidget::~PluginChooserWidget()
	{
		delete ui;
	}

	void PluginChooserWidget::loadImpl()
	{
		clear();
		Config group = Config().group("plugins").group("list");
		QStandardItem *parent_item = m_model->invisibleRootItem();

		QList<QPointer<Plugin>> plugins = pluginsList();
		for (int i = 0; i < plugins.size(); i++)
		{
			const PluginInfo &info = plugins.at(i)->info();
			if (!m_plugin_items.contains(info.name()))
			{
				QIcon icon = info.icon();
				if (icon.isNull() || !icon.availableSizes().count())
					icon = Icon("help-hint");
				ServiceItem *item = new ServiceItem(icon, info.name(), true);
				item->setToolTip(html(info));
				item->setCheckable(true);
				item->setData(false, Qt::UserRole);
				item->setCheckState((group.value(info.name(), true) ? Qt::Checked : Qt::Unchecked));
				parent_item->appendRow(item);
				m_plugin_items.insert(info.name(), item);
				m_plugins.insert(info.name(), plugins.at(i));
			}
		}
	}
	void PluginChooserWidget::cancelImpl()
	{

	}
	void PluginChooserWidget::saveImpl()
	{
		Config group = Config().group("plugins").group("list");
		QHash<QString, ServiceItem *>::const_iterator it;
		bool needRestart = false;
		for (it = m_plugin_items.constBegin();it!=m_plugin_items.constEnd();it++)
		{
			bool oldValue = group.value(it.key(), true);
			bool newValue = (it.value()->checkState() == Qt::Checked ? true : false);

			group.setValue(it.key(), newValue);

			int unloadFails = 0;

			if ( oldValue && !newValue )
			{
				if( m_plugins.value(it.key())->avaiableExtensions().count() )
					needRestart = true;
				else
					m_plugins.value(it.key())->unload();
			}
			else if ( !oldValue && newValue )
			{
				if( m_plugins.value(it.key())->avaiableExtensions().count() )
					needRestart = true;
				else
					m_plugins.value(it.key())->load();
			}
		}
		if (needRestart)
			Notifications::sendNotification(tr("To take effect you must restart qutIM"));
	}

	void PluginChooserWidget::clear()
	{
		m_model->clear();
		m_plugin_items.clear();
	}
	
	void PluginChooserWidget::onItemChanged(QStandardItem* )
	{
		emit modifiedChanged(true);
	}

	QString PluginChooserWidget::html(const qutim_sdk_0_3::PluginInfo& info)
	{
		QString html = tr("<b>Name: </b> %1 <br />").arg(info.name());
		html += tr("<b>Description: </b> %1 <br />").arg(info.description());
		
		html += "<blockoute>";
		foreach (const PersonInfo &person, info.authors()) {
			html += "<br/>";
			html += tr("<b>Name:</b> %1<br/>").arg(person.name());
			if ( !person.task().toString().isEmpty() )
				html += tr("<b>Task:</b> %1<br/>").arg(person.task());
			if ( !person.email().isEmpty() )
				html += tr("<b>Email:</b> <a href=\"mailto:%1\">%1</a><br/>").arg(person.email());
			if ( !person.web().isEmpty() )
				html += tr("<b>Webpage:</b> <a href=\"%1\">%1</a><br/>").arg(person.web());
		}
		html += "</blockoute>";
		return html;
	}	

}
