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
#ifndef PIXEl_CAYMAN_DOCUMENT_IO_HPP
#define PIXEl_CAYMAN_DOCUMENT_IO_HPP
#include "visitor.hpp"
#include <QXmlStreamWriter>
#include <QFile>
#include <QBuffer>

namespace document {

namespace visitor {

/**
 * \brief Visitor that recursively writes XML on a IODevice
 */
class SaverXml : public Visitor
{
public:
    explicit SaverXml(QIODevice* output);
    ~SaverXml();

    bool enter(Document& document) override;
    void leave(Document& document) override;
    bool enter(Layer& layer) override;
    void leave(Layer& layer) override;
    void visit(Image& image) override;
    bool enter(Animation& animation) override;
    void leave(Animation& animation) override;
    void visit(Frame& frame) override;

private:
    void writeMetadata(const Metadata& data);
    void writeId(const DocumentElement& element, const QString& attr = "id");
    QXmlStreamWriter writer;
};

} // namespace visitor

/**
 * \brief A document file format
 * \todo Mime types?
 */
class AbstractFormat
{
public:
    virtual ~AbstractFormat(){}

    /**
     * \brief Unique machine-readable identifier for the file format
     */
    virtual QString id() const = 0;
    /**
     * \brief A human-readable name of the file format shown in the file dialog
     */
    virtual QString name() const { return id(); }
    /**
     * \brief List of file extensions to filter in the file dialog
     */
    virtual QStringList saveExtensions() const { return {id()}; }
    /**
     * \brief List of file extensions to filter in the file dialog
     */
    virtual QStringList openExtensions() const { return saveExtensions(); }
    /**
     * \brief Whether the file format supports saving document
     */
    virtual bool canSave() const { return false; }
    /**
     * \brief Save the document contents to the output device
     * \return \b true on success
     */
    virtual bool save(Document* input, QIODevice* device)  { return false; }
    /**
     * \brief Save the document to a file with the given name
     * \return \b true on success
     */
    bool save(Document* document, const QString& filename);
    /**
     * \brief Save the document to the file stored in its filename
     * \return \b true on success
     */
    bool save(Document* document);
    /**
     * \brief Whether the file format supports opening documents
     */
    virtual bool canOpen() const { return false; }
    /**
     * \brief Load the device contents in a new document
     * \return A new Document object, \b nullptr on error
     */
    virtual Document* open(QIODevice* device) { return nullptr; }
    /**
     * \brief Load the file contents in a new document
     * \return A new Document object, \b nullptr on error
     */
    Document* open(const QString& filename);


protected:
    /**
     * \brief Extract a file name from a i/o device
     */
    static QString fileName(const QIODevice* device)
    {
        if ( const QFile* file = qobject_cast<const QFile*>(device) )
            return file->fileName();
        return QObject::tr("Image");
    }
};

/**
 * \brief Writes a .mela file
 * \todo Read files
 */
class FormatXmlMela : public AbstractFormat
{
public:
    QString id() const override { return "mela"; }
    QString name() const override { return QObject::tr("Cayman Files"); }
    bool canSave() const override { return true; }
    bool save(Document* input, QIODevice* device) override
    {
        visitor::SaverXml xml(device);
        input->apply(xml);
        return true;
    }
};

/**
 * \brief Reads and writes bitmap images with the Qt image reader/writer
 *
 * Can be used as base class for formats that need to render the file to a
 * bitmap before saving it to a file
 */
class FormatBitmap : public AbstractFormat
{
public:
    QString id() const override;
    QString name() const override;
    QStringList saveExtensions() const override;
    QStringList openExtensions() const override;
    bool canSave() const override { return true; }
    bool canOpen() const override { return true; }

    bool save(document::Document* input, QIODevice* device) final;
    Document* open(QIODevice* device) final;

protected:
    /**
     * \brief Format used by for the QImage generated from the document
     */
    virtual QImage::Format imageFormat(const Document* input, const QIODevice* device) const;

    /**
     * \brief The color used to fill the QImage generated from the document
     */
    virtual QColor fillColor(const Document* input, const QIODevice* device) const;

    /**
     * \brief Saves the image to the device
     */
    virtual bool saveImage(const QImage& img, QIODevice* device);

    /**
     * \brief Opens an image from the device
     */
    virtual QImage openImage(QIODevice* device);
};

/**
 * \brief Keeps track of file formats
 */
class Formats
{
public:
    /**
     * \brief Singleton instance
     */
    static Formats& instance()
    {
        static Formats singleton;
        return singleton;
    }

    /**
     * \brief Registers a format
     *
     * Takes ownership of \p format
     *
     * \returns \b true on success
     *
     * \note If the function fails (because a format with the same id is found)
     * \p format will be destroyed
     */
    bool addFormat(AbstractFormat* format);

    /**
     * \brief Removes and deletes a format
     *
     * \p format is destroyed if successful
     *
     * \returns \b true on success
     */
    bool deleteFormat(AbstractFormat* format);

    /**
     * \brief Returns a format by id
     */
    AbstractFormat* format(const QString& id) const;

    /**
     * \brief List of available formats
     */
    QList<AbstractFormat*> formats() const;

    /**
     * \brief Save \p document with the format with the matching id
     */
    template<class... Args>
        bool save(const QString& format_id, Document* document, Args&&... args)
    {
        if ( auto fmt = format(format_id) )
            return fmt->save(document, std::forward<Args>(args)...);
        return false;
    }

    /**
     * \brief Open a document with the format with the matching id
     */
    template<class... Args>
        Document* open(const QString& format_id, Args&&... args)
    {
        if ( auto fmt = format(format_id) )
            return fmt->open(std::forward<Args>(args)...);
        return nullptr;
    }

private:
    Formats(){}
    ~Formats();

    QList<AbstractFormat*> formats_;
};

} // namespace document
#endif // PIXEl_CAYMAN_DOCUMENT_IO_HPP
