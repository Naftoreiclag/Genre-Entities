/*
 *  Copyright 2016-2017 James Fong
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "pegr/resource/Json_Util.hpp"

#include <fstream>

#include "pegr/logger/Logger.hpp"
#include "pegr/except/Except.hpp"

namespace pegr {
namespace Resour {
namespace Json_Util {

Json::Value read_commentated(boost::filesystem::path file) {
    Json::Value retval;
    std::ifstream fs(file.string());
    std::stringstream sss;
    std::string line;
    while (std::getline(fs, line)) {
        bool comment = false;
        for (char c : line) {
            if (c == ' ' || c == '\t') {
                continue;
            }
            if (c == '#') {
                comment = true;
            } else {
                break;
            }
        }
        if (comment) {
            sss << '\n';
        } else {
            sss << line << '\n';
        }
    }
    fs.close();
    try {
        sss >> retval;
    } catch (Json::RuntimeError e) {
        std::stringstream ess;
        ess << "Failed to parse JSON file, "
            << file << ": " << e.what();
        throw Except::Runtime(ess.str());
    }
    return retval;
}
Json::Value read(boost::filesystem::path file) {
    Json::Value retval;
    std::ifstream fs(file.string());
    try {
        fs >> retval;
    } catch (Json::RuntimeError e) {
        std::stringstream ess;
        ess << "Failed to parse JSON file, "
            << file << ": " << e.what();
        throw Except::Runtime(ess.str());
    }
    return retval;
}

void write(std::string filename, Json::Value& value, bool compact) {
    std::ofstream fs(filename.c_str());
    Json::FastWriter fast_writer;
    fs << fast_writer.write(value);
    fs.close();
}

std::string as_string(const Json::Value& val, const char* def) {
    if (val.isConvertibleTo(Json::ValueType::stringValue)) {
        return val.asString();
    }
    if (!def) {
        std::stringstream sss;
        sss << "Cannot convert JSON value to string: "
            << val.toStyledString();
        throw Except::Runtime(sss.str());
    }
    return def;
}

} // namespace Json_Util
} // namespace Resour
} // namespace pegr
