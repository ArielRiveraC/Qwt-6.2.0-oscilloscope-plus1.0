/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "MainWindow.h"
#include "Plot.h"
#include "Knob.h"
#include "WheelBox.h"
#include "ImageWidget.h"

#include <QLayout>
#include <QColorDialog>

MainWindow::MainWindow( QWidget* parent )
    : QWidget( parent )
{
    const double intervalLength = 10.0; // seconds

    m_plot = new Plot();
    m_plot->setIntervalLength( intervalLength );

    m_amplitudeKnob = new Knob( "Amplitude", 0.0, 200.0 );
    m_amplitudeKnob->setValue( 160.0 );

    m_frequencyKnob = new Knob( "Frequency [Hz]", 0.1, 20.0 );
    m_frequencyKnob->setValue( 17.8 );

    m_intervalWheel = new WheelBox( "Displayed [s]", 1.0, 100.0, 1.0 );
    m_intervalWheel->setValue( intervalLength );

    m_timerWheel = new WheelBox( "Sample Interval [ms]", 0.0, 20.0, 0.1 );
    m_timerWheel->setValue( 10.0 );

    m_Stopbutton = new QPushButton("Stop", this);
    m_Stopbutton->setStyleSheet("color:red;font-weight:bold");
    m_Capturebutton = new QPushButton("Capture", this);
    m_Capturebutton->setStyleSheet("color:red;font-weight:bold");
    m_ChangeColorbutton = new QPushButton("Color", this);
    m_ChangeColorbutton->setStyleSheet("color:red;font-weight:bold");

    m_legendItem_checkBox = new QCheckBox( "Show Item Legend" );
    m_legendItem_checkBox->setCheckState(Qt::Unchecked);

    QVBoxLayout* vLayout1 = new QVBoxLayout();
    vLayout1->addWidget( m_intervalWheel );
    vLayout1->addWidget( m_timerWheel );
    vLayout1->addWidget( m_plot->getLevelWheel() );
    vLayout1->addStretch( 10 );
    vLayout1->addWidget( m_amplitudeKnob );
    vLayout1->addWidget( m_frequencyKnob );
    vLayout1->addWidget( m_legendItem_checkBox );
    vLayout1->addWidget(m_Stopbutton);
    vLayout1->addWidget(m_Capturebutton);
    vLayout1->addWidget(m_ChangeColorbutton);

    m_captureListWidget = new QListWidget(this);
    vLayout1->addWidget(m_captureListWidget);

    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->addWidget( m_plot, 10 );
    layout->addLayout( vLayout1 );

    connect( m_Stopbutton, &QPushButton::released, this,
             &MainWindow::handle_Stopbutton);
    connect( m_Capturebutton, &QPushButton::released, this,
             &MainWindow::handle_Capturebutton);
    connect( m_ChangeColorbutton, &QPushButton::released, this,
             &MainWindow::handle_ChangeColorbutton);
    connect( m_captureListWidget, &QListWidget::itemDoubleClicked, this,
             &MainWindow::handle_ItemDoubleClicked);
    connect( m_legendItem_checkBox, &QCheckBox::released, this,
             &MainWindow::handle_legendItem_checkBox);

    connect( m_amplitudeKnob, SIGNAL(valueChanged(double)),
        SIGNAL(amplitudeChanged(double)) );

    connect( m_frequencyKnob, SIGNAL(valueChanged(double)),
        SIGNAL(frequencyChanged(double)) );

    connect( m_timerWheel, SIGNAL(valueChanged(double)),
        SIGNAL(signalIntervalChanged(double)) );

    connect( m_intervalWheel, SIGNAL(valueChanged(double)),
        m_plot, SLOT(setIntervalLength(double)) );
}

void MainWindow::start()
{
    m_plot->start();
}

double MainWindow::frequency() const
{
    return m_frequencyKnob->value();
}

void MainWindow::frequencyChanged(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);

    update_Curvetitle();
}

double MainWindow::amplitude() const
{
    return m_amplitudeKnob->value();
}

void MainWindow::amplitudeChanged(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);

    update_Curvetitle();
}

void MainWindow::update_Curvetitle()
{
    QString legenItemText = QString("Amplitude: %1 V\nFrequency: %2 Hz")
                                    .arg(amplitude()).arg(frequency());

    m_plot->setCurveTitle(legenItemText);
}

double MainWindow::signalInterval() const
{
    return m_timerWheel->value();
}

void MainWindow::handle_ChangeColorbutton()
{     
     QColor color = QColorDialog::getColor(Qt::yellow, this);
     m_plot->setColorCurve(color);
}

void MainWindow::handle_legendItem_checkBox()
{
     if( m_legendItem_checkBox->checkState() == Qt::Checked ){
         update_Curvetitle();
         m_plot->updateItemLegend( Qt::Checked );
     }
     else{
         m_plot->updateItemLegend( Qt::Unchecked );
     }
}

void MainWindow::handle_Stopbutton()
{
    if ( m_plot->setStartTimer())
        m_Stopbutton->setText("Stop");
    else
        m_Stopbutton->setText("Play");
}

void MainWindow::handle_Capturebutton()
{
    QString time_format = "yyMMdd_HH-mm-ss_";
    QDateTime a = QDateTime::currentDateTime();

    QString str = a.toString(time_format) +
                  QString("Amp_%1 V_").arg(amplitude()) +
                  QString("Freq_%1 Hz").arg(frequency());

    m_captureListWidget->addItem(str);
    m_plot->getplotScreenShot(800, 600);
}

void MainWindow::handle_ItemDoubleClicked()
{
    int index_vectorstring = m_captureListWidget->currentRow();
    QString filename = m_captureListWidget->item(index_vectorstring)->text() + ".png";

    ImageWidget* ImageView = new ImageWidget();
    ImageView->loadfileImage(m_plot->getimagefromqbuffer(index_vectorstring), filename);
    ImageView->show();
}

#include "moc_MainWindow.cpp"
