// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lib/data/context.hpp"


class ContextTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        m.insert(7, 'a');
        m.insert(42,'+');
        m.insert(3,  fcpp::details::make_field(1,{{0,3}, {6,4}}));
        m.insert(18, fcpp::details::make_field(9,{{1,2}, {9,2}}));
        m.insert(8);
        data.insert(1, m);
    }
    
    fcpp::multitype_map<fcpp::trace_t, fcpp::field<int>, char> m;
    fcpp::context<fcpp::field<int>, char> data{0};
};


TEST_F(ContextTest, Operators) {
    fcpp::context<fcpp::field<int>, char> x(data), y(-1), z(-1);
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}

TEST_F(ContextTest, InsertErase) {
    fcpp::context<fcpp::field<int>, char> x(data);
    x.insert(2, m);
    x.erase(2);
    EXPECT_EQ(data, x);
}

TEST_F(ContextTest, Align) {
    m.insert(9);
    data.insert(2, m);
    std::vector<fcpp::device_t> ex, res;
    ex = std::vector<fcpp::device_t>{1,2};
    res = data.align(8);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(ex, res);
    ex = std::vector<fcpp::device_t>{2};
    res = data.align(9);
    std::sort(res.begin(), res.end());
    EXPECT_EQ(ex, res);
}

TEST_F(ContextTest, Old) {
    char c;
    c = data.old(7, 'c');
    EXPECT_EQ('c', c);
    data.insert(0, m);
    c = data.old(7, 'c');
    EXPECT_EQ('a', c);
}

TEST_F(ContextTest, Nbr) {
    m.insert(42, '-');
    m.insert(3,  fcpp::details::make_field(1, {{0,2}, {5,9}}));
    m.insert(18, fcpp::details::make_field(1, {{0,3}, {5,7}}));
    data.insert(2, m);
    fcpp::field<char> fcr, fce;
    fcr = data.template nbr<char>(42, '*');
    fce = fcpp::details::make_field('*', {{1,'+'}, {2,'-'}});
    EXPECT_EQ(fce, fcr);
    fcpp::field<int> fir, fie;
    fir = data.template nbr<fcpp::field<int>>(18, -1);
    fie = fcpp::details::make_field(-1, {{1,9}, {2,3}});
    EXPECT_EQ(fie, fir);
    fir = data.template nbr<fcpp::field<int>>(3, 7);
    fie = fcpp::details::make_field(7, {{1,3}, {2,2}});
    EXPECT_EQ(fie, fir);
}
