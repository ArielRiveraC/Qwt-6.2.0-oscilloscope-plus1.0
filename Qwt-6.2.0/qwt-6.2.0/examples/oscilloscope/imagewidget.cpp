#ifndef IMAGEWIDGET_CPP
#define IMAGEWIDGET_CPP

#include "ImageWidget.h"

#include <QHBoxLayout>
#include <qfiledialog.h>

ImageWidget::ImageWidget(QWidget *parent) :
    QWidget(parent)
{
    setStyleSheet( "QWidget{ background-color : "
                   "rgba( 160, 160, 160, 255); border-radius : 7px;  }" );

    m_imagelabel = new QLabel(this);
    m_textlabel = new QLabel(this);

    m_previousbutton = new QPushButton("Previous", this);
    m_previousbutton->setStyleSheet("color:red;font-weight:bold");
    m_savebutton = new QPushButton("Save", this);
    m_savebutton->setStyleSheet("color:red;font-weight:bold");
    m_closebutton = new QPushButton("Close", this);
    m_closebutton->setStyleSheet("color:red;font-weight:bold");
    m_nextbutton = new QPushButton("Next", this);
    m_nextbutton->setStyleSheet("color:red;font-weight:bold");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_textlabel);
    layout->addWidget(m_imagelabel);
    QHBoxLayout* layout1 = new QHBoxLayout();
    layout1->addWidget(m_previousbutton, 50);
    layout1->addWidget(m_savebutton, 50);
    layout1->addWidget(m_closebutton, 50);
    layout1->addWidget(m_nextbutton, 50);
    layout->addLayout(layout1);
    setLayout(layout);

    connect( m_savebutton, &QPushButton::released, this,
             &ImageWidget::handle_savebutton);
    connect( m_closebutton, &QPushButton::released, this,
             &ImageWidget::handle_closebutton);
}

void ImageWidget::loadfileImage(QBuffer* imagebuffer, QString imagefilename)
{
    QImage image;

    m_namefile = imagefilename;
    m_imageqbuffer = imagebuffer;
    image.loadFromData(m_imageqbuffer->data(), nullptr);
    m_imagelabel->setPixmap(QPixmap::fromImage(image));
    m_imagelabel->adjustSize();
    m_textlabel->setText("File name: " + m_namefile);
}

void ImageWidget::handle_savebutton()
{
    QImage image;
    image.loadFromData(m_imageqbuffer->data(), nullptr);

    QString fileName = QFileDialog::getSaveFileName( this, tr("Save Image"),
                       m_namefile, tr("Images (*.png);;All Files (*)"));

    if (!fileName.isEmpty()) {
        if (image.save(fileName)) {
            QMessageBox::information(this, tr("Image Saved"),
                                  tr("Image has been saved successfully!"));
        } else {
            QMessageBox::warning(this, tr("Image Save Failed"),
                              tr("Failed to save the image."));
        }
    }
}

void ImageWidget::handle_closebutton()
{
      this->close();
}

#endif // IMAGEWIDGET_CPP
