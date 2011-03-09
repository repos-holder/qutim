/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2011 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This file is part of free software; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************
 ****************************************************************************/

#include "simpleaboutdialog.h"
#include "ui_simpleaboutdialog.h"
#include <qutim/plugin.h>
#include <qutim/debug.h>
#include <QHash>
#include <QFile>
#include <QRegExp>

using namespace qutim_sdk_0_3;

namespace Core
{
typedef QPair<PersonInfo, int> PersonIntPair;
typedef QHash<QString, PersonIntPair> StringPersonHash;

bool personLessThen(const PersonIntPair &a, const PersonIntPair &b)
{
	return a.second > b.second || (a.second == b.second && a.first.name() < b.first.name());
}

SimpleAboutDialog::SimpleAboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SimpleAboutDialog)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	StringPersonHash authors;
	StringPersonHash::iterator it;
	foreach (Plugin *plugin, pluginsList()) {
		foreach (const PersonInfo &person, plugin->info().authors()) {
			it = authors.find(person.name());
			if (it == authors.end())
				it = authors.insert(person.name(), qMakePair(person, 0));
			it.value().second += plugin->avaiableExtensions().size();
		}
	}
	QVector<PersonIntPair> persons;
	persons.reserve(authors.size());
	it = authors.begin();
	for (; it != authors.end(); it++)
		persons.append(it.value());
	qSort(persons.begin(), persons.end(), personLessThen);
	QString html;
	for (int i = 0; i < persons.size(); i++) {
		const PersonInfo &info = persons.at(i).first;
		html += QLatin1String("<p><div><b>");
		html += Qt::escape(info.name());
		html += QLatin1String("</b></div><div>");
		html += Qt::escape(info.task());
		html += QLatin1String("</div><div><a href=\"mailto:\"");
		html += Qt::escape(info.email());
		html += QLatin1String("\">");
		html += Qt::escape(info.email());
		html += QLatin1String("</a></div></p>");
	}
	ui->texteditDevelopers->setHtml(html);
	ui->labelVersion->setText(qutimVersionStr());
	ui->labelQtVer ->setText(tr("Based on Qt %1 (%2 bit).").arg(QLatin1String(QT_VERSION_STR), QString::number(QSysInfo::WordSize)));
	QFile licenseFile(":/GPL");
	QString license = tr("<div><b>qutIM</b> %1 is licensed under GNU General Public License, version 2"
								" or (at your option) any later version.</div>"
								"<div>qutIM resources such as themes, icons, sounds may come along with a "
								"different license.</div><br><hr><br>").arg(qutimVersionStr());
	if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text))
		license += Qt::escape(QLatin1String(licenseFile.readAll()));
	else
		license += "<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPLv2</a>";
	license.replace(QLatin1String("\n\n"), "<br><br>");
	const char * const authorsStr  = "Dear translators, please, replace this string with your names like this:"
			" [RU : Russian Federation] <br><br> Vasya Pupkin <br> <a href=\"mailto:vasya@example.com\">mailto:vasya@example.com</a>"
			" <br><br> Petya Pomazok <br> <a href=\"mailto:petya@example.org\">petya@example.org</a>";
	QString translatedStr = tr(authorsStr);
	if (translatedStr != QLatin1String(authorsStr))
		ui->texteditTranslators->setHtml(translatedStr);
	else
		ui->tabWidget->removeTab(1);
	ui->texteditLicense->setHtml(license);
}

SimpleAboutDialog::~SimpleAboutDialog()
{
	delete ui;
}
}
