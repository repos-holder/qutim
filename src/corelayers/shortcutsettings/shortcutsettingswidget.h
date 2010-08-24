#ifndef SHORTCUTSETTINGSWIDGET_H
#define SHORTCUTSETTINGSWIDGET_H

#include <QWidget>
#include <qutim/settingswidget.h>

namespace Ui {
    class ShortcutSettingsWidget;
}
class QStandardItemModel;
class QStandardItem;
namespace Core
{
	using namespace qutim_sdk_0_3;
	class ShortcutSettingsWidget : public SettingsWidget
	{
		Q_OBJECT

	public:
		explicit ShortcutSettingsWidget();
		~ShortcutSettingsWidget();
		virtual void loadImpl();
		virtual void saveImpl();
		virtual void cancelImpl();
	protected:
		void changeEvent(QEvent *e);
	private slots:
		void onItemChanged(QStandardItem *item);
	private:
		Ui::ShortcutSettingsWidget *ui;
		void fillModel(QHash <QString, QStandardItem*> &group, bool global);
		QStandardItemModel *m_model;
		QList<QStandardItem *> m_changed_items;
	};

}

#endif // SHORTCUTSETTINGSWIDGET_H
