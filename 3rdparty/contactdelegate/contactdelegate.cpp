#include "contactdelegate.h"
#include <libqutim/buddy.h>
#include <libqutim/tooltip.h>
#include <QApplication>
#include <QHelpEvent>
#include <QTreeView>
#include <avatarfilter.h>
#include <QPainter>
#include <QLatin1Literal>

namespace qutim_sdk_0_3
{

struct ContactDelegatePrivate
{
	const QWidget *getWidget(const QStyleOptionViewItem &option)
	{
		if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option))
			return v3->widget;

		return 0;
	}
	QStyle *getStyle(const QStyleOptionViewItem& option)
	{
		if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option))
			return v3->widget ? v3->widget->style() : QApplication::style();

		return QApplication::style();
	}
	int verticalPadding;
	int horizontalPadding;
	ContactDelegate::ShowFlags showFlags;
	QHash<QString, bool> extInfo;
};

bool contactInfoLessThan (const QVariantHash &a, const QVariantHash &b) {
	QString priority = QLatin1String("priorityInContactList");
	int p1 = a.value(priority).toInt();
	int p2 = b.value(priority).toInt();
	return p1 > p2;
}

ContactDelegate::ContactDelegate(QObject *parent) :
		QAbstractItemDelegate(parent),p(new ContactDelegatePrivate)
{
	p->horizontalPadding = 5;
	p->verticalPadding = 3;
	p->showFlags = ShowExtendedInfoIcons | ShowAvatars;
}

ContactDelegate::~ContactDelegate()
{

}

void ContactDelegate::paint(QPainter *painter,
							const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 opt(option);
	painter->save();
	QStyle *style = p->getStyle(option);

	ContactItemType type = static_cast<ContactItemType>(index.data(ItemTypeRole).toInt());

	QString name = index.data(Qt::DisplayRole).toString();

	QRect title_rect = option.rect;
	title_rect.adjust(p->horizontalPadding,
					  p->verticalPadding,
					  0,
					  -p->verticalPadding);

	switch (type) {
	case TagType: {
			QStyleOptionButton buttonOption;

			buttonOption.state = option.state;
#ifdef Q_WS_MAC
			buttonOption.features = QStyleOptionButton::Flat;
			buttonOption.state |= QStyle::State_Raised;
			buttonOption.state &= ~QStyle::State_HasFocus;
#endif

			buttonOption.rect = option.rect;
			buttonOption.palette = option.palette;
			style->drawControl(QStyle::CE_PushButton, &buttonOption, painter, opt.widget);

			if(QTreeView *view = static_cast<QTreeView *>(parent())) {
				QStyleOption branchOption;
				static const int i = 9; // ### hardcoded in qcommonstyle.cpp
				QRect r = option.rect;
				branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
				branchOption.palette = option.palette;
				branchOption.state = QStyle::State_Children;
				title_rect.adjust(branchOption.rect.width() +p->horizontalPadding,0,0,0);

				if (view->isExpanded(index))
					branchOption.state |= QStyle::State_Open;

				style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, view);
			}

			QFont font = opt.font;
			font.setBold(true);
			painter->setFont(font);

			QString count = index.data(ContactsCountRole).toString();
			QString online_count = index.data(OnlineContactsCountRole).toString();

			QString txt = name % QLatin1Literal(" (")
						  % online_count
						  % QLatin1Char('/')
						  % count
						  % QLatin1Char(')');

			painter->drawText(title_rect,
							  Qt::AlignVCenter,
							  txt
							  );
			break;
		}
	case ContactType: {
			style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
			QRect bounding;
			Status status = index.data(StatusRole).value<Status>();

			if (p->showFlags & ShowExtendedInfoIcons)
			{
				QHash<QString, QVariantHash> extStatuses = status.extendedInfos();

				QList<QVariantHash> list;
				foreach (const QVariantHash &data, extStatuses) {
					QList<QVariantHash>::iterator search_it =
							qLowerBound(list.begin(), list.end(), data, contactInfoLessThan);
					list.insert(search_it,data);
				}

				QString icon = QLatin1String("icon");
				QString showIcon = QLatin1String("showIcon");
				QString id = QLatin1String("id");

				foreach (const QVariantHash &hash, list) {
					QVariant extIconVar = hash.value(icon);
					QIcon icon;
					if (extIconVar.canConvert<ExtensionIcon>())
						icon = extIconVar.value<ExtensionIcon>().toIcon();
					else if (extIconVar.canConvert(QVariant::Icon))
						icon = extIconVar.value<QIcon>();
					if (!hash.value(showIcon,true).toBool() || icon.isNull())
						continue;
					if (!p->extInfo.value(hash.value(id).toString(), true))
						continue;
					icon.paint(painter,
							   option.rect.left() + p->horizontalPadding,
							   option.rect.top() + p->verticalPadding,
							   title_rect.right() - p->horizontalPadding,
							   option.decorationSize.height(), //FIXME
							   Qt::AlignTop |
							   Qt::AlignRight);
					title_rect.adjust(0,0,-option.decorationSize.width()-p->horizontalPadding/2,0);
				}
			}
			title_rect.adjust(opt.decorationSize.width()+p->horizontalPadding,
							  0,
							  0,
							  0);
			bool isStatusText = (p->showFlags & ShowStatusText) && !status.text().isEmpty();
			painter->drawText(title_rect,
							  isStatusText ? Qt::AlignTop : Qt::AlignVCenter,
							  name,
							  &bounding
							  );

			if (isStatusText) {
				QRect status_rect = title_rect;
				status_rect.setTop(status_rect.top() + bounding.height());
				//small hack
#ifdef Q_WS_MAC
				painter->setPen(opt.palette.color(QPalette::Inactive, QPalette::WindowText));
#else
				painter->setPen(opt.palette.color(QPalette::Shadow));
#endif
				QFont font = opt.font;
				font.setPointSize(font.pointSize()-1);
				painter->setFont(font);
				painter->drawText(status_rect,
								  Qt::AlignTop | Qt::TextWordWrap,
								  status.text().remove(QLatin1Char('\n'))
								  );
			}

			QIcon itemIcon = index.data(Qt::DecorationRole).value<QIcon>();
			bool hasAvatar = false;
			if (p->showFlags & ShowAvatars) {
				QString avatar = index.data(AvatarRole).toString();
//				if (avatar.isEmpty())
//					avatar = IconLoader::instance()->iconPath("user-identity",option.decorationSize.width());
				QSize avatarSize (option.decorationSize.width() + p->horizontalPadding,
								  option.decorationSize.height() + p->verticalPadding);
				AvatarFilter filter(avatarSize);
				hasAvatar = filter.draw(painter,
										option.rect.left()+p->horizontalPadding/2,
										option.rect.top()+p->verticalPadding/2,
										avatar,
										itemIcon);
			}
			if (!hasAvatar) {
				itemIcon.paint(painter,
							   option.rect.left() + p->horizontalPadding,
							   option.rect.top() + p->verticalPadding,
							   option.decorationSize.width(),
							   option.decorationSize.height(),
							   Qt::AlignTop);
			}
			break;
		}
	default:
		break;
	}
	painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QVariant value = index.data(Qt::SizeHintRole);
	if (value.isValid())
		return value.toSize();

	const QWidget *widget = p->getWidget(option);
	QRect rect = widget->geometry();
	rect.adjust(2*p->horizontalPadding + option.decorationSize.width(),0,0,0);
	QFontMetrics metrics = option.fontMetrics;
	int height = metrics.boundingRect(rect, Qt::TextSingleLine,
									  index.data(Qt::DisplayRole).toString()).height();

	Status status = index.data(StatusRole).value<Status>();

	ContactItemType type = static_cast<ContactItemType>(index.data(ItemTypeRole).toInt());

	bool isContact = (type == ContactType);

	if (isContact && (p->showFlags & ShowStatusText) && !status.text().isEmpty()) {
		QFont desc_font = option.font;
		desc_font.setPointSize(desc_font.pointSize() -1);
		metrics = QFontMetrics(desc_font);
		height += metrics.boundingRect(rect,
									   Qt::TextSingleLine,
									   // There is no differ what text to use, but shotter is better
									   QLatin1String(".")
									   //											status.text().remove(QLatin1Char('\n'))
									   ).height();
	}
	if (isContact)
		height = qMax(option.decorationSize.height(),height);

	height += 2*p->verticalPadding;
	QSize size (option.rect.width(),height);
	return size;
}

bool ContactDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
										const QStyleOptionViewItem &option,
										const QModelIndex &index)
{
	if (event->type() == QEvent::ToolTip) {
		Buddy *buddy = index.data(BuddyRole).value<Buddy*>();
		if (buddy)
			ToolTip::instance()->showText(event->globalPos(), buddy, view);
		return true;
	} else {
		return QAbstractItemDelegate::helpEvent(event, view, option, index);
	}
}

void ContactDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
								const QRect &rect) const
{
	Q_UNUSED(rect);
	QStyle *style = p->getStyle(option);
	const QWidget *widget = p->getWidget(option);
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, widget);
}

void ContactDelegate::setShowFlags(ShowFlags flags)
{
	p->showFlags = flags;
}

void ContactDelegate::setExtInfo(const QHash<QString, bool> &info)
{
	p->extInfo = info;
}

}
