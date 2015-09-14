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
#ifndef PIXEL_CAYMAN_PLUGIN_API_HPP
#define PIXEL_CAYMAN_PLUGIN_API_HPP

#include "settings.hpp"
#include "plugin.hpp"

namespace plugin {

/**
 * \brief Returns a string representing a settings key for the plugin
 */
inline QString settingsKey(Plugin* plugin, const QString& key )
{
    return QString("plugins/%1/%2").arg(plugin->id()).arg(key);
}

/**
 * \brief Writes a plugin setting
 */
template<class T>
    void settingsPut(Plugin* plugin, const QString& key, T&& value)
    {
        settings::put(settingsKey(plugin, key), std::forward<T>(value));
    }

/**
 * \brief Reads a plugin setting
 */
template<class T>
    typename std::remove_reference<T>::type
        settingsGet(Plugin* plugin, const QString& key, T&& default_value = T())
    {
        return settings::get(settingsKey(plugin, key), std::forward<T>(default_value));
    }

/**
 * \brief Class that structures the required functions of a library plugin
 */
class CaymanPlugin : public plugin::Plugin
{
    Q_OBJECT
public:
    /**
     * \brief Writes a plugin setting
     */
    template<class T>
        void settingsPut(const QString& key, T&& value)
        {
            settings::put(plugin::settingsKey(this, key), std::forward<T>(value));
        }

    /**
     * \brief Reads a plugin setting
     */
    template<class T>
        typename std::remove_reference<T>::type
            settingsGet(const QString& key, T&& default_value = T())
        {
            return settings::get(plugin::settingsKey(this, key), std::forward<T>(default_value));
        }
};

#define MACRO_TO_STRING_IMPL(x) #x
#define MACRO_TO_STRING(x) MACRO_TO_STRING_IMPL(x)
#define PLUGIN_INIT_FUNCTION PixelCaymanPlugin_init
#define PLUGIN_INIT_FUNCTION_STRING MACRO_TO_STRING(PLUGIN_INIT_FUNCTION)

} // namespace plugin
#endif // PIXEL_CAYMAN_PLUGIN_API_HPP
