#ifndef KB_QWERTY_H
#define KB_QWERTY_H

#include <QWidget>


namespace Ui {
    class kb_Qwerty;
}
namespace Core
{

namespace AdiumChat
{



class kb_Qwerty : public QWidget
{
    Q_OBJECT

public:
    explicit kb_Qwerty(QWidget *parent = 0);
    ~kb_Qwerty();
    static const QString DELETE;
    static const QString NEWLINE;
    static const QString SPACE;

private:
    Ui::kb_Qwerty *ui;
    int m_nActiveModifiers;
    void setButtonsLabel();

public slots:
    void directInputClicked();
           // void settingsChanged(int);
signals:
    void input(QString);

private slots:
    void maiuscClicked(/*int nForceCheck = 0*/);
    void symClicked();
    void newLineClicked();
    void spaceClicked();
    void deleteClicked();
    void accentClicked();

};
}
}

#endif // KB_QWERTY_H
