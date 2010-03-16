#ifndef SIMPLETAGSEDITOR_H
#define SIMPLETAGSEDITOR_H

#include <QWidget>
#include <libqutim/libqutim_global.h>
#include <QSet>

namespace Ui {
	class SimpleTagsEditor;
}

namespace qutim_sdk_0_3
{
	class Contact;
}

using namespace qutim_sdk_0_3;

namespace Core
{
	class SimpleTagsEditor : public QWidget {
		Q_OBJECT
	public:
		SimpleTagsEditor(Contact *contact);
		~SimpleTagsEditor();
		void setTags (QSet<QString> tags);
	public slots:
		void load();
	private slots:
		void onActionTriggered(QAction *act);
		void onAddTagTriggered();
		void save();
	protected:
		void changeEvent(QEvent *e);
	private:
		Ui::SimpleTagsEditor *ui;
		Contact *m_contact;
		QSet<QString> m_additional_tags;
	};

}
#endif // SIMPLETAGSEDITOR_H
