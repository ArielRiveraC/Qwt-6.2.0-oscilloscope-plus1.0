/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "Plot.h"
#include "SignalData.h"
#include "WheelBox.h"

#include <QwtPlotGrid>
#include <QwtPlotLayout>
#include <QwtPlotCanvas>
#include <QwtPlotMarker>
#include <QwtPlotLegendItem>
#include <QwtPlotCurve>
#include <QwtScaleDiv>
#include <QwtScaleMap>
#include <QwtPlotDirectPainter>
#include <QwtPlotRenderer>
#include <QwtPainter>
#include <QPainter>
#include <QEvent>

namespace
{
    class LegendItem : public QwtPlotLegendItem
    {
      public:
        LegendItem()
        {
            setRenderHint( QwtPlotItem::RenderAntialiased );

            const QColor c1( Qt::white );

            setTextPen( c1 );
            setBorderPen( c1 );

            QColor c2( Qt::gray );
            c2.setAlpha( 200 );

            setBackgroundBrush( c2 );
        }
    };

    class Canvas : public QwtPlotCanvas
    {
      public:
        Canvas( QwtPlot* plot = NULL )
            : QwtPlotCanvas( plot )
        {
            /*
                The backing store is important, when working with widget
                overlays ( f.e rubberbands for zooming ).
                Here we don't have them and the internal
                backing store of QWidget is good enough.
             */

            setPaintAttribute( QwtPlotCanvas::BackingStore, false );
            setBorderRadius( 10 );

            if ( QwtPainter::isX11GraphicsSystem() )
            {
#if QT_VERSION < 0x050000
                /*
                    Qt::WA_PaintOutsidePaintEvent works on X11 and has a
                    nice effect on the performance.
                 */

                setAttribute( Qt::WA_PaintOutsidePaintEvent, true );
#endif

                /*
                    Disabling the backing store of Qt improves the performance
                    for the direct painter even more, but the canvas becomes
                    a native window of the window system, receiving paint events
                    for resize and expose operations. Those might be expensive
                    when there are many points and the backing store of
                    the canvas is disabled. So in this application
                    we better don't disable both backing stores.
                 */

                if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) )
                {
                    setAttribute( Qt::WA_PaintOnScreen, true );
                    setAttribute( Qt::WA_NoSystemBackground, true );
                }
            }

            setupPalette();
        }

      private:
        void setupPalette()
        {
            QPalette pal = palette();

            QLinearGradient gradient;
            gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
            gradient.setColorAt( 0.0, QColor( 0, 49, 110 ) );
            gradient.setColorAt( 1.0, QColor( 0, 87, 174 ) );

            pal.setBrush( QPalette::Window, QBrush( gradient ) );

            // QPalette::WindowText is used for the curve color
            pal.setColor( QPalette::WindowText, Qt::green );

            setPalette( pal );
        }
    };

    class CurveData : public QwtSeriesData< QPointF >
    {
      public:
        const SignalData& values() const
        {
            return SignalData::instance();
        }

        SignalData& values()
        {
            return SignalData::instance();
        }

        virtual QPointF sample( size_t index ) const QWT_OVERRIDE
        {
            return SignalData::instance().value( index );
        }

        virtual size_t size() const QWT_OVERRIDE
        {
            return SignalData::instance().size();
        }

        virtual QRectF boundingRect() const QWT_OVERRIDE
        {
            return SignalData::instance().boundingRect();
        }
    };
}

Plot::Plot( QWidget* parent )
    : QwtPlot( parent )
    , m_paintedPoints( 0 )
    , m_interval( 0.0, 10.0 )
    , m_timerId( -1 )
    , m_legendItem( NULL )
    , m_isDirty( false )
{
    m_directPainter = new QwtPlotDirectPainter();

    setAutoReplot( false );
    setCanvas( new Canvas() );

    plotLayout()->setAlignCanvasToScales( true );

    setAxisTitle( QwtAxis::XBottom, "Time [s]" );
    setAxisScale( QwtAxis::XBottom, m_interval.minValue(), m_interval.maxValue() );
    setAxisScale( QwtAxis::YLeft, -200.0, 200.0 );

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach( this );

    m_origin = new QwtPlotMarker();
    m_origin->setLineStyle( QwtPlotMarker::Cross );
    m_origin->setValue( m_interval.minValue() + m_interval.width() / 2.0, 0.0 );
    m_origin->setLinePen( Qt::gray, 0.0, Qt::DashLine );
    m_origin->attach( this );

    m_qvectorqbuffer = new QVector<QBuffer*>(0);

    m_levelWheel = new WheelBox( "Zero Level", -400.0, 0.0, 1.0 );
    m_levelWheel->setValue( -200.0 );

    connect( m_levelWheel, SIGNAL(valueChanged(double)),
                           SLOT(scrollLeftAxis(double)) );

    m_curve = new QwtPlotCurve();
    m_curve->setStyle( QwtPlotCurve::Lines );
    m_curve->setPen( canvas()->palette().color( QPalette::WindowText ) );
    m_curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    m_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
    m_curve->setData( new CurveData() );
    m_curve->attach( this );
}

Plot::~Plot()
{
    delete m_directPainter;
}

WheelBox* Plot::getLevelWheel()
{
   return m_levelWheel;
}

void Plot::start()
{
    m_elapsedTimer.start();
    m_timerId = startTimer( 10 );
}

void Plot::replot()
{
    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().lock();

    QwtPlot::replot();
    m_paintedPoints = curveData->size();

    curveData->values().unlock();
}

void Plot::updateItemLegend(bool legendItem_checkState)
{
    m_isDirty = false;
    setAutoReplot( true );

    if (legendItem_checkState == true)
    {
        if ( m_legendItem == NULL )
        {
            m_legendItem = new LegendItem();
            m_legendItem->attach( this );
        }

        m_legendItem->setMaxColumns(2);
        m_legendItem->setAlignmentInCanvas( Qt::Alignment( Qt::AlignRight | Qt::AlignTop ) );
        m_legendItem->setBackgroundMode( QwtPlotLegendItem::BackgroundMode( 0 ) );

        if ( m_legendItem->backgroundMode() == QwtPlotLegendItem::ItemBackground )
        {
            m_legendItem->setBorderRadius( 4 );
            m_legendItem->setMargin( 0 );
            m_legendItem->setSpacing( 4 );
            m_legendItem->setItemMargin( 2 );
        }
        else
        {
            m_legendItem->setBorderRadius( 8 );
            m_legendItem->setMargin( 4 );
            m_legendItem->setSpacing( 2 );
            m_legendItem->setItemMargin( 0 );
        }

        QFont font = m_legendItem->font();
        font.setPointSize(12);
        m_legendItem->setFont( font );
    }
    else
    {
        delete m_legendItem;
        m_legendItem = NULL;
    }

    setAutoReplot( false );
    if ( m_isDirty )
    {
        m_isDirty = false;
        replot();
    }
}

void Plot::setCurveTitle(QString TextTitle)
{
     Plot::m_curve->setTitle(TextTitle);
     replot();
}

void Plot::setIntervalLength( double interval )
{
    if ( interval > 0.0 && interval != m_interval.width() )
    {
        m_interval.setMaxValue( m_interval.minValue() + interval );
        setAxisScale( QwtAxis::XBottom,
            m_interval.minValue(), m_interval.maxValue() );

        replot();
    }
}

void Plot::setColorCurve(QColor color)
{
     Plot::m_curve->setPen(color);
}

bool Plot::setStartTimer()
{
    if (m_timerId == -1){
        m_timerId = startTimer( 10 );
        return true;
    }
    else {       
        m_timerId = -1;
        return false;
    }
}

void Plot::updateCurve()
{
    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().lock();

    const int numPoints = curveData->size();
    if ( numPoints > m_paintedPoints )
    {
        const bool doClip = !canvas()->testAttribute( Qt::WA_PaintOnScreen );
        if ( doClip )
        {
            /*
                Depending on the platform setting a clip might be an important
                performance issue. F.e. for Qt Embedded this reduces the
                part of the backing store that has to be copied out - maybe
                to an unaccelerated frame buffer device.
             */

            const QwtScaleMap xMap = canvasMap( m_curve->xAxis() );
            const QwtScaleMap yMap = canvasMap( m_curve->yAxis() );

            QRectF br = qwtBoundingRect( *curveData,
                m_paintedPoints - 1, numPoints - 1 );

            const QRect clipRect = QwtScaleMap::transform( xMap, yMap, br ).toRect();
            m_directPainter->setClipRegion( clipRect );
        }

        m_directPainter->drawSeries( m_curve,
            m_paintedPoints - 1, numPoints - 1 );
        m_paintedPoints = numPoints;
    }

    curveData->values().unlock();
}

void Plot::incrementInterval()
{
    m_interval = QwtInterval( m_interval.maxValue(),
        m_interval.maxValue() + m_interval.width() );

    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().clearStaleValues( m_interval.minValue() );

    // To avoid, that the grid is jumping, we disable
    // the autocalculation of the ticks and shift them
    // manually instead.

    QwtScaleDiv scaleDiv = axisScaleDiv( QwtAxis::XBottom );
    scaleDiv.setInterval( m_interval );

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        QList< double > ticks = scaleDiv.ticks( i );
        for ( int j = 0; j < ticks.size(); j++ )
            ticks[j] += m_interval.width();
        scaleDiv.setTicks( i, ticks );
    }
    setAxisScaleDiv( QwtAxis::XBottom, scaleDiv );

    m_origin->setValue( m_interval.minValue() + m_interval.width() / 2.0, 0.0 );

    m_paintedPoints = 0;
    replot();
}

void Plot::timerEvent( QTimerEvent* event )
{
    if ( event->timerId() == m_timerId )
    {
        updateCurve();

        const double elapsed = m_elapsedTimer.elapsed() / 1e3;
        if ( elapsed > m_interval.maxValue() )
            incrementInterval();

        return;
    }

    QwtPlot::timerEvent( event );
}

void Plot::resizeEvent( QResizeEvent* event )
{
    m_directPainter->reset();
    QwtPlot::resizeEvent( event );
}

void Plot::showEvent( QShowEvent* )
{
    replot();
}

void Plot::scrollLeftAxis( double value )
{
    const double range = axisScaleDiv( QwtAxis::YLeft ).range();
    setAxisScale( QwtAxis::YLeft, value, value + range );
    replot();
}

bool Plot::eventFilter( QObject* object, QEvent* event )
{
    if ( object == canvas() &&
        event->type() == QEvent::PaletteChange )
    {
        m_curve->setPen( canvas()->palette().color( QPalette::WindowText ) );
    }

    return QwtPlot::eventFilter( object, event );
}

void Plot::getplotScreenShot(int width, int height )
{
    QwtPlotRenderer renderer;
    QImage image( width, height, QImage::Format_ARGB32 );

    QPainter painter( &image );
    renderer.render(this, &painter, QRectF( 0, 0, width, height ) );
    painter.end();

    m_qbuffer = new QBuffer;

    if( image.save(m_qbuffer, "png") )
        m_qvectorqbuffer->append(m_qbuffer);
}

QBuffer* Plot::getimagefromqbuffer(int bufferindex)
{
    return m_qvectorqbuffer->at(bufferindex);
}

#include "moc_Plot.cpp"
