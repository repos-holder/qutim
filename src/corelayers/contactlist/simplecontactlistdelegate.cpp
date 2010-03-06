#include "simplecontactlistdelegate.h"
#include "libqutim/tooltip.h"
#include <QToolTip>
/****************************************************************************
 *  simplecontactlistitem.cpp
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#include "simplecontactlistitem.h"
#include <QHelpEvent>
#include <QAbstractItemView>
#include <QApplication>
#include <libqutim/debug.h>
#include <QPainter>

namespace Core
{
	static int vertical_padding = 3;
	
	namespace SimpleContactList
	{
		SimpleContactListDelegate::SimpleContactListDelegate(QObject *parent) :
				QStyledItemDelegate(parent)
		{
		}
		
		void SimpleContactListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{			
			QStyleOptionViewItemV4 opt(option);
			QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
			
			const QAbstractItemModel *model = index.model();
			
			ItemType type = static_cast<ItemType>(model->data(index,ItemDataType).toInt());
			
			QString name = model->data(index,Qt::DisplayRole).toString();
			QString status_text = model->data(index,ItemStatusText).toString();
			
			switch (type) {
				case TagType: {
					style->drawPrimitive(QStyle::PE_PanelButtonCommand,&opt,painter, opt.widget);
					
					QPixmap pixmap(option.rect.size());
					pixmap.fill(Qt::transparent);				
					QPainter p(&pixmap);
					p.translate(-option.rect.topLeft());
					
					QFont font = opt.font;
					font.setItalic(true);
					font.setBold(true);
					p.setFont(font);
					
					p.drawText(option.rect.left(),
							   option.rect.top(),
							   option.rect.right() - 5,
							   option.rect.height(),
							   Qt::AlignRight | Qt::AlignVCenter,
							   name  
							   );
					
					p.end();
					painter->drawPixmap(option.rect,pixmap);
					
					break;
				}
				case ContactType: {
					style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
					
					
					if (!status_text.isEmpty()) {
 						QPixmap pixmap(option.rect.size());
						pixmap.fill(Qt::transparent);				
						QPainter p(&pixmap);
						p.translate(-option.rect.topLeft());
				
						p.setPen(opt.palette.color(QPalette::Shadow));
						QFont font = opt.font;
						font.setPointSize(font.pointSize() - 1);
						p.setFont(font);
						p.drawText(option.rect.left() + 25,
								   option.rect.top(),
								   option.rect.right(),
								   option.rect.height(),
								   Qt::AlignBottom,
								   status_text
									);
						p.end();
						painter->drawPixmap(option.rect,pixmap);
						
					}
					
					painter->drawText(option.rect.left() + 25,
									  option.rect.top() + vertical_padding,
									  option.rect.right(),
									  option.rect.height() - vertical_padding,
									  Qt::AlignTop,
									  name  
									);
					
					break;
				}
				default:
					break;
			}
			
			QIcon item_icon = model->data(index, Qt::DecorationRole).value<QIcon>();
			item_icon.paint(painter,
							option.rect.left() + 5, //FIXME
							option.rect.top(),
							16, //FIXME
							option.rect.height(),
							Qt::AlignVCenter);
		}

		
		bool SimpleContactListDelegate::helpEvent(QHelpEvent *event,
					   QAbstractItemView *view,
					   const QStyleOptionViewItem &option,
					   const QModelIndex &index)
		{
			Q_UNUSED(option);
		
			if (!event || !view)
				return false;
			if (event->type() == QEvent::ToolTip) {
				QHelpEvent *he = static_cast<QHelpEvent*>(event);
				if (getItemType(index) == ContactType) {
					ContactItem *item = reinterpret_cast<ContactItem*>(index.internalPointer());
					qutim_sdk_0_3::ToolTip::instance()->showText(he->globalPos(),
																 item->data->contact, view);
					return true;
				}
				QVariant tooltip = index.data(Qt::ToolTipRole);
				if (qVariantCanConvert<QString>(tooltip)) {
					QToolTip::showText(he->globalPos(), tooltip.toString(), view);
					return true;
				}
			}
			return false;
		}
		
		QSize SimpleContactListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			QSize size = QStyledItemDelegate::sizeHint(option, index);
			int height = index.model()->data(index,ItemStatusText).toString().isEmpty() ? 24 : 30; //TODO
			height = (index.model()->data(index,ItemDataType).toInt() == TagType) ? 20 : height;
			size.rheight() = height;	
			return size;
		}

	}
}
