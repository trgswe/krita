/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "VanishingPointAssistant.h"

#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>

#include <math.h>

VanishingPointAssistant::VanishingPointAssistant()
    : KisPaintingAssistant("vanishing point", i18n("Vanishing Point assistant"))
{
}

QPointF VanishingPointAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    //Q_ASSERT(handles().size() == 1 || handles().size() == 5);
    //code nicked from the perspective ruler.
    qreal dx = pt.x() - strokeBegin.x();
    qreal dy = pt.y() - strokeBegin.y();

    if (dx * dx + dy * dy < 4.0) {
        // allow some movement before snapping
        return strokeBegin;
    }

    //dbgKrita<<strokeBegin<< ", " <<*handles()[0];
    QLineF snapLine = QLineF(*handles()[0], strokeBegin);
    
    
    dx = snapLine.dx();
    dy = snapLine.dy();

    const qreal dx2 = dx * dx;
    const qreal dy2 = dy * dy;
    const qreal invsqrlen = 1.0 / (dx2 + dy2);

    QPointF r(dx2 * pt.x() + dy2 * snapLine.x1() + dx * dy * (pt.y() - snapLine.y1()),
              dx2 * snapLine.y1() + dy2 * pt.y() + dx * dy * (pt.x() - snapLine.x1()));

    r *= invsqrlen;
    return r;
}

QPointF VanishingPointAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin)
{
    return project(pt, strokeBegin);
}

void VanishingPointAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPointF delta(0,0);
    QPointF mousePos(0,0);
    QPointF endPoint(0,0);//this is the final point that the line is being extended to, we seek it just outside the view port//
    
    if (canvas){
        //simplest, cheapest way to get the mouse-position//
        mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
        m_canvas = canvas;
    }
    else {
        //...of course, you need to have access to a canvas-widget for that.//
        mousePos = QCursor::pos();//this'll give an offset//
        dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
    }
    


     // draw controls when we are not editing
     if (canvas->paintingAssistantsDecoration()->isEditingAssistants() == false) {

         if (isAssistantComplete() && isSnappingActive() && previewVisible == true) {
             //don't draw if invalid.
             QTransform initialTransform = converter->documentToWidgetTransform();
             QPointF startPoint = initialTransform.map(*handles()[0]);

             QLineF snapLine= QLineF(startPoint, mousePos);
             QRect viewport= gc.viewport();

             KisAlgebra2D::intersectLineRect(snapLine, viewport);

             QRect bounds= QRect(snapLine.p1().toPoint(), snapLine.p2().toPoint());

             QPainterPath path;

             if (bounds.contains(startPoint.toPoint())){
                 path.moveTo(startPoint);
                 path.lineTo(snapLine.p1());
             }
             else
             {
                 path.moveTo(snapLine.p1());
                 path.lineTo(snapLine.p2());
             }

             drawPreview(gc, path);//and we draw the preview.
         }


     }




    // draws a circle around the vanishing point node while editing
    if (canvas->paintingAssistantsDecoration()->isEditingAssistants()) {
        QTransform initialTransform = converter->documentToWidgetTransform();
        QPointF startPoint2 = initialTransform.map(*handles()[0]);

        QPainterPath pathCenter;

        QRectF ellipse = QRectF(QPointF(startPoint2.x() -15, startPoint2.y() -15), QSizeF(30, 30));;
        pathCenter.addEllipse(ellipse);
        drawPath(gc, pathCenter, isSnappingActive());
    }


    gc.restore();

    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);



}

void VanishingPointAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (!m_canvas || !isAssistantComplete()) {
        return;
    }

    if (assistantVisible == false ||   m_canvas->paintingAssistantsDecoration()->isEditingAssistants()) {
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();
    QPointF p0 = initialTransform.map(*handles()[0]);

    // draws an "X"
    QPainterPath path;
    path.moveTo(QPointF(p0.x() - 10.0, p0.y() - 10.0));
    path.lineTo(QPointF(p0.x() + 10.0, p0.y() + 10.0));

    path.moveTo(QPointF(p0.x() - 10.0, p0.y() + 10.0));
    path.lineTo(QPointF(p0.x() + 10.0, p0.y() - 10.0));


    drawPath(gc, path, isSnappingActive());
}

QPointF VanishingPointAssistant::buttonPosition() const
{
    return (*handles()[0]);
}

bool VanishingPointAssistant::isAssistantComplete() const
{
    return handles().size() > 0; // only need one point to be ready
}


VanishingPointAssistantFactory::VanishingPointAssistantFactory()
{
}

VanishingPointAssistantFactory::~VanishingPointAssistantFactory()
{
}

QString VanishingPointAssistantFactory::id() const
{
    return "vanishing point";
}

QString VanishingPointAssistantFactory::name() const
{
    return i18n("Vanishing Point");
}

KisPaintingAssistant* VanishingPointAssistantFactory::createPaintingAssistant() const
{
    return new VanishingPointAssistant;
}
