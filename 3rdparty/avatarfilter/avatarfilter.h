#ifndef AVATARFILTER_H
#define AVATARFILTER_H
#include <QString>
#include <QScopedPointer>
#include <QGraphicsEffect>

class QIcon;
namespace qutim_sdk_0_3
{
	class Status;
	struct AvatarFilterPrivate;
	class AvatarFilter
	{
		Q_DECLARE_PRIVATE(AvatarFilter)
	public:
		AvatarFilter(const QSize &defaultSize/*, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio*/);
		~AvatarFilter();
		void draw(QPainter *painter, int x, int y,
				  const QString &path,const QIcon &overlayIcon) const;
	private:
		QScopedPointer<AvatarFilterPrivate> d_ptr;
	};

}
#endif // AVATARFILTER_H
