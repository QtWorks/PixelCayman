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
 *
 */

#include "layer_container.hpp"
#include "document.hpp"

namespace document {

LayerContainer::LayerContainer(const Metadata& metadata)
    : DocumentElement(metadata)
{
    connect(this, &LayerContainer::layerAdded,   this, &DocumentElement::edited);
    connect(this, &LayerContainer::layerRemoved, this, &DocumentElement::edited);
}

void LayerContainer::apply(Visitor& visitor)
{
    for ( auto layer : layers_ )
        layer->apply(visitor);
}

void LayerContainer::insertLayer(document::Layer* layer, int index)
{
    parentDocument()->pushCommand(new command::AddLayer(
        tr("Add Layer"), this, layer, index
    ));
}

void LayerContainer::insertLayerRaw(Layer* layer, int index)
{
    if ( layer->parentDocument() != parentDocument() )
        parentDocument()->registerElement(layer);

    onInsertLayer(layer);

    if ( index < 0 || index >= layers_.size() )
        layers_.append(layer);
    else
        layers_.insert(index, layer);

    connect(layer, &LayerContainer::layerAdded, this, &LayerContainer::layerAdded);
    connect(layer, &LayerContainer::layerRemoved, this, &LayerContainer::layerRemoved);

    emit layerAdded(layer, this, index);
}

Layer* LayerContainer::layer(int index)
{
    return index < 0 || index >= layers_.size() ? nullptr : layers_[index];
}

bool LayerContainer::removeLayer(Layer* layer)
{
    int index = layerIndex(layer);

    if ( index < 0 || index > layers_.size() )
        return false;

    parentDocument()->pushCommand(new command::RemoveLayer(
        tr("Remove Layer"), this, layer, index
    ));

    return true;
}

bool LayerContainer::removeLayerRaw(Layer* layer)
{
    int index = layerIndex(layer);

    if ( index < 0 || index > layers_.size() )
        return false;

    layers_.removeAt(index);

    onRemoveLayer(layer);

    disconnect(layer, &LayerContainer::layerAdded, this, &LayerContainer::layerAdded);
    disconnect(layer, &LayerContainer::layerRemoved, this, &LayerContainer::layerRemoved);

    emit layerRemoved(layer, this, index);

    return true;
}

int LayerContainer::layerIndex(Layer* layer) const
{
    return layers_.indexOf(layer);
}

int LayerContainer::countLayers() const
{
    return layers_.size();
}

} // namespace document
