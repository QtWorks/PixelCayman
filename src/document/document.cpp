/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \section License
 *
 * Copyright (C) 2015 Mattia Basaglia
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
 *
 */

#include "document.hpp"
#include "visitor.hpp"
#include <QFileInfo>

namespace document {

Document::Document(const QSize& size,
                   const QString& file_name,
                   const Metadata& metadata)
    : DocumentElement(metadata),
      image_size(size),
      file_name(file_name)
{
    Layer* layer = new Layer(this, QObject::tr("Layer"));
    layer->addFrameImage();
    addLayer(layer);
}

Document::Document(const QImage& image, const QString& file_name)
    : image_size(image.size()),
      file_name(file_name)
{
    /// \todo Read exif metadata (maybe with Exiv2?)
    Layer* layer = new Layer(this, QFileInfo(file_name).baseName());
    layer->addFrameImage(image);
    addLayer(layer);
}

Document::~Document()
{
    // layers/animations are cleaned uoìp by QObject
}

QString Document::fileName() const
{
    return file_name;
}

void Document::setFileName(const QString& file_name)
{
    this->file_name = file_name;
}

QSize Document::imageSize() const
{
    return image_size;
}

void Document::apply(Visitor& visitor)
{
    if ( visitor.enter(*this) )
    {
        for ( auto layer : layers_ )
            layer->apply(visitor);

        for ( auto anim : animations_ )
            anim->apply(visitor);

        visitor.leave(*this);
    }
}

Document* Document::parentDocument() const
{
    return const_cast<Document*>(this);
}

QList<Layer*> Document::layers()
{
    return layers_;
}

void Document::addLayer(Layer* layer)
{
    if ( layer->owner_ != this )
    {
        layer->owner_ = this;
        registerElement(layer);
    }
    layers_.append(layer);
}

} // namespace document
