/* 
 * Cfgparser - configuration reader C++ library
 * Copyright (c) 2008, Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Seznam.cz, a.s.
 * Radlicka 2, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:cfgparser@firma.seznam.cz
 * 
 * $Id: $
 *
 * PROJECT
 * Configuration file parser
 *
 * DESCRIPTION
 * Config parser wrapper for easier access.
 *
 * AUTHOR
 * Vasek Blazek <blazek@firma.seznam.cz>
 *
 */


#include <ctype.h>

#include <string>
#include <algorithm>
#include <iostream>

#include "configwrapper.h"

using std::string;

void ConfigWrapper_t::getString(
        const string &section, const string &option, string &value) const {

    string tmp;
    if (config.getValue(section, option, &tmp)) {
        value = tmp;
    } else {
        std::cerr <<"Option [" << section.c_str() << "]::" << option.c_str() << " not given. "
            << "Using default '" << value.c_str() << "'." << std::endl;
    }
}

void ConfigWrapper_t::getInt(
        const string &section,
        const string &option, unsigned int &value) const {

    int tmp;
    if (config.getValue(section, option, &tmp)) {
        if (tmp < 0)
            std::cerr << "Negative value of [" << section.c_str() << "]::" << option.c_str() << ". "
                 << "Using default " << value << "." << std::endl;
        else value = tmp;
    } else {
        std::cerr << "Option [" << section.c_str() << "]::" << option.c_str() << " not given or not integer. "
             << "Using default " << value << "."  << std::endl;
    }
}

void ConfigWrapper_t::getInt(
        const string &section, const string &option, int &value) const {

    int tmp;
    if (config.getValue(section, option, &tmp)) {
        value = tmp;
    } else {
        std::cerr << "Option [" << section.c_str() << "]::" << option.c_str() << " not given or not integer. "
             << "Using default " << value << "." << std::endl;
    }
}

void ConfigWrapper_t::getTime(
        const string &section, const string &option, time_t &value) const {

    int tmp;
    if (config.getValue(section, option, &tmp)) {
        if (tmp < 0)
            std::cerr << "Negative value of [" << section.c_str() << "]::" << option.c_str() << ". "
                "Using default " << value << "." << std::endl;
        else value = tmp;
    } else {
        std::cerr << "Option [" << section.c_str() << "]::" << option.c_str() << " not given or not integer. "
            "Using default " << value << "." << std::endl;
    }
}

void ConfigWrapper_t::getFloat(
        const string &section, const string &option, float &value) const {

    double tmp;
    if (config.getValue(section, option, &tmp)) {
        if (tmp <= 0)
            std::cerr << "Negative value of [" << section.c_str() << "]::" << option.c_str() << ". "
                 << "Using default " << value << std::endl;
        else value = tmp;
    } else {
        std::cerr << "Option [" << section.c_str() << "]::" << option.c_str() << " not given or not integer. "
             << "Using default " << value << std::endl;
    }
}

void ConfigWrapper_t::getBool(
        const string &section, const string &option, bool &value) const {

    string tmp;
    if (config.getValue(section, option, &tmp)) {
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), toupper);
        if ((tmp == "TRUE") || (tmp == "YES")) value = true;
        else if ((tmp == "FALSE") || (tmp == "NO")) value = false;
        else {
            std::cerr << "Invalid value of boolean option [" << section.c_str() << "]::" << option.c_str() << ". "
                 << "Using default " << value << ". " << std::endl;
        }
    } else {
        std::cerr << "Option [" << section.c_str() << "]::" << option.c_str() << " not given. "
             << "Using default " << value << ". " << std::endl;
    }
}

struct timeval ConfigWrapper_t::getTimeval(
        const string &section, const string &option,
        unsigned int defaultValue, TimeType_t timeType) const {

    struct timeval tv;
    getInt(section, option, defaultValue);
    switch (timeType) {
    case TT_MILISECONDS:
        tv.tv_sec = defaultValue / 1000;
        tv.tv_usec = (defaultValue % 1000) * 1000;
        break;
    case TT_SECONDS:
    default:
        tv.tv_sec = defaultValue;
        tv.tv_usec = 0;
        break;
    }
    return tv;
}
