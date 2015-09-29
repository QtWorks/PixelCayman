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

#include <QUndoStack>
#include "animation.hpp"
#include "layer.hpp"
#include "format_settings.hpp"
#include "color_palette.hpp"

namespace document {

/**
 * \brief Root of a document structure
 */
class Document : public LayerContainer
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)

    /**
     * \brief Palette for the document
     *
     * Has an effect only if indexedColors is true
     */
    Q_PROPERTY(color_widgets::ColorPalette palette READ palette NOTIFY paletteChanged)

    /**
     * \brief Whether the document should be using indexed colors
     *
     * The colors are defined by palette
     */
    Q_PROPERTY(bool indexedColors READ indexedColors WRITE setIndexedColors NOTIFY indexedColorsChanged)

public:
    /**
     * \brief Build a document with a single layer of the given size
     */
    explicit Document(const QSize& size,
                      const QString& file_name = {},
                      const QColor& background = Qt::transparent,
                      const Metadata& metadata = {});

    /**
     * \brief Build a document with a single layer with the given image
     */
    explicit Document(const QImage& image, const QString& file_name);

    /**
     * \brief Build a document with no layers and an invalid size
     */
    Document(const Metadata& metadata = {});

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
     * \brief Set the image size
     * \note It doesn't resize the layers, only set this if you can keep everything
     * consistent
     */
    void setImageSize(const QSize& size);

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

    /**
     * \brief Registers an element belonging to a different document
     * \note Only use if you don't you can keep the edit history consistent
     *       and the owner document is destroyed soon after this call
     */
    void stealElement(DocumentElement* element);

    /**
     * \brief Stack with the commands used to edit this document
     */
    const QUndoStack& undoStack() const;
    QUndoStack& undoStack();

    /**
     * \brief Add a command to the document
     */
    void pushCommand(QUndoCommand* command);

    FormatSettings& formatSettings()
    {
        return format_settings;
    }

    const FormatSettings& formatSettings() const
    {
        return format_settings;
    }

    color_widgets::ColorPalette& palette();
    const color_widgets::ColorPalette& palette() const;
    /**
     * \brief Cached color table
     */
    const QVector<QRgb>& colorTable() const
    {
        return color_table;
    }

    bool indexedColors() const;
    void setIndexedColors(bool uses_palette);

signals:
    void fileNameChanged(const QString& fileName);
    void indexedColorsChanged(bool indexedColors);
    void paletteChanged(const color_widgets::ColorPalette& palette);

protected:
    void onInsertLayer(Layer* layer) override;
    void onRemoveLayer(Layer* layer) override;

private:
    void registerElement(DocumentElement* element, const QMetaObject& meta);

    QList<Animation*>   animations_;
    QSize               image_size;
    QString             file_name;
    QUndoStack          undo_stack;
    FormatSettings      format_settings;
    color_widgets::ColorPalette palette_;
    bool                indexed_colors = false;
    QVector<QRgb>       color_table;
};

} // namespace document
#endif // PIXEL_CAYMAN_DOCUMENT_HPP
