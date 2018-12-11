#include <memory>
#include <gtest/gtest.h>
#include <ka/src.hpp>
#include <qi/atomic.hpp>
#include <qi/future.hpp>
#include <qi/messaging/sock/option.hpp>

TEST(Src, VariousTypes)
{
  using namespace qi;
  using namespace ka;
  int i = 42;
  {
    const Future<int> x{i};
    ASSERT_EQ(i, src(x));
  }
  {
    const Atomic<int> x{i};
    ASSERT_EQ(i, src(x));
  }
  {
    const bool b = true;
    const sock::IpV6Enabled x{b};
    ASSERT_EQ(b, src(x));
  }
}

TEST(Src, Polymorphic)
{
  using namespace qi;
  using namespace ka;

  const int i = 42;
  src_t src;
  ASSERT_EQ(i, src(Future<int>{i}));

  std::unique_ptr<int> p{new int{i}};
  ASSERT_EQ(i, src(p));
  ++src(p);
  ASSERT_EQ(i + 1, src(p));

  ASSERT_EQ(i, src(i));

  auto f = compose(src, UnitFuture{});
  ASSERT_EQ(i, f(i));
}
