// This file is derived from xsonrpc Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
// This file is part of jsonrpc-lean, a c++11 JSON-RPC client/server library.
//
// Modifications and additions Copyright (C) 2015 Adriano Maia <tony@stark.im>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef JSONRPC_LEAN_UTIL_H
#define JSONRPC_LEAN_UTIL_H

#include <stdint.h>
#include <string>
#include <cassert>
#include <ctime>
#include <sstream>
#include <iomanip>

struct tm;

namespace {

    const char DATE_TIME_FORMAT[] = "%Y%m%dT%T";

    constexpr char BASE_64_ALPHABET[64 + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    constexpr int8_t BASE_64_LUT[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };

    inline constexpr char Base64Char0(uint8_t byte0) noexcept
    {
        return BASE_64_ALPHABET[byte0 >> 2];
    }
        inline constexpr char Base64Char1(uint8_t byte0, uint8_t byte1) noexcept
    {
        return BASE_64_ALPHABET[((byte0 << 4) | (byte1 >> 4)) & 0x3f];
    }
        inline constexpr char Base64Char2(uint8_t byte1, uint8_t byte2) noexcept
    {
        return BASE_64_ALPHABET[((byte1 << 2) | (byte2 >> 6)) & 0x3f];
    }
        inline constexpr char Base64Char3(uint8_t byte2) noexcept
    {
        return BASE_64_ALPHABET[byte2 & 0x3f];
    }

} // namespace

namespace jsonrpc {
    namespace util {

        inline std::string FormatIso8601DateTime(const tm& dt) {
            char str[128];
            return std::string(str, strftime(str, sizeof(str), DATE_TIME_FORMAT, &dt));
        }

        inline bool ParseIso8601DateTime(const char* text, tm& dt) {
            if (!text) {
                return false;
            }
            memset(&dt, 0, sizeof(dt));
            std::istringstream ss(text);
            ss >> std::get_time(&dt, DATE_TIME_FORMAT);

            if (ss.fail()) {
                return false;
            }
            dt.tm_isdst = -1;
            return true;
        }

        inline std::string Base64Encode(const std::string& data); // forward declaration

        inline std::string Base64Encode(const char* data, size_t size) {
            const size_t lineLength = 76;
            static_assert(lineLength % 4 == 0, "invalid line length");

            if (size == 0) {
                return{};
            }

            const size_t encodedSize = 4 * ((size + 2) / 3);
            std::string str(encodedSize + 2 * ((encodedSize - 1) / lineLength), '\0');

            size_t in = 0;
            size_t out = 0;
            for (size_t wrap = lineLength; in + 3 <= size; in += 3, wrap -= 4) {
                if (wrap == 0) {
                    str[out++] = '\r';
                    str[out++] = '\n';
                    wrap = lineLength;
                }
                str[out++] = Base64Char0(data[in]);
                str[out++] = Base64Char1(data[in], data[in + 1]);
                str[out++] = Base64Char2(data[in + 1], data[in + 2]);
                str[out++] = Base64Char3(data[in + 2]);
            }

            if (in < size) {
                str[out++] = Base64Char0(data[in]);
                if (in + 1 < size) {
                    str[out++] = Base64Char1(data[in], data[in + 1]);
                    str[out++] = Base64Char2(data[in + 1], 0);
                } else {
                    str[out++] = Base64Char1(data[in], 0);
                    str[out++] = '=';
                }
                str[out++] = '=';
            }

            assert(str.size() == out);
            return str;
        }

        inline std::string Base64Decode(const std::string& str); // forward declaration

        inline std::string Base64Decode(const char* str, size_t size) {
            const size_t maxDecodedSize = 3 * ((size + 3) / 4);

            std::string data(maxDecodedSize, '\0');

            size_t out = 0;
            uint32_t bits = 0;
            size_t bitCount = 0;

            for (size_t in = 0; in < size; ++in) {
                const int value = BASE_64_LUT[static_cast<uint8_t>(str[in])];
                if (value != -1) {
                    bits = (bits << 6) | value;
                    bitCount += 6;
                    if (bitCount == 24) {
                        data[out++] = bits >> 16;
                        data[out++] = bits >> 8;
                        data[out++] = bits;

                        bits = 0;
                        bitCount = 0;
                    }
                }
            }

            if (bitCount >= 12) {
                bits = bits >> (bitCount % 8);
                if (bitCount == 18) {
                    data[out++] = bits >> 8;
                }
                data[out++] = bits;
            }

            assert(data.size() >= out);
            data.resize(out);
            return data;
        }

    } // namespace util
} // namespace jsonrpc

inline std::string jsonrpc::util::Base64Encode(const std::string& data) {
    return Base64Encode(data.data(), data.size());
}

inline std::string jsonrpc::util::Base64Decode(const std::string& str) {
    return Base64Decode(str.data(), str.size());
}

#endif // JSONRPC_LEAN_UTIL_H
