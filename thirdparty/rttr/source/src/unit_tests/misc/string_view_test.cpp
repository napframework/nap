/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 - 2017 Axel Menzel <info@rttr.org>                     *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#include <rttr/type>
#include <rttr/string_view.h>

#include <iostream>
#include <memory>
#include <functional>
#include <type_traits>

#include <catch/catch.hpp>

using namespace rttr;
using namespace std;


/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - rttr::string_view()", "[rttr::string_view]")
{
    rttr::string_view text;

    CHECK(text.empty() == true);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - rttr::string_view(const rttr::string_view&)", "[rttr::string_view]")
{
    rttr::string_view text1;
    rttr::string_view text2 = text1;
    rttr::string_view text3 = "Hello World";
    rttr::string_view text4 = text3;

    CHECK(text1.empty() == true);
    CHECK(text2.empty() == true);
    CHECK(text3.empty() == false);
    CHECK(text4.empty() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - rttr::string_view(const char*)", "[rttr::string_view]")
{
    rttr::string_view text("Hello World");

    CHECK(text.empty() == false);
    CHECK(text == "Hello World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - rttr::string_view(const char*, std::size_t)", "[rttr::string_view]")
{
    rttr::string_view text("Hello World", 5);

    CHECK(text.empty() == false);
    CHECK(text == "Hello");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - rttr::string_view(std::string)", "[rttr::string_view]")
{
    std::string string = "Hello World";
    rttr::string_view text(string);

    CHECK(text.empty() == false);
    CHECK(text == "Hello World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - operator=(const basic_rttr::string_view&)", "[rttr::string_view]")
{
    rttr::string_view obj1 = "Hello";
    rttr::string_view obj2 = "World";
    obj1 = obj2;

    CHECK(obj1 == "World");
    CHECK(obj2 == "World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - swap()", "[rttr::string_view]")
{
    rttr::string_view obj1 = "Hello";
    rttr::string_view obj2 = "World";
    obj1.swap(obj2);

    CHECK(obj1 == "World");
    CHECK(obj2 == "Hello");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - size(), length(), max_size(), empty()", "[rttr::string_view]")
{
    rttr::string_view text = "Hello World";

    CHECK(text.size()       == 11);
    CHECK(text.length()     == 11);
    CHECK(text.max_size()   == 11);
    CHECK(text.empty()      == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - front(), back(), data()", "[rttr::string_view]")
{
    rttr::string_view text = "Hello World";

    CHECK(rttr::string_view(&text.front(), 1) == "H");
    CHECK(rttr::string_view(&text.back(), 1)  == "d");
    CHECK(rttr::string_view(text.data())      == "Hello World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - begin(), end(), cbegin(), cend()", "[rttr::string_view]")
{
    auto string_literal = "Hello World";
    rttr::string_view text = string_literal;

    CHECK(*text.begin() == *"H");
    CHECK(text.end() == string_literal + std::char_traits<char>::length(string_literal));

    CHECK(*text.cbegin() == *"H");
    CHECK(text.cend() == string_literal + std::char_traits<char>::length(string_literal));
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - rbegin(), rend(), crbegin(), crend()", "[rttr::string_view]")
{
    auto string_literal = "Hello World";
    rttr::string_view text = string_literal;
    auto e = *text.rbegin();


    CHECK(*text.rbegin() == *"d");
    CHECK(text.rend() == rttr::string_view::reverse_iterator(string_literal));

    CHECK(*text.crbegin() == *"d");
    CHECK(text.crend() == rttr::string_view::reverse_iterator(string_literal));
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - operator[]", "[rttr::string_view]")
{
    rttr::string_view text1 = "Hello World";
    std::string text2(text1);
    for (std::size_t i = 0; i < text2.length(); ++i)
    {
        CHECK(text1[i] == text2[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - remove_prefix()", "[rttr::string_view]")
{
    rttr::string_view text = "Hello World";
    text.remove_prefix(6);

    CHECK(text == "World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - remove_suffix()", "[rttr::string_view]")
{
    rttr::string_view text = "Hello World";
    text.remove_suffix(6);

    CHECK(text == "Hello");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - std::string operator()", "[rttr::string_view]")
{
    rttr::string_view text = "Hello World";
    std::string string_value(text);

    CHECK(string_value == "Hello World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - to_string", "[rttr::string_view]")
{
    rttr::string_view text = "Hello World";
    std::string string_value = text.to_string();

    CHECK(string_value == "Hello World");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - operator==()", "[rttr::string_view]")
{
    rttr::string_view text1 = "Hello World";
    rttr::string_view text2 = "Hello World";
    rttr::string_view text3;
    rttr::string_view text4 = "Other";

    CHECK(text1 == text2);
    CHECK(text1 == std::string("Hello World") );

    // negative
    CHECK( !(text1 == text3) );
    CHECK( !(text1 == text4) );
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("rttr::string_view - operator!=()", "[rttr::string_view]")
{
    rttr::string_view text1 = "Hello World";
    rttr::string_view text2 = "Hello World";
    rttr::string_view text3;
    rttr::string_view text4 = "Other";

    CHECK(text1 == text2);
    // negative
    CHECK( !(text1 == text3) );
    CHECK( !(text1 == text4) );
}

/////////////////////////////////////////////////////////////////////////////////////////
