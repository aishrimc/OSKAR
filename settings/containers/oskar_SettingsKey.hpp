/*
 * Copyright (c) 2014-2015, The University of Oxford
 * All rights reserved.
 *
 * This file is part of the OSKAR package.
 * Contact: oskar at oerc.ox.ac.uk
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSKAR_SETTINGS_KEY_HPP_
#define OSKAR_SETTINGS_KEY_HPP_

/**
 * @file oskar_SettingsKey.hpp
 */

#ifdef __cplusplus

#include <iostream>
#include <vector>
#include <string>

namespace oskar {

class SettingsKey
{
public:
    SettingsKey(char separator = '/');
    SettingsKey(const std::string& key, char separator = '/');
    ~SettingsKey();

    SettingsKey(const SettingsKey&);

    void set_separator(char s = '/');
    char separator() const;

    bool empty() const;
    int depth() const;
    int size() const;

    bool operator==(const SettingsKey& other) const;

    /* Implicit conversion operators. */
    operator std::string() const;
    std::string operator[](int i) const;

    std::string front() const;
    std::string back() const;
    std::string group() const;

    const char* c_str() const;

    friend std::ostream& operator<<(std::ostream& stream, const SettingsKey& k);

private:
    std::string key_;
    std::vector<std::string> tokens_;
    char sep_;
};

std::ostream& operator<< (std::ostream& stream, const SettingsKey& k);

} /* namespace oskar */

#endif /* __cplusplus */

#endif /* OSKAR_SETTINGS_KEY_HPP_ */