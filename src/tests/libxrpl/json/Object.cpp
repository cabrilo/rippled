//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include <xrpl/json/Object.h>

#include <doctest/doctest.h>

#include <memory>
#include <string>

using namespace ripple;
using namespace Json;

TEST_SUITE_BEGIN("JsonObject");

struct JsonObjectFixture
{
    std::string output;
    std::unique_ptr<WriterObject> writerObject;

    Object&
    makeRoot()
    {
        writerObject =
            std::make_unique<WriterObject>(stringWriterObject(output));
        return **writerObject;
    }

    void
    expectResult(std::string const& expected)
    {
        writerObject.reset();
        CHECK(output == expected);
    }
};

TEST_CASE_FIXTURE(JsonObjectFixture, "empty object")
{
    auto& root = makeRoot();
    (void)root;
    expectResult("{}");
}

TEST_CASE_FIXTURE(JsonObjectFixture, "simple object")
{
    auto& root = makeRoot();
    root["hello"] = "world";
    root["skidoo"] = 23;
    root["awake"] = false;
    root["temperature"] = 98.6;

    expectResult(
        R"({"hello":"world","skidoo":23,"awake":false,"temperature":98.6})");
}

TEST_CASE_FIXTURE(JsonObjectFixture, "object with array")
{
    auto& root = makeRoot();
    root.setArray("ar");
    expectResult(R"({"ar":[]})");
}

TEST_CASE_FIXTURE(JsonObjectFixture, "object with multiple aggregate types")
{
    auto& root = makeRoot();
    {
        auto array = root.setArray("ar");
        array.append(23);
        array.append(false);
        array.append(23.5);
    }
    {
        auto obj = root.setObject("obj");
        obj["hello"] = "world";
    }

    {
        Json::Value value;
        value["a"] = "w";
        value["b"] = false;
        root["obj2"] = value;
    }

    expectResult(
        R"({"ar":[23,false,23.5],"obj":{"hello":"world"},"obj2":{"a":"w","b":false}})");
}

TEST_CASE_FIXTURE(JsonObjectFixture, "set object in one line")
{
    auto& root = makeRoot();
    {
        auto array = root.setArray("ar");
        array.append(23);
        array.append(false);
        array.append(23.5);
    }

    root.setObject("obj")["hello"] = "world";
    {
        auto object = root.setObject("obj2");
        object.set("a", "w");
        object.set("b", false);
    }
    expectResult(
        R"({"ar":[23,false,23.5],"obj":{"hello":"world"},"obj2":{"a":"w","b":false}})");
}

TEST_CASE_FIXTURE(JsonObjectFixture, "modify locked object")
{
    {
        auto& root = makeRoot();
        auto obj = root.setObject("o1");
        CHECK_THROWS_AS(root["fail"] = "complete", std::logic_error);
    }
    {
        auto& root = makeRoot();
        auto obj = root.setObject("o1");
        CHECK_THROWS_AS(root.setObject("o2"), std::logic_error);
    }
    {
        auto& root = makeRoot();
        auto obj = root.setArray("o1");
        CHECK_THROWS_AS(root.setArray("o2"), std::logic_error);
    }
}

TEST_CASE_FIXTURE(JsonObjectFixture, "modify locked array")
{
    {
        auto& root = makeRoot();
        auto array = root.setArray("array");
        auto subarray = array.appendArray();
        CHECK_THROWS_AS(array.append("fail"), std::logic_error);
    }
    {
        auto& root = makeRoot();
        auto array = root.setArray("array");
        auto subarray = array.appendArray();
        CHECK_THROWS_AS(array.appendArray(), std::logic_error);
    }
    {
        auto& root = makeRoot();
        auto array = root.setArray("array");
        auto subarray = array.appendArray();
        CHECK_THROWS_AS(array.appendObject(), std::logic_error);
    }
}

TEST_CASE_FIXTURE(JsonObjectFixture, "add duplicate field")
{
    auto& root = makeRoot();
    root.set("foo", "bar");
    root.set("baz", 0);
    CHECK_THROWS_AS(root.set("foo", "bar"), std::logic_error);
}

TEST_SUITE_END();
