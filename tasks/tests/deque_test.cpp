#include "gtest/gtest.h"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <gtest/gtest.h>
#include <iterator>
#include <numeric>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef USE_STL_DEQUE
#include <deque>
template <typename T> using Deque = std::deque<T>;
#endif
#ifndef USE_STL_DEQUE
#include "deque.h"
#endif

namespace {

template <typename Container, typename Value>
concept HasConcstructorsAndAssigns =
    requires(Container d, size_t size, const Value &value,
             const Container &cd) {
      Container();
      Container(size);
      Container(size, value);
      Container(d);
    } &&
    std::is_assignable_v<Container, Container> &&
    std::is_assignable_v<Container, const Container> &&
    !std::is_assignable_v<const Container, Container>;

template <typename Container, typename Value>
concept HasSettersAndGetters = requires(Container d, const Container cd) {
  { d.front() } -> std::same_as<Value &>;
  { d.back() } -> std::same_as<Value &>;
  { d[0] } -> std::same_as<Value &>;
  { d.at(0) } -> std::same_as<Value &>;

  { cd.front() } -> std::same_as<const Value &>;
  { cd.back() } -> std::same_as<const Value &>;
  { cd[0] } -> std::same_as<const Value &>;
  { cd.at(0) } -> std::same_as<const Value &>;
};

template <typename Container, typename Value>
concept HasPushPop = requires(Container d, const Value &value) {
  { d.push_front(value) } -> std::same_as<void>;
  { d.push_back(value) } -> std::same_as<void>;
  { d.pop_front() } -> std::same_as<void>;
  { d.pop_back() } -> std::same_as<void>;
};

template <typename Container, typename Value>
concept HasInsertEraseClear = requires(Container d, const Value &value) {
  { d.insert(d.begin(), value) } -> std::same_as<typename Container::iterator>;
  { d.erase(d.begin()) } -> std::same_as<typename Container::iterator>;
  { d.clear() } -> std::same_as<void>;
};

template <typename Container, typename Value>
concept HasBeginEnd = requires(Container d, const Container cd) {
  { d.begin() } -> std::same_as<typename Container::iterator>;
  { d.end() } -> std::same_as<typename Container::iterator>;
  { cd.begin() } -> std::same_as<typename Container::const_iterator>;
  { cd.end() } -> std::same_as<typename Container::const_iterator>;

  { d.cbegin() } -> std::same_as<typename Container::const_iterator>;
  { d.cend() } -> std::same_as<typename Container::const_iterator>;
  { cd.cbegin() } -> std::same_as<typename Container::const_iterator>;
  { cd.cend() } -> std::same_as<typename Container::const_iterator>;

  { d.rbegin() } -> std::same_as<typename Container::reverse_iterator>;
  { d.rend() } -> std::same_as<typename Container::reverse_iterator>;
  { cd.rbegin() } -> std::same_as<typename Container::const_reverse_iterator>;
  { cd.rend() } -> std::same_as<typename Container::const_reverse_iterator>;

  { d.crbegin() } -> std::same_as<typename Container::const_reverse_iterator>;
  { d.crend() } -> std::same_as<typename Container::const_reverse_iterator>;
  { cd.crbegin() } -> std::same_as<typename Container::const_reverse_iterator>;
  { cd.crend() } -> std::same_as<typename Container::const_reverse_iterator>;
};

template <typename Container, typename Value>
concept HasMembers = requires(Container d, size_t size) {
  { d.resize(size) } -> std::same_as<void>;
  { d.shrink_to_fit() } -> std::same_as<void>;
  { d.swap(d) } -> std::same_as<void>;
};

template <typename Container, typename Value>
concept HasTypes =
    std::same_as<typename Container::value_type, Value> &&
    std::same_as<typename Container::size_type, size_t> &&
    std::same_as<typename Container::difference_type, std::ptrdiff_t> &&
    std::same_as<typename Container::reference, Value &> &&
    std::same_as<typename Container::const_reference, const Value &> &&
    std::same_as<typename Container::pointer, Value *> &&
    std::same_as<typename Container::const_pointer, const Value *>;

template <typename Container, typename Value>
concept HasIterators =
    std::same_as<typename Container::reverse_iterator,
                 std::reverse_iterator<typename Container::iterator>> &&
    std::same_as<typename Container::const_reverse_iterator,
                 std::reverse_iterator<typename Container::const_iterator>> &&
    std::random_access_iterator<typename Container::iterator> &&
    std::is_convertible_v<typename Container::iterator,
                          typename Container::const_iterator> &&
    !std::is_convertible_v<typename Container::const_iterator,
                           typename Container::iterator>;

template <typename Container, typename Value>
concept implies_deque =
    HasConcstructorsAndAssigns<Container, Value> &&
    HasSettersAndGetters<Container, Value> && HasPushPop<Container, Value> &&
    HasInsertEraseClear<Container, Value> && HasBeginEnd<Container, Value> &&
    HasMembers<Container, Value> &&
    std::random_access_iterator<typename Container::iterator>;

template <typename T>
  requires(implies_deque<std::deque<T>, T>)
using deque = Deque<T>;

// Counts every call of defaul/copy-ctor and dtor
class Accountant {
public:
  Accountant() { ++default_ctor_calls; }
  Accountant(const Accountant &) { ++copy_default_ctor_calls; }
  ~Accountant() { ++dtor_calls; }

  static void ResetDefaultCtorCounter() { default_ctor_calls = 0; }
  static void ResetCopyCtorCounter() { copy_default_ctor_calls = 0; }
  static void ResetDtorCounter() { dtor_calls = 0; }
  static void ResetAll() {
    ResetDefaultCtorCounter();
    ResetCopyCtorCounter();
    ResetDtorCounter();
  }

  static size_t default_ctor_calls;
  static size_t copy_default_ctor_calls;
  static size_t dtor_calls;
};

size_t Accountant::default_ctor_calls = 0;
size_t Accountant::copy_default_ctor_calls = 0;
size_t Accountant::dtor_calls = 0;

// Every default-constructed object has unique number
// When copied number remains the same
class UniqueDefaultConstructed {
public:
  static size_t counter;
  size_t number_;

  UniqueDefaultConstructed() : number_(counter++) {}
  static void Reset() { counter = 0; }
};

size_t UniqueDefaultConstructed::counter = 0;

class Unique {
public:
  static size_t counter;
  size_t number_;

  Unique() : number_(counter++) {}
  Unique(const Unique &other) : number_(counter++) {}
  static void Reset() { counter = 0; }
};

class NorDefaultNorCopyConstructible {
public:
  NorDefaultNorCopyConstructible() = delete;
  NorDefaultNorCopyConstructible(const NorDefaultNorCopyConstructible &) =
      delete;
};

size_t Unique::counter = 0;

class ConstructorsTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 1000000;

  void SetUp() override {
    Accountant::ResetAll();
    UniqueDefaultConstructed::Reset();
    Unique::Reset();
  }

  void TearDown() override {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};
TEST_F(ConstructorsTests, DefaultConstructorWithAccountant) {

  {
    deque<Accountant> d;
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(0, d.size());
  }

  ASSERT_EQ(0, Accountant::default_ctor_calls);
  ASSERT_EQ(0, Accountant::dtor_calls);
  ASSERT_EQ(0, Accountant::copy_default_ctor_calls);
}

TEST_F(ConstructorsTests, SizeConstructorWithAccountant) {
  {
    deque<Accountant> d(kSmallSize);
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kSmallSize, d.size());
  }

  ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
  ASSERT_EQ(kSmallSize, Accountant::dtor_calls);
  ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kSmallSize, Accountant());
    ASSERT_EQ(1, Accountant::default_ctor_calls);
    ASSERT_EQ(1, Accountant::dtor_calls);
    ASSERT_EQ(kSmallSize, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kSmallSize, d.size());
  }

  ASSERT_EQ(1, Accountant::default_ctor_calls);
  ASSERT_EQ(kSmallSize + 1, Accountant::dtor_calls);
  ASSERT_EQ(kSmallSize, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kBigSize);
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kBigSize, d.size());
  }

  ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
  ASSERT_EQ(kBigSize, Accountant::dtor_calls);
  ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kBigSize, Accountant());
    ASSERT_EQ(1, Accountant::default_ctor_calls);
    ASSERT_EQ(1, Accountant::dtor_calls);
    ASSERT_EQ(kBigSize, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kBigSize, d.size());
  }

  ASSERT_EQ(1, Accountant::default_ctor_calls);
  ASSERT_EQ(kBigSize + 1, Accountant::dtor_calls);
  ASSERT_EQ(kBigSize, Accountant::copy_default_ctor_calls);
}

TEST_F(ConstructorsTests, CopyConstructorWithAccountant) {
  {
    deque<Accountant> d;
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    deque<Accountant> d2(d);
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(0, d2.size());
  }

  ASSERT_EQ(0, Accountant::default_ctor_calls);
  ASSERT_EQ(0, Accountant::dtor_calls);
  ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kSmallSize);
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    deque<Accountant> d2(d);
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(kSmallSize, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kSmallSize, d2.size());
  }

  ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
  ASSERT_EQ(2 * kSmallSize, Accountant::dtor_calls);
  ASSERT_EQ(kSmallSize, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kBigSize);
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    deque<Accountant> d2(d);
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(kBigSize, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kBigSize, d2.size());
  }

  ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
  ASSERT_EQ(2 * kBigSize, Accountant::dtor_calls);
  ASSERT_EQ(kBigSize, Accountant::copy_default_ctor_calls);
}

TEST_F(ConstructorsTests, SizeConstructor) {
  {
    UniqueDefaultConstructed::Reset();
    deque<UniqueDefaultConstructed> d(kSmallSize);

    ASSERT_EQ(kSmallSize, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
    }
  }

  {
    UniqueDefaultConstructed::Reset();
    deque<UniqueDefaultConstructed> d(kBigSize);

    ASSERT_EQ(kBigSize, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
    }
  }

  {
    UniqueDefaultConstructed::Reset();
    deque<UniqueDefaultConstructed> d(kSmallSize, UniqueDefaultConstructed());

    ASSERT_EQ(1, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(0, d[i].number_);
    }
  }

  {
    UniqueDefaultConstructed::Reset();
    deque<UniqueDefaultConstructed> d(kBigSize, UniqueDefaultConstructed());

    ASSERT_EQ(1, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(0, d[i].number_);
    }
  }
}

TEST_F(ConstructorsTests, CopyConstructor) {
  {
    Unique::Reset();
    deque<Unique> d(kSmallSize);
    deque<Unique> d2(d);

    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kSmallSize, d2[i].number_);
    }
  }

  {
    Unique::Reset();
    deque<Unique> d(kBigSize);
    deque<Unique> d2(d);

    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kBigSize, d2[i].number_);
    }
  }
}

TEST_F(ConstructorsTests, InitializerListConstructor) {
  {
    deque<int> d({1, 2, 3});
    ASSERT_EQ(3, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(2, d[1]);
    ASSERT_EQ(3, d[2]);
  }
}

TEST_F(ConstructorsTests, CopyAssigment) {
  {
    Unique::Reset();
    deque<Unique> d(kSmallSize);
    deque<Unique> d2;
    d2 = d;

    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kSmallSize, d2[i].number_);
    }
  }

  {
    Unique::Reset();
    deque<Unique> d(kBigSize);
    deque<Unique> d2;
    d2 = d;

    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kBigSize, d2[i].number_);
    }
  }
}

TEST_F(ConstructorsTests, CopyAssigmentWithAccountant) {
  {
    deque<Accountant> d;
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    deque<Accountant> d2;
    d2 = d;
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(0, d2.size());
  }

  ASSERT_EQ(0, Accountant::default_ctor_calls);
  ASSERT_EQ(0, Accountant::dtor_calls);
  ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kSmallSize);
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    deque<Accountant> d2;
    d2 = d;
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(kSmallSize, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kSmallSize, d2.size());
  }

  ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
  ASSERT_EQ(2 * kSmallSize, Accountant::dtor_calls);
  ASSERT_EQ(kSmallSize, Accountant::copy_default_ctor_calls);

  {
    Accountant::ResetAll();
    deque<Accountant> d(kBigSize);
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    deque<Accountant> d2;
    d2 = d;
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(kBigSize, Accountant::copy_default_ctor_calls);
    ASSERT_EQ(kBigSize, d2.size());
  }

  ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
  ASSERT_EQ(2 * kBigSize, Accountant::dtor_calls);
  ASSERT_EQ(kBigSize, Accountant::copy_default_ctor_calls);
}

class IndexationTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;
  deque<UniqueDefaultConstructed> smalldeque_;
  deque<UniqueDefaultConstructed> bigdeque_;

  IndexationTests() {}
  ~IndexationTests() {}

  void SetUp() override {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smalldeque_ = deque<UniqueDefaultConstructed>(kSmallSize);
    bigdeque_ = deque<UniqueDefaultConstructed>(kBigSize);
  }
  void TearDown() override {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(IndexationTests, IndexationSetAndGet) {
  for (size_t i = 0; i < kSmallSize; ++i) {
    ASSERT_EQ(i, smalldeque_[i].number_);
  }

  smalldeque_[2] = UniqueDefaultConstructed();
  smalldeque_[5] = UniqueDefaultConstructed();
  ASSERT_EQ(kSmallSize + kBigSize, smalldeque_[2].number_);
  ASSERT_EQ(kSmallSize + kBigSize + 1, smalldeque_[5].number_);
  for (size_t i = 0; i < kSmallSize; ++i) {
    if (i != 2 && i != 5) {
      ASSERT_EQ(i, smalldeque_[i].number_);
    }
  }

  for (size_t i = 0; i < kBigSize; ++i) {
    ASSERT_EQ(i + kSmallSize, bigdeque_[i].number_);
  }

  const int indices[] = {1, 127, 5556, 123123};
  for (size_t i = 0; i < sizeof(indices) / sizeof(int); ++i) {
    bigdeque_[indices[i]] = UniqueDefaultConstructed();
  }

  for (size_t i = 0; i < sizeof(indices) / sizeof(int); ++i) {
    ASSERT_EQ(kSmallSize + kBigSize + 2 + i, bigdeque_[indices[i]].number_);
  }
  for (size_t i = 0; i < kBigSize; ++i) {
    if (std::find(indices, indices + sizeof(indices) / sizeof(int), i) ==
        indices + sizeof(indices) / sizeof(int)) {
      ASSERT_EQ(kSmallSize + i, bigdeque_[i].number_);
    }
  }
}

class PushPopTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;
  deque<UniqueDefaultConstructed> smalldeque_;
  deque<UniqueDefaultConstructed> bigdeque_;

  PushPopTests() {}
  virtual ~PushPopTests() {}
  virtual void SetUp() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smalldeque_ = deque<UniqueDefaultConstructed>(kSmallSize);
    bigdeque_ = deque<UniqueDefaultConstructed>(kBigSize);
  }
  virtual void TearDown() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(PushPopTests, PushBackPopBack) {
  ASSERT_EQ(kSmallSize, smalldeque_.size());
  smalldeque_.push_back(UniqueDefaultConstructed());
  ASSERT_EQ(kSmallSize + 1, smalldeque_.size());
  UniqueDefaultConstructed last_element = smalldeque_.back();
  smalldeque_.pop_back();
  ASSERT_EQ(kSmallSize, smalldeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize, last_element.number_);

  for (size_t i = 0; i < smalldeque_.size(); ++i) {
    ASSERT_EQ(i, smalldeque_[i].number_);
  }

  //---------------------------------------------------

  bigdeque_.push_back(UniqueDefaultConstructed());
  ASSERT_EQ(kBigSize + 1, bigdeque_.size());

  UniqueDefaultConstructed last_big_element = bigdeque_.back();
  bigdeque_.pop_back();
  ASSERT_EQ(kBigSize, bigdeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize + 1, last_big_element.number_);

  for (size_t i = 0; i < bigdeque_.size(); ++i) {
    ASSERT_EQ(i + kSmallSize, bigdeque_[i].number_);
  }
}

TEST_F(PushPopTests, PushFrontPopFront) {
  smalldeque_.push_front(UniqueDefaultConstructed());

  ASSERT_EQ(kSmallSize + 1, smalldeque_.size());

  UniqueDefaultConstructed first_element = smalldeque_.front();
  smalldeque_.pop_front();

  ASSERT_EQ(kSmallSize, smalldeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize, first_element.number_);

  for (size_t i = 0; i < smalldeque_.size(); ++i) {
    ASSERT_EQ(i, smalldeque_[i].number_);
  }

  //------------------------------------------------------------------------------

  bigdeque_.push_front(UniqueDefaultConstructed());
  ASSERT_EQ(kBigSize + 1, bigdeque_.size());
  UniqueDefaultConstructed first_big_element = bigdeque_.front();
  bigdeque_.pop_front();
  ASSERT_EQ(kBigSize, bigdeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize + 1, first_big_element.number_);

  for (size_t i = 0; i < bigdeque_.size(); ++i) {
    ASSERT_EQ(i + kSmallSize, bigdeque_[i].number_);
  }
}

TEST_F(PushPopTests, ManyPushAndPops) {
  for (size_t i = 0; i < 100000; ++i) {
    bigdeque_.push_back(UniqueDefaultConstructed());
    bigdeque_.push_front(UniqueDefaultConstructed());
    bigdeque_.pop_back();
    bigdeque_.pop_front();
  }

  ASSERT_EQ(kBigSize, bigdeque_.size());

  deque<int> d;
  for (int i = 0; i < 1000; ++i) {
    for (int j = 0; j < 1000; ++j) {

      if (j % 3 == 2) {
        d.pop_back();
      } else {
        d.push_front(i * j);
      }
    }
  }

  ASSERT_EQ(334'000, d.size());

  for (size_t i = 0; i < 100000; ++i) {
    d.pop_front();
  }

  for (size_t i = 0; i < 233'990; ++i) {
    d.pop_back();
  }

  ASSERT_EQ(10, d.size());
  const int result[] = {62050, 61200, 59500, 58650, 56950,
                        56100, 54400, 53550, 51850, 51000};
  for (size_t i = 0; i < 10; ++i) {
    ASSERT_EQ(result[i], d[i]);
  }
}

class IteratorsTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;
  deque<UniqueDefaultConstructed> smalldeque_;

  IteratorsTests() {}
  virtual ~IteratorsTests() {}
  virtual void SetUp() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smalldeque_ = deque<UniqueDefaultConstructed>(kSmallSize);
  }
  virtual void TearDown() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};
template <typename Iter, typename T> void testIteratorArithmeticTypes() {
  using traits = std::iterator_traits<Iter>;

  ASSERT_TRUE((std::is_same_v<decltype(std::declval<Iter>()++), Iter>));
  ASSERT_TRUE((std::is_same_v<decltype(++std::declval<Iter>()), Iter &>));
  ASSERT_TRUE((std::is_same_v<decltype(std::declval<Iter>() + 5), Iter>));
  ASSERT_TRUE((std::is_same_v<decltype(std::declval<Iter>() += 5), Iter &>));

  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() - std::declval<Iter>()),
                      typename traits::difference_type>));
  ASSERT_TRUE((std::is_same_v<decltype(*std::declval<Iter>()), T &>));

  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() < std::declval<Iter>()),
                      bool>));
  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() <= std::declval<Iter>()),
                      bool>));
  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() > std::declval<Iter>()),
                      bool>));
  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() >= std::declval<Iter>()),
                      bool>));
  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() == std::declval<Iter>()),
                      bool>));
  ASSERT_TRUE(
      (std::is_same_v<decltype(std::declval<Iter>() != std::declval<Iter>()),
                      bool>));
}

TEST_F(IteratorsTests, IteratorArithmeticsTypes) {
  testIteratorArithmeticTypes<deque<Unique>::iterator, Unique>();
  testIteratorArithmeticTypes<deque<Unique>::const_iterator, const Unique>();
  testIteratorArithmeticTypes<deque<Unique>::reverse_iterator, Unique>();
  testIteratorArithmeticTypes<deque<Unique>::const_reverse_iterator,
                              const Unique>();
}

TEST_F(IteratorsTests, IteratorArithmetics) {

  ASSERT_EQ(smalldeque_.begin() + smalldeque_.size(), smalldeque_.end());
  ASSERT_EQ(smalldeque_.end() - smalldeque_.begin(), smalldeque_.size());
  ASSERT_EQ(smalldeque_.end() - smalldeque_.size(), smalldeque_.begin());

  {
    auto iter = smalldeque_.begin();
    for (size_t i = 0; i < smalldeque_.size(); ++i) {
      ASSERT_EQ(iter, smalldeque_.begin() + i);
      ++iter;
    }
    ASSERT_EQ(iter, smalldeque_.end());
  }

  {
    auto iter = smalldeque_.begin();
    for (size_t i = 0; i < smalldeque_.size(); ++i) {
      ASSERT_EQ(iter->number_, smalldeque_[i].number_);
      ++iter;
    }
  }

  {
    auto iter = smalldeque_.end() - 1;
    for (size_t i = 0; i < smalldeque_.size(); ++i) {
      ASSERT_EQ(iter->number_, smalldeque_[smalldeque_.size() - 1 - i].number_);
      if (i < smalldeque_.size() - 1) {
        --iter;
      }
    }
  }

  {
    auto iter_incr = smalldeque_.begin();
    auto iter_plus = smalldeque_.begin();
    for (; iter_incr != smalldeque_.end(); ++iter_incr, iter_plus += 1) {
      ASSERT_EQ(iter_incr, iter_plus);
    }
  }

  {
    for (size_t step = 1; step < smalldeque_.size(); ++step) {
      auto iter = smalldeque_.begin();
      for (size_t i = 0; i < smalldeque_.size(); i += step) {
        ASSERT_EQ(i, (iter + i)->number_);
      }
    }
  }
}

TEST_F(IteratorsTests, IteratorComparison) {
  {
    for (size_t i = 0; i < smalldeque_.size(); ++i) {
      ASSERT_GT(smalldeque_.end(), smalldeque_.begin() + i);
      ASSERT_GT(smalldeque_.cend(), smalldeque_.cbegin() + i);
      ASSERT_GT(smalldeque_.rend(), smalldeque_.rbegin() + i);
      ASSERT_GT(smalldeque_.crend(), smalldeque_.crbegin() + i);
    }

    ASSERT_EQ(smalldeque_.end(), smalldeque_.begin() + smalldeque_.size());
    ASSERT_EQ(smalldeque_.cend(), smalldeque_.cbegin() + smalldeque_.size());
    ASSERT_EQ(smalldeque_.rend(), smalldeque_.rbegin() + smalldeque_.size());
    ASSERT_EQ(smalldeque_.crend(), smalldeque_.crbegin() + smalldeque_.size());
  }

  {
    deque<Unique> empty;
    ASSERT_EQ(empty.end() - empty.begin(), 0);
    ASSERT_EQ(empty.begin() + 0, empty.end());
    ASSERT_EQ(empty.end() - 0, empty.begin());

    ASSERT_EQ(empty.rend() - empty.rbegin(), 0);
    ASSERT_EQ(empty.rbegin() + 0, empty.rend());
    ASSERT_EQ(empty.rend() - 0, empty.rbegin());

    ASSERT_EQ(empty.cend() - empty.cbegin(), 0);
    ASSERT_EQ(empty.cbegin() + 0, empty.cend());
    ASSERT_EQ(empty.cend() - 0, empty.cbegin());

    ASSERT_EQ(empty.crend() - empty.crbegin(), 0);
    ASSERT_EQ(empty.crbegin() + 0, empty.crend());
    ASSERT_EQ(empty.crend() - 0, empty.crbegin());
  }
}

TEST_F(IteratorsTests, IteratorsAlgorithms) {
  deque<int> d(1000);
  std::iota(d.begin(), d.end(), 13);
  std::mt19937 g(31415);
  std::shuffle(d.begin(), d.end(), g);
  std::sort(d.rbegin(), d.rbegin() + 500);
  std::reverse(d.begin(), d.end());
  auto sorted_border = std::is_sorted_until(d.begin(), d.end());
  ASSERT_EQ(sorted_border - d.begin(), 500);
}

TEST_F(IteratorsTests, IteratorsInvalidationAfterPop) {
  {
    auto iter = smalldeque_.begin();
    for (size_t i = 0; i < smalldeque_.size(); ++i) {
      ASSERT_EQ(iter->number_, smalldeque_[i].number_);
      ++iter;
    }
  }

  {
    std::vector<deque<UniqueDefaultConstructed>::iterator> iterators;
    for (size_t i = 0; i <= smalldeque_.size(); ++i) {
      iterators.push_back(smalldeque_.begin() + i);
    }

    size_t start = 0, end = smalldeque_.size();
    while (start < end) {
      smalldeque_.pop_front();
      ++start;
      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(iterators[i]->number_, i);
        ASSERT_EQ(iterators[i], smalldeque_.begin() + i - start);
        ASSERT_EQ(&*iterators[i], &smalldeque_[i - start]);
      }
      ASSERT_EQ(iterators[end], smalldeque_.end());
    }
  }

  {
    std::vector<deque<UniqueDefaultConstructed>::iterator> iterators;
    for (size_t i = 0; i <= smalldeque_.size(); ++i) {
      iterators.push_back(smalldeque_.begin() + i);
    }

    size_t start = 0, end = smalldeque_.size();
    while (start < end) {
      smalldeque_.pop_back();
      --end;
      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(iterators[i]->number_, i);
        ASSERT_EQ(iterators[i], smalldeque_.begin() + i - start);
        ASSERT_EQ(&*iterators[i], &smalldeque_[i - start]);
      }
      ASSERT_EQ(iterators[end], smalldeque_.end());
    }
  }

  {
    std::vector<deque<UniqueDefaultConstructed>::iterator> iterators;
    for (size_t i = 0; i <= smalldeque_.size(); ++i) {
      iterators.push_back(smalldeque_.begin() + i);
    }

    size_t start = 0, end = smalldeque_.size();
    for (size_t round = 0; start < end; ++round) {
      if (round % 2 == 0) {
        smalldeque_.pop_back();
        --end;
      } else {
        smalldeque_.pop_front();
        ++start;
      }

      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(iterators[i]->number_, i);
        ASSERT_EQ(iterators[i], smalldeque_.begin() + i - start);
        ASSERT_EQ(&*iterators[i], &smalldeque_[i - start]);
      }
      ASSERT_EQ(iterators[end], smalldeque_.end());
    }
  }
}

TEST_F(IteratorsTests, PointerAndReferenceInvalidationAfterPush) {
  {
    std::vector<UniqueDefaultConstructed *> pointers;
    for (size_t i = 0; i < smalldeque_.size(); ++i) {
      pointers.push_back(&smalldeque_[i]);
    }

    size_t start = 0, end = smalldeque_.size();
    for (size_t round = 0; round < 100; ++round) {
      if (round % 2 == 0) {
        smalldeque_.push_back(UniqueDefaultConstructed());
      } else {
        smalldeque_.push_front(UniqueDefaultConstructed());
        ++start;
        ++end;
      }

      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(pointers[i - start]->number_, i - start);
        ASSERT_EQ(pointers[i - start], &smalldeque_[i]);
        ASSERT_EQ(pointers[i - start], &*(smalldeque_.begin() + i));
      }
    }
  }
}

class InsertEraseTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;
  deque<UniqueDefaultConstructed> smalldeque_;
  deque<UniqueDefaultConstructed> bigdeque_;

  InsertEraseTests() {}
  virtual ~InsertEraseTests() {}
  virtual void SetUp() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smalldeque_ = deque<UniqueDefaultConstructed>(kSmallSize);
    bigdeque_ = deque<UniqueDefaultConstructed>(kBigSize);
  }
  virtual void TearDown() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(InsertEraseTests, InsertErase) {
  {
    smalldeque_.insert(smalldeque_.begin() + kSmallSize / 2,
                       UniqueDefaultConstructed());
    ASSERT_EQ(kSmallSize + 1, smalldeque_.size());
    for (size_t i = 0; i < kSmallSize / 2; ++i) {
      ASSERT_EQ(i, smalldeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize, smalldeque_[kSmallSize / 2].number_);
    for (size_t i = kSmallSize / 2; i < kSmallSize; ++i) {
      ASSERT_EQ(i, smalldeque_[i + 1].number_);
    }
  }

  {
    bigdeque_.insert(bigdeque_.begin() + kBigSize / 2,
                     UniqueDefaultConstructed());
    ASSERT_EQ(kBigSize + 1, bigdeque_.size());
    for (size_t i = 0; i < kBigSize / 2; ++i) {
      ASSERT_EQ(i + kSmallSize, bigdeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize + 1, bigdeque_[kBigSize / 2].number_);
    for (size_t i = kBigSize / 2; i < kBigSize; ++i) {
      ASSERT_EQ(i + kSmallSize, bigdeque_[i + 1].number_);
    }
  }

  {
    smalldeque_.erase(smalldeque_.begin() + kSmallSize / 2 - 1);
    ASSERT_EQ(kSmallSize, smalldeque_.size());
    for (size_t i = 0; i < kSmallSize / 2 - 1; ++i) {
      ASSERT_EQ(i, smalldeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize, smalldeque_[kSmallSize / 2 - 1].number_);
    for (size_t i = kSmallSize / 2; i < kSmallSize; ++i) {
      ASSERT_EQ(i, smalldeque_[i].number_);
    }
  }

  {
    bigdeque_.erase(bigdeque_.begin() + kBigSize / 2 - 1);
    ASSERT_EQ(kBigSize, bigdeque_.size());
    for (size_t i = 0; i < kBigSize / 2 - 1; ++i) {
      ASSERT_EQ(i + kSmallSize, bigdeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize + 1, bigdeque_[kBigSize / 2 - 1].number_);
    for (size_t i = kBigSize / 2; i < kBigSize; ++i) {
      ASSERT_EQ(i + kSmallSize, bigdeque_[i].number_);
    }
  }
}

class ExceptionTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;

  struct CountedException : public std::exception {};

  template <int when_throw> struct Counted {
    inline static int counter = 0;

    Counted() {
      ++counter;
      if (counter == when_throw) {
        --counter;
        throw CountedException();
      }
    }

    Counted(const Counted &) : Counted() {}

    ~Counted() { --counter; }
  };

  struct Explosive {
    struct Safeguard {};

    inline static bool exploded = false;

    Explosive() : should_explode(true) { throw 1; }

    Explosive(Safeguard) : should_explode(false) {}

    Explosive(const Explosive &) : should_explode(true) { throw 2; }

    Explosive &operator=(const Explosive &) { return *this; }

    ~Explosive() { exploded |= should_explode; }

  private:
    const bool should_explode;
  };

  struct Fragile {
    Fragile(int durability, int data) : durability(durability), data(data) {}
    ~Fragile() = default;

    // for std::swap
    Fragile(Fragile &&other) : Fragile() { *this = other; }

    Fragile(const Fragile &other) : Fragile() { *this = other; }

    Fragile &operator=(const Fragile &other) {
      durability = other.durability - 1;
      data = other.data;
      if (durability <= 0) {
        throw 2;
      }
      return *this;
    }

    int durability;
    int data;

  private:
    Fragile() {}
  };

  ExceptionTests() {}
  virtual ~ExceptionTests() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(ExceptionTests, ThrowsExceptions) {
  ASSERT_THROW([]() { deque<Counted<17>> d(100); }(), CountedException);
  try {
    deque<Counted<17>> d(100);
  } catch (CountedException &) {
    ASSERT_EQ(Counted<17>::counter, 0);
  }

  ASSERT_THROW([]() { deque<Explosive> d(100); }(), int);
  ASSERT_NO_THROW([]() { deque<Explosive> d; }());
  ASSERT_FALSE(Explosive::exploded);

  ASSERT_THROW(
      []() {
        deque<Explosive> d;
        auto safe = Explosive(Explosive::Safeguard{});
        d.push_back(safe);
      }(),
      int);
  ASSERT_FALSE(Explosive::exploded);
}

TEST_F(ExceptionTests, StrongGuarantee) {
  const size_t size = 20'000;
  const size_t initial_data = 100;
  deque<Fragile> d(size, Fragile(size, initial_data));

  auto is_intact = [&] {
    return d.size() == size &&
           std::all_of(d.begin(), d.end(), [initial_data](const auto &item) {
             return item.data == initial_data;
           });
  };

  EXPECT_THROW(
      [&d]() {
        d.insert(d.begin() + size / 2, Fragile(0, initial_data + 1));
      }(),
      int);
  EXPECT_TRUE(is_intact());

  try {
    // for those who like additional copies...
    d.insert(d.begin() + size / 2, Fragile(3, initial_data + 2));
  } catch (...) {
    // might throw depending on the implementation
    // if it DID throw, then deque should be untouched
    EXPECT_TRUE(is_intact());
  }
}

} // namespace

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}