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
#ifndef PIXEL_CAYMAN_PLUGIN_HPP
#define PIXEL_CAYMAN_PLUGIN_HPP

#include <QStringList>
#include <QFileInfo>
#include <QHash>
#include <QObject>

namespace plugin {

/**
 * \brief Interface for plugin types
 */
class Plugin : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(int version READ version)

public:
    /**
     * \brief Plugin dependency descriptor
     */
    struct Dependency
    {
        /**
         * \brief Name of the plugin that is required to be loaded
         */
        QString name;

        /**
         * \brief Minimum version.
         *
         * Zero means no minimum version
         */
        int minimum_version = 0;

        /**
         * \brief Maximum version.
         *
         * Zero means no maximum version
         */
        int maximum_version = 0;
    };

    virtual ~Plugin() {}

    /**
     * \brief Loads the plugin functionality to make it ready for use
     * \returns \b true on success
     */
    bool load()
    {
        if ( loaded_ )
            return false;

        loaded_ = onLoad();

        if ( loaded_ )
            emit loaded(QPrivateSignal());

        return loaded_;
    }

    /**
     * \brief Removes the plugin functionality to disable the plugin
     */
    void unload()
    {
        if ( loaded_ )
        {
            loaded_ = false;
            onUnload();
            emit unloaded(QPrivateSignal());
        }
    }

    /**
     * \brief Whether the plugin has been loaded
     */
    bool isLoaded()
    {
        return loaded_;
    }

    /**
     * \brief Returns the plugin name
     */
    QString name()
    {
        if ( name_.isEmpty() )
            name_ = onName();
        return name_;
    }

    /**
     * \brief Plugin version number
     */
    int version()
    {
        if ( version_ < 0 )
            version_ = onVersion();
        return version_;
    }

    /**
     * \brief List of dependencies to other plugins
     */
    QList<Dependency> dependencies()
    {
        if ( !dependencies_loaded_ )
        {
            dependencies_loaded_ = true;
            dependencies_ = onDependencies();
        }

        return dependencies_;
    }

signals:
    void loaded(QPrivateSignal);
    void unloaded(QPrivateSignal);

protected:
    /**
     * \brief Loads the plugin functionality to make it ready for use
     * \returns \b true on success
     */
    virtual bool onLoad() = 0;

    /**
     * \brief Removes the plugin functionality to disable the plugin
     */
    virtual void onUnload() {}

    /**
     * \brief Returns the plugin name
     *
     * Must be unique across all plugins and not empty
     */
    virtual QString onName() = 0;

    /**
     * \brief Plugin version number
     */
    virtual int onVersion() { return 0; }

    /**
     * \brief List of dependencies to other plugins
     */
    virtual QList<Dependency> onDependencies() { return {}; }

private:
    QString name_;
    bool loaded_ = false;
    int version_ = -1;
    bool dependencies_loaded_ = false;
    QList<Dependency> dependencies_;
};

/**
 * \brief Interface for classes that can create plugin objects from files
 */
class PluginFactory
{
public:
    virtual ~PluginFactory() {}

    /**
     * \brief Returns whether \p file is in a format that the plugin factory can handle
     */
    virtual bool canCreate(const QFileInfo& file) const = 0;

    /**
     * \brief Creates a plugin object from file
     */
    virtual Plugin* create(const QString& fileName) = 0;
};

/**
 * \brief Class that handles plugins
 * \todo Functionality to define which plugins are to be loaded in the config
 */
class PluginRegistry : public QObject
{
    Q_OBJECT

public:
    static PluginRegistry& instance()
    {
        static PluginRegistry singleton;
        return singleton;
    }

    /**
     * \brief The list of available plugins
     */
    QList<Plugin*> plugins() const
    {
        return plugins_.values();
    }

    /**
     * \brief The plugin with the given name
     */
    Plugin* plugin(const QString& name) const
    {
        return plugins_.value(name);
    }

    /**
     * \brief Whether there is a loaded plugin that meets the dependency
     */
    bool meetsDependency(const Plugin::Dependency& dependency)
    {
        if ( auto plugin = plugins_.value(dependency.name) )
        {
            int version = plugin->version();
            if ( dependency.minimum_version && dependency.minimum_version > version )
                return false;
            if ( dependency.maximum_version && dependency.maximum_version < version )
                return false;
            return true;
        }
        return false;
    }

    /**
     * \brief Whether all the dependencies in the list are met
     */
    bool meetsDependency(const QList<Plugin::Dependency>& dependencies)
    {
        for ( const auto& dep : dependencies )
            if ( ! meetsDependency(dep) )
                return false;
        return true;
    }

    /**
     * \brief Adds a plugin factory
     *
     * Takes ownership of the factory object
     */
    void addFactory(PluginFactory* factory)
    {
        factories_.push_back(factory);
    }

    /**
     * \brief Creates the plugin objects and loads them
     */
    void load();

    /**
     * \brief Unloads all plugins and removes them
     */
    void unload();

    /**
     * \brief To search plugins into
     */
    QStringList searchPaths() const
    {
        return search_paths_;
    }

    void setSearchPaths(const QStringList& search_paths)
    {
        search_paths_ = search_paths;
    }

    void addSearchPath(const QString& path)
    {
        search_paths_.push_back(path);
    }

signals:
    /**
     * \brief Emitted when a plugin has been created
     */
    void created(Plugin* plugin);
    /**
     * \brief Emitted when a plugin is being destroyed
     */
    void destroyed(Plugin* plugin);
    /**
     * \brief Emitted when a plugin has been enabled
     */
    void loaded(Plugin* plugin);
    /**
     * \brief Emitted when a plugin has been disabled
     */
    void unloaded(Plugin* plugin);

    /**
     * \brief Emitted on minor issues with the plugins
     */
    void warning(const QString& msg);

private:
    PluginRegistry() {}

    ~PluginRegistry()
    {
        for ( auto factory : factories_ )
            delete factory;
    }

    /**
     * \brief Queue a plugin file for loading
     */
    bool queue(const QFileInfo& file);

    void addPlugin(Plugin* plugin);
    void removePlugin(Plugin* plugin);

    QHash<QString, Plugin*> plugins_;
    QList<Plugin*>          queued_;
    QList<PluginFactory*>   factories_;
    QStringList             search_paths_;
};

} // namespace plugin
#endif // PIXEL_CAYMAN_PLUGIN_HPP
