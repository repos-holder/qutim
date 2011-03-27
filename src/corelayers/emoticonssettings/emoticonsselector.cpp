/****************************************************************************
 *  emoticonsselector.cpp
 *
 *  Copyright (c) 2010 by Sidorov Aleksey <sauron@citadelspb.com>
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

#include "emoticonsselector.h"
#include "ui_emoticonsselector.h"
#include <qutim/emoticons.h>
#include <qmovie.h>
#include <QLabel>
#include "flowlayout.h"
#include <QDebug>

EmoticonsSelector::EmoticonsSelector() : ui(new Ui::emoticonsSelector)
{
	ui->setupUi(this);
	connect(ui->themeSelector,SIGNAL(currentIndexChanged(QString)),SLOT(currentIndexChanged(QString)));
	FlowLayout *flowLayout = new FlowLayout;
	ui->emoticons->setLayout(flowLayout);
}

EmoticonsSelector::~EmoticonsSelector()
{
	delete ui;
	clearEmoticonsPreview();
}

void EmoticonsSelector::loadImpl()
{
	 QStringList themeList =  Emoticons::themeList();
	 int index = themeList.indexOf(QLatin1String(""));
	 themeList.removeAt(index);
	 themeList.sort();
	 themeList.prepend(tr("No emoticons"));
	 ui->themeSelector->addItems(themeList);
	 cancelImpl();
}

void EmoticonsSelector::cancelImpl()
{
	int index = ui->themeSelector->findText(Emoticons::currentThemeName());
	ui->themeSelector->setCurrentIndex(qMax(0, index));
}

void EmoticonsSelector::saveImpl()
{
	Emoticons::setTheme(m_selected_theme);
}

void EmoticonsSelector::currentIndexChanged(const QString& text)
{
	QHash<QString, QStringList> theme_map = Emoticons::theme(text).emoticonsMap();
	QHash<QString, QStringList>::const_iterator it;
	clearEmoticonsPreview();
	for (it = theme_map.constBegin();it != theme_map.constEnd();it ++) {
		QLabel *label = new QLabel(this);
		QMovie *emoticon = new QMovie (it.key(), QByteArray(), label);
		label->setMovie(emoticon);
		ui->emoticons->layout()->addWidget(label);
		m_active_emoticons.append(label);
		emoticon->start();
	}
	m_selected_theme = text;
	emit modifiedChanged(true);
}

void EmoticonsSelector::clearEmoticonsPreview()
{
	qDeleteAll(m_active_emoticons);
	m_active_emoticons.clear();
}
