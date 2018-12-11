/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>

#include <qi/atomic.hpp>
#include <limits>
#include <qi/types.hpp>

template<template <typename> class A, typename T>
void sub_test_type()
{
  T max = std::numeric_limits<T>::max();
  T min = std::numeric_limits<T>::min();

  A<T> n(max);
  EXPECT_EQ(n.load(), max);
  ++n;
  EXPECT_EQ(n.load(), min);

  A<T> m(min);
  EXPECT_EQ(m.load(), min);
  --m;
  EXPECT_EQ(m.load(), max);

  m = 42;
  EXPECT_EQ((T) 42, m.load());

  T old = m.load();
  EXPECT_EQ(old, m.swap(51));
  EXPECT_EQ((T) 51, m.load());
}

template <typename T>
void test_type()
{
  // new API
  sub_test_type<qi::Atomic, T>();
}

TEST(QiAtomic, tas)
{
  long lock = 0;
  EXPECT_EQ(1, qi::testAndSet(&lock));
  EXPECT_EQ(0, qi::testAndSet(&lock));
  lock = 0;
  EXPECT_EQ(1, qi::testAndSet(&lock));
}


TEST(QiAtomic, int)
{
  test_type<int>();
}

TEST(QiAtomic, unsignedInt)
{
  test_type<unsigned int>();
}
