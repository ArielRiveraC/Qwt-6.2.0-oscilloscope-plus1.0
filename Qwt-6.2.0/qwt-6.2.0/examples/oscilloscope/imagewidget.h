#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QBuffer>
#include <QPushButton>
#include <QMessageBox>

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = 0);

    void loadfileImage(QBuffer*, QString);
    void handle_savebutton();
    void handle_closebutton();

private:
    QLabel* m_imagelabel;
    QLabel* m_textlabel;
    QString m_namefile;
    QPushButton* m_savebutton;
    QPushButton* m_closebutton;
    QPushButton* m_previousbutton;
    QPushButton* m_nextbutton;
    QBuffer* m_imageqbuffer;
};

#endif // IMAGEWIDGET_H
