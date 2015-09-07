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
#ifndef PIXEL_CAYMAN_DOCUMENT_HPP
#define PIXEL_CAYMAN_DOCUMENT_HPP

#include "animation.hpp"
#include "layer.hpp"

namespace document {

/**
 * \brief Root of a document structure
 */
class Document : public DocumentElement
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)

public:
    explicit Document(const QSize& size = {},
                      const QString& file_name = {},
                      const Metadata& metadata = {});

    explicit Document(const QImage& image, const QString& file_name);

    ~Document();

    /**
     * \brief Name of the file associated with this document
     */
    QString fileName() const;
    void setFileName(const QString& file_name);

    /**
     * \brief Image size, must be consistent with the size of the layer images
     */
    QSize imageSize() const;

    /**
     * \brief Rect at (0,0) with imageSize
     */
    QRect imageRect() const
    {
        return QRect(QPoint(), imageSize());
    }

    /**
     * \brief Animations available in this document
     */
    QList<const Animation*> animations() const;
    QList<Animation*> animations();
    const Animation* animation(const QString & name) const;
    Animation* animation(const QString & name);
    Animation* addAnimation(const QString& name);
    void removeAnimation(Animation* animation);

    /**
     * \brief Document layers
     */
    QList<Layer*> layers();
    void addLayer(document::Layer* layer);

    void apply(Visitor& visitor) override;
    Document* parentDocument() const override;

    /**
     * \brief Parents the element to the document and ensures it has a proper name
     *
     * Template so it can be called from the object constructor
     */
    template<class T>
    void registerElement(T* element)
    {
        static_assert(std::is_base_of<DocumentElement,T>::value,
            "Document::registerElement Must be used with document elements");
        registerElement(element, T::staticMetaObject);
    }

    /**
     * \brief Polymorphic version
     */
    void registerElement(DocumentElement* element)
    {
        registerElement(element, *element->metaObject());
    }

signals:
    void fileNameChanged(const QString& fileName);

    /**
     * \brief Emitted on operations that changes the layer layout
     *
     * Eg: New layers added, re-ordered etc
     */
    void layersChanged();

private:
    void registerElement(DocumentElement* element, const QMetaObject& meta);

    QList<Layer*> layers_;
    QList<Animation*> animations_;
    QSize image_size;
    QString file_name;
};

} // namespace document
#endif // PIXEL_CAYMAN_DOCUMENT_HPP
