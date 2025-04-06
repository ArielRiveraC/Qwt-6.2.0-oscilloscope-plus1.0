/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QwtPlot>
#include <QVector>
#include <QBuffer>
#include <QDebug>
#include <QwtInterval>
#include <QElapsedTimer>

#include "Settings.h"
#include "MainWindow.h"

class WheelBox;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;
class QwtPlotLegendItem;

class Plot : public QwtPlot
{
    Q_OBJECT

  public:
    Plot( QWidget* = NULL );
    virtual ~Plot();

    void start();
    virtual void replot() QWT_OVERRIDE;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

    void setColorCurve(QColor color);
    void setCurveTitle(QString);
    bool setStartTimer();

  public Q_SLOTS:
    void setIntervalLength(double);
    void updateItemLegend(bool);
    void getplotScreenShot(int, int);
    QBuffer* getimagefromqbuffer(int);
    WheelBox* getLevelWheel();

  protected:
    virtual void showEvent( QShowEvent* ) QWT_OVERRIDE;
    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;
    virtual void timerEvent( QTimerEvent* ) QWT_OVERRIDE;

  Q_SIGNALS:
    void edited();

  private Q_SLOTS:
    void scrollLeftAxis( double );

  private:
    void updateCurve();
    void incrementInterval();

    QwtPlotCurve* m_curve;
    QwtPlotMarker* m_origin;
    WheelBox* m_levelWheel;
    QwtPlotLegendItem* m_legendItem;
    bool m_isDirty;

    QBuffer* m_qbuffer;
    QVector<QBuffer*>* m_qvectorqbuffer;

    int m_paintedPoints;

    QwtPlotDirectPainter* m_directPainter;

    QwtInterval m_interval;
    int m_timerId;

    QElapsedTimer m_elapsedTimer;
};
