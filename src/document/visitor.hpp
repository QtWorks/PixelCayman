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
#ifndef PIXEL_CAYMAN_DOCUMENT_VISITOR_HPP
#define PIXEL_CAYMAN_DOCUMENT_VISITOR_HPP

#include "document.hpp"

namespace document {

/**
 * \brief Visitor for document elements
 */
class Visitor
{
public:
    virtual ~Visitor() {}

    /**
     * \brief Visit a generic element, makes the virtual call
     */
    void visit(DocumentElement& element)
    {
        element.apply(*this);
    }

    /**
     * \brief Begin processing a document
     *
     * \return \b true if the document should be processed
     *
     * If returns true, the document's children will be visited
     * and after that, leave() will be called for that document
     */
    virtual bool enter(Document& document) { return false; }
    /**
     * \brief Finish processing a document
     */
    virtual void leave(Document& document) {}

    /**
     * \brief Begin processing a layer
     *
     * \return \b true if the layer should be processed
     *
     * If returns true, the layer's children will be visited
     * and after that, leave() will be called for that layer
     */
    virtual bool enter(Layer& layer) { return false; }
    /**
     * \brief Finish processing a layer
     */
    virtual void leave(Layer& layer) {}

    /**
     * \brief Process an image
     */
    virtual void visit(Image& image) {}

    /**
     * \brief Begin processing an animation
     *
     * \return \b true if the animation should be processed
     *
     * If returns true, the animation's children will be visited
     * and after that, leave() will be called for that animation
     */
    virtual bool enter(Animation& animation)  { return false; }
    /**
     * \brief Finish processing an animation
     */
    virtual void leave(Animation& animation) {}

    /**
     * \brief Process a frame
     */
    virtual void visit(Frame& frame) {}
};

namespace visitor {

/**
 * \brief Abstract class to visit all images to render a single frame
 */
class FrameRenderer : public Visitor
{
public:
    explicit FrameRenderer(Frame* frame) : frame_(frame) {}

    bool enter(Document& document) override
    {
        return true;
    }

    bool enter(Layer& layer) override
    {
        return true;
    }

    void visit(Image& image) override
    {
        if ( image.frame() == frame_ )
            render(image);
    }

protected:
    virtual void render(Image& image) = 0;

private:
    Frame* frame_;
};

/**
 * \brief Visitor that draws a single frame on a painter
 */
class Paint : public FrameRenderer
{
public:
    Paint(Frame* frame, QPainter* painter, bool full_alpha = false)
        : FrameRenderer(frame), painter(painter), full_alpha(full_alpha)
    {}

    bool enter(Document& document) override
    {
        blend = painter->compositionMode();
        return true;
    }

    void leave(Document& document) override
    {
        painter->setCompositionMode(blend);
    }

    bool enter(Layer& layer) override
    {
        painter->setCompositionMode(layer.blendMode());
        return true;
    }

protected:
    void render(Image& image) override
    {
        if ( full_alpha )
            image.paint(*painter);
        else if ( image.layer()->visible() )
            image.paint(*painter, image.layer()->opacity());
    }

private:
    QPainter* painter;
    bool full_alpha;
    QPainter::CompositionMode blend;
};


/**
 * \brief Searches for a layer by name
 */
class FindLayer : public Visitor
{
public:
    explicit FindLayer(const QString& name) : name_(name) {}

    bool enter(Document& document) override
    {
        found_ = nullptr;
        return true;
    }

    bool enter(Layer& layer) override
    {
        if ( layer.name() == name_ )
            found_ = &layer;

        return !found_;
    }

    Layer* found() const
    {
        return found_;
    }

    static Layer* find(Document& document, const QString& name)
    {
        FindLayer vis(name);
        document.apply(vis);
        return vis.found_;
    }

private:
    QString name_;
    Layer*  found_ = nullptr;
};

} // namespace visitor

} // namespace document
#endif // PIXEL_CAYMAN_DOCUMENT_VISITOR_HPP
