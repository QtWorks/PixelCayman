/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2015 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PIXEL_CAYMAN_VIEW_GRAPHICS_WIDGET_HPP
#define PIXEL_CAYMAN_VIEW_GRAPHICS_WIDGET_HPP

#include <QGraphicsView>
#include "graphics_item.hpp"

namespace tool {
class Tool;
} // namespace tool

namespace view {

class GraphicsWidget : public QGraphicsView
{
    Q_OBJECT

    /**
     * \brief Zooming factor of the view (1 = real size)
     */
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged)

    /**
     * \brief Primary color used by tools
     */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    /**
     * \brief The currently active layer
     *
     * The active layer can be used by tools to perform editing operations
     * (after checking that the layer itself isn't locked).
     *
     * As long as there is at leas one layer in the document,
     * the view will have an active layer.
     *
     * If there is no layer, it returns \b nullptr.
     */
    Q_PROPERTY(::document::Layer* activeLayer READ activeLayer WRITE setActiveLayer NOTIFY activeLayerChanged)

public:
    explicit GraphicsWidget(::document::Document* document);
    ~GraphicsWidget();

    ::document::Document* document() const;

    qreal zoomFactor() const;

    using QGraphicsView::translate;

    /**
     * \brief The currently active tool (\b nullptr to have no active tool)
     */
    tool::Tool* currentTool() const;
    
    /**
     * \brief Changes the currently active tool
     *
     * Finalizes the old tool and initializes the new one
     */
    void setCurrentTool(::tool::Tool* tool);

    /**
     * \brief Converts a point from local coordinates to image coordinates
     */
    QPoint mapToImage(const QPoint& point);

    /**
     * \brief Converts a point from image coordinates to local coordinates
     */
    QPoint mapFromImage(const QPoint& point);

    QColor color() const;

    ::document::Layer* activeLayer() const;
    void setActiveLayer(::document::Layer* layer);

public slots:
    void setZoomFactor(qreal factor);
    void zoom(qreal factor);
    void translate(const QPointF& delta);
    void setColor(const QColor& color);

signals:
    void zoomFactorChanged(qreal zoomFactor);
    void colorChanged(const QColor& color);
    void activeLayerChanged(::document::Layer* activeLayer);

protected:
    void drawBackground(QPainter * painter, const QRectF & rect) override;
    void drawForeground(QPainter * painter, const QRectF & rect) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void fitSceneRect();

private:
    class Private;
    Private* p;
};

} // namespace view
#endif // PIXEL_CAYMAN_VIEW_GRAPHICS_WIDGET_HPP
