#include "gtest/gtest.h"
#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <iterator>
#include <numeric>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// #include <deque>
// template <typename T> using Deque = std::deque<T>;

#include "deque.h"

namespace {

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

TEST_F(ConstructorsTests, HasConstructors) {
  ASSERT_TRUE((std::is_default_constructible_v<Deque<Unique>>));
  ASSERT_TRUE((std::is_copy_constructible_v<Deque<Unique>>));
  ASSERT_TRUE((std::is_constructible_v<Deque<Unique>, int>));
  ASSERT_TRUE((std::is_constructible_v<Deque<Unique>, int, const Unique &>));
  ASSERT_TRUE((std::is_copy_assignable_v<Deque<Unique>>));
}

TEST_F(ConstructorsTests, DefaultConstructorWithAccountant) {

  {
    Deque<Accountant> d;
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
    Deque<Accountant> d(kSmallSize);
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
    Deque<Accountant> d(kSmallSize, Accountant());
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
    Deque<Accountant> d(kBigSize);
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
    Deque<Accountant> d(kBigSize, Accountant());
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
    Deque<Accountant> d;
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    Deque<Accountant> d2(d);
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
    Deque<Accountant> d(kSmallSize);
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    Deque<Accountant> d2(d);
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
    Deque<Accountant> d(kBigSize);
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    Deque<Accountant> d2(d);
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
    Deque<UniqueDefaultConstructed> d(kSmallSize);

    ASSERT_EQ(kSmallSize, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
    }
  }

  {
    UniqueDefaultConstructed::Reset();
    Deque<UniqueDefaultConstructed> d(kBigSize);

    ASSERT_EQ(kBigSize, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
    }
  }

  {
    UniqueDefaultConstructed::Reset();
    Deque<UniqueDefaultConstructed> d(kSmallSize, UniqueDefaultConstructed());

    ASSERT_EQ(1, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(0, d[i].number_);
    }
  }

  {
    UniqueDefaultConstructed::Reset();
    Deque<UniqueDefaultConstructed> d(kBigSize, UniqueDefaultConstructed());

    ASSERT_EQ(1, UniqueDefaultConstructed::counter);
    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(0, d[i].number_);
    }
  }
}

TEST_F(ConstructorsTests, CopyConstructor) {
  {
    Unique::Reset();
    Deque<Unique> d(kSmallSize);
    Deque<Unique> d2(d);

    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kSmallSize, d2[i].number_);
    }
  }

  {
    Unique::Reset();
    Deque<Unique> d(kBigSize);
    Deque<Unique> d2(d);

    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kBigSize, d2[i].number_);
    }
  }
}

TEST_F(ConstructorsTests, InitializerListConstructor) {
  {
    Deque<int> d({1, 2, 3});
    ASSERT_EQ(3, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(2, d[1]);
    ASSERT_EQ(3, d[2]);
  }
}

TEST_F(ConstructorsTests, CopyAssigment) {
  {
    Unique::Reset();
    Deque<Unique> d(kSmallSize);
    Deque<Unique> d2;
    d2 = d;

    for (size_t i = 0; i < kSmallSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kSmallSize, d2[i].number_);
    }
  }

  {
    Unique::Reset();
    Deque<Unique> d(kBigSize);
    Deque<Unique> d2;
    d2 = d;

    for (size_t i = 0; i < kBigSize; ++i) {
      ASSERT_EQ(i, d[i].number_);
      ASSERT_EQ(i + kBigSize, d2[i].number_);
    }
  }
}

TEST_F(ConstructorsTests, CopyAssigmentWithAccountant) {
  {
    Deque<Accountant> d;
    ASSERT_EQ(0, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    Deque<Accountant> d2;
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
    Deque<Accountant> d(kSmallSize);
    ASSERT_EQ(kSmallSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    Deque<Accountant> d2;
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
    Deque<Accountant> d(kBigSize);
    ASSERT_EQ(kBigSize, Accountant::default_ctor_calls);
    ASSERT_EQ(0, Accountant::dtor_calls);
    ASSERT_EQ(0, Accountant::copy_default_ctor_calls);

    Deque<Accountant> d2;
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
  Deque<UniqueDefaultConstructed> smallDeque_;
  Deque<UniqueDefaultConstructed> bigDeque_;

  IndexationTests() {}
  ~IndexationTests() {}

  void SetUp() override {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smallDeque_ = Deque<UniqueDefaultConstructed>(kSmallSize);
    bigDeque_ = Deque<UniqueDefaultConstructed>(kBigSize);
  }
  void TearDown() override {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(IndexationTests, IndexationReturnType) {
  ASSERT_TRUE(
      (std::is_same_v<Unique &,
                      decltype(std::declval<Deque<Unique>>().operator[](0))>));
  ASSERT_TRUE((std::is_same_v<
               const Unique &,
               decltype(std::declval<Deque<const Unique>>().operator[](0))>));
  ASSERT_TRUE((
      std::is_same_v<Unique &,
                     decltype(std::declval<Deque<Unique> &>().operator[](0))>));
  ASSERT_TRUE((std::is_same_v<
               const Unique &,
               decltype(std::declval<const Deque<Unique> &>().operator[](0))>));

  ASSERT_TRUE(
      (std::is_same_v<Unique &,
                      decltype(std::declval<Deque<Unique>>().front())>));
  ASSERT_TRUE(
      (std::is_same_v<const Unique &,
                      decltype(std::declval<Deque<const Unique>>().front())>));
  ASSERT_TRUE(
      (std::is_same_v<Unique &,
                      decltype(std::declval<Deque<Unique> &>().front())>));
  ASSERT_TRUE((
      std::is_same_v<const Unique &,
                     decltype(std::declval<const Deque<Unique> &>().front())>));

  ASSERT_TRUE((std::is_same_v<Unique &,
                              decltype(std::declval<Deque<Unique>>().back())>));
  ASSERT_TRUE(
      (std::is_same_v<const Unique &,
                      decltype(std::declval<Deque<const Unique>>().back())>));
  ASSERT_TRUE(
      (std::is_same_v<Unique &,
                      decltype(std::declval<Deque<Unique> &>().back())>));
  ASSERT_TRUE(
      (std::is_same_v<const Unique &,
                      decltype(std::declval<const Deque<Unique> &>().back())>));

  ASSERT_TRUE((
      std::is_same_v<Unique &, decltype(std::declval<Deque<Unique>>().at(0))>));
  ASSERT_TRUE(
      (std::is_same_v<const Unique &,
                      decltype(std::declval<Deque<const Unique>>().at(0))>));
  ASSERT_TRUE(
      (std::is_same_v<Unique &,
                      decltype(std::declval<Deque<Unique> &>().at(0))>));
  ASSERT_TRUE(
      (std::is_same_v<const Unique &,
                      decltype(std::declval<const Deque<Unique> &>().at(0))>));

  ASSERT_THROW(smallDeque_.at(kSmallSize + 100), std::out_of_range);
  ASSERT_THROW(bigDeque_.at(kBigSize + 100), std::out_of_range);
  ASSERT_THROW(smallDeque_.at(-100), std::out_of_range);
  ASSERT_THROW(bigDeque_.at(-100), std::out_of_range);
}

TEST_F(IndexationTests, IndexationSetAndGet) {
  for (size_t i = 0; i < kSmallSize; ++i) {
    ASSERT_EQ(i, smallDeque_[i].number_);
  }

  smallDeque_[2] = UniqueDefaultConstructed();
  smallDeque_[5] = UniqueDefaultConstructed();
  ASSERT_EQ(kSmallSize + kBigSize, smallDeque_[2].number_);
  ASSERT_EQ(kSmallSize + kBigSize + 1, smallDeque_[5].number_);
  for (size_t i = 0; i < kSmallSize; ++i) {
    if (i != 2 && i != 5) {
      ASSERT_EQ(i, smallDeque_[i].number_);
    }
  }

  for (size_t i = 0; i < kBigSize; ++i) {
    ASSERT_EQ(i + kSmallSize, bigDeque_[i].number_);
  }

  const int indices[] = {1, 127, 5556, 123123};
  for (size_t i = 0; i < sizeof(indices) / sizeof(int); ++i) {
    bigDeque_[indices[i]] = UniqueDefaultConstructed();
  }

  for (size_t i = 0; i < sizeof(indices) / sizeof(int); ++i) {
    ASSERT_EQ(kSmallSize + kBigSize + 2 + i, bigDeque_[indices[i]].number_);
  }
  for (size_t i = 0; i < kBigSize; ++i) {
    if (std::find(indices, indices + sizeof(indices) / sizeof(int), i) ==
        indices + sizeof(indices) / sizeof(int)) {
      ASSERT_EQ(kSmallSize + i, bigDeque_[i].number_);
    }
  }
}

class PushPopTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;
  Deque<UniqueDefaultConstructed> smallDeque_;
  Deque<UniqueDefaultConstructed> bigDeque_;

  PushPopTests() {}
  virtual ~PushPopTests() {}
  virtual void SetUp() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smallDeque_ = Deque<UniqueDefaultConstructed>(kSmallSize);
    bigDeque_ = Deque<UniqueDefaultConstructed>(kBigSize);
  }
  virtual void TearDown() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(PushPopTests, PushBackPopBack) {
  ASSERT_EQ(kSmallSize, smallDeque_.size());
  smallDeque_.push_back(UniqueDefaultConstructed());
  ASSERT_EQ(kSmallSize + 1, smallDeque_.size());
  UniqueDefaultConstructed last_element = smallDeque_.back();
  smallDeque_.pop_back();
  ASSERT_EQ(kSmallSize, smallDeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize, last_element.number_);

  for (size_t i = 0; i < smallDeque_.size(); ++i) {
    ASSERT_EQ(i, smallDeque_[i].number_);
  }

  //---------------------------------------------------

  bigDeque_.push_back(UniqueDefaultConstructed());
  ASSERT_EQ(kBigSize + 1, bigDeque_.size());

  UniqueDefaultConstructed last_big_element = bigDeque_.back();
  bigDeque_.pop_back();
  ASSERT_EQ(kBigSize, bigDeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize + 1, last_big_element.number_);

  for (size_t i = 0; i < bigDeque_.size(); ++i) {
    ASSERT_EQ(i + kSmallSize, bigDeque_[i].number_);
  }
}

TEST_F(PushPopTests, PushFrontPopFront) {
  smallDeque_.push_front(UniqueDefaultConstructed());

  ASSERT_EQ(kSmallSize + 1, smallDeque_.size());

  UniqueDefaultConstructed first_element = smallDeque_.front();
  smallDeque_.pop_front();

  ASSERT_EQ(kSmallSize, smallDeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize, first_element.number_);

  for (size_t i = 0; i < smallDeque_.size(); ++i) {
    ASSERT_EQ(i, smallDeque_[i].number_);
  }

  //------------------------------------------------------------------------------

  bigDeque_.push_front(UniqueDefaultConstructed());
  ASSERT_EQ(kBigSize + 1, bigDeque_.size());
  UniqueDefaultConstructed first_big_element = bigDeque_.front();
  bigDeque_.pop_front();
  ASSERT_EQ(kBigSize, bigDeque_.size());
  ASSERT_EQ(kSmallSize + kBigSize + 1, first_big_element.number_);

  for (size_t i = 0; i < bigDeque_.size(); ++i) {
    ASSERT_EQ(i + kSmallSize, bigDeque_[i].number_);
  }
}

TEST_F(PushPopTests, ManyPushAndPops) {
  for (size_t i = 0; i < 100000; ++i) {
    bigDeque_.push_back(UniqueDefaultConstructed());
    bigDeque_.push_front(UniqueDefaultConstructed());
    bigDeque_.pop_back();
    bigDeque_.pop_front();
  }

  ASSERT_EQ(kBigSize, bigDeque_.size());

  Deque<int> d;
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
  Deque<UniqueDefaultConstructed> smallDeque_;

  IteratorsTests() {}
  virtual ~IteratorsTests() {}
  virtual void SetUp() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smallDeque_ = Deque<UniqueDefaultConstructed>(kSmallSize);
  }
  virtual void TearDown() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(IteratorsTests, IteratorTypes) {
  ASSERT_TRUE((std::is_same_v<std::reverse_iterator<Deque<Unique>::iterator>,
                              Deque<Unique>::reverse_iterator>));
  ASSERT_TRUE(
      (std::is_same_v<std::reverse_iterator<Deque<Unique>::const_iterator>,
                      Deque<Unique>::const_reverse_iterator>));

  ASSERT_TRUE((
      std::is_same_v<Unique &, decltype(std::declval<Deque<Unique>::iterator>().
                                        operator*())>));
  ASSERT_TRUE(
      (std::is_same_v<const Unique &,
                      decltype(std::declval<Deque<Unique>::const_iterator>().
                               operator*())>));
  ASSERT_TRUE(
      (std::is_same_v<Unique &,
                      decltype(std::declval<Deque<Unique>::reverse_iterator>().
                               operator*())>));
  ASSERT_TRUE((std::is_same_v<
               const Unique &,
               decltype(std::declval<Deque<Unique>::const_reverse_iterator>().
                        operator*())>));

  ASSERT_TRUE(
      (std::is_same_v<
          Unique *,
          decltype(std::declval<Deque<Unique>::iterator>().operator->())>));
  ASSERT_TRUE((std::is_same_v<
               const Unique *,
               decltype(std::declval<Deque<Unique>::const_iterator>().
                        operator->())>));
  ASSERT_TRUE(
      (std::is_same_v<
          Unique *,
          decltype(std::declval<Deque<Unique>::reverse_iterator>().
                   operator->())>));
  ASSERT_TRUE((std::is_same_v<
               const Unique *,
               decltype(std::declval<Deque<Unique>::const_reverse_iterator>()
                            .
                            operator->())>));

  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::iterator,
                      decltype(std::declval<Deque<Unique>>().begin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<const Deque<Unique>>().begin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::iterator,
                      decltype(std::declval<Deque<Unique> &>().begin())>));
  ASSERT_TRUE((
      std::is_same_v<Deque<Unique>::const_iterator,
                     decltype(std::declval<const Deque<Unique> &>().begin())>));

  ASSERT_TRUE((std::is_same_v<Deque<Unique>::iterator,
                              decltype(std::declval<Deque<Unique>>().end())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<const Deque<Unique>>().end())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::iterator,
                      decltype(std::declval<Deque<Unique> &>().end())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<const Deque<Unique> &>().end())>));

  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<Deque<Unique>>().cbegin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<const Deque<Unique>>().cbegin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<Deque<Unique> &>().cbegin())>));
  ASSERT_TRUE((std::is_same_v<
               Deque<Unique>::const_iterator,
               decltype(std::declval<const Deque<Unique> &>().cbegin())>));

  ASSERT_TRUE((std::is_same_v<Deque<Unique>::const_iterator,
                              decltype(std::declval<Deque<Unique>>().cend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<const Deque<Unique>>().cend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<Deque<Unique> &>().cend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_iterator,
                      decltype(std::declval<const Deque<Unique> &>().cend())>));

  // reverse
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::reverse_iterator,
                      decltype(std::declval<Deque<Unique>>().rbegin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<const Deque<Unique>>().rbegin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::reverse_iterator,
                      decltype(std::declval<Deque<Unique> &>().rbegin())>));
  ASSERT_TRUE((std::is_same_v<
               Deque<Unique>::const_reverse_iterator,
               decltype(std::declval<const Deque<Unique> &>().rbegin())>));

  ASSERT_TRUE((std::is_same_v<Deque<Unique>::reverse_iterator,
                              decltype(std::declval<Deque<Unique>>().rend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<const Deque<Unique>>().rend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::reverse_iterator,
                      decltype(std::declval<Deque<Unique> &>().rend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<const Deque<Unique> &>().rend())>));

  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<Deque<Unique>>().crbegin())>));
  ASSERT_TRUE((
      std::is_same_v<Deque<Unique>::const_reverse_iterator,
                     decltype(std::declval<const Deque<Unique>>().crbegin())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<Deque<Unique> &>().crbegin())>));
  ASSERT_TRUE((std::is_same_v<
               Deque<Unique>::const_reverse_iterator,
               decltype(std::declval<const Deque<Unique> &>().crbegin())>));

  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<Deque<Unique>>().crend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<const Deque<Unique>>().crend())>));
  ASSERT_TRUE(
      (std::is_same_v<Deque<Unique>::const_reverse_iterator,
                      decltype(std::declval<Deque<Unique> &>().crend())>));
  ASSERT_TRUE((
      std::is_same_v<Deque<Unique>::const_reverse_iterator,
                     decltype(std::declval<const Deque<Unique> &>().crend())>));

  ASSERT_TRUE((std::is_convertible_v<Deque<Unique>::iterator,
                                     Deque<Unique>::const_iterator>));
  ASSERT_TRUE((!std::is_convertible_v<Deque<Unique>::const_iterator,
                                      Deque<Unique>::iterator>));
}

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
  testIteratorArithmeticTypes<Deque<Unique>::iterator, Unique>();
  testIteratorArithmeticTypes<Deque<Unique>::const_iterator, const Unique>();
  testIteratorArithmeticTypes<Deque<Unique>::reverse_iterator, Unique>();
  testIteratorArithmeticTypes<Deque<Unique>::const_reverse_iterator,
                              const Unique>();
}

TEST_F(IteratorsTests, IteratorArithmetics) {

  ASSERT_EQ(smallDeque_.begin() + smallDeque_.size(), smallDeque_.end());
  ASSERT_EQ(smallDeque_.end() - smallDeque_.begin(), smallDeque_.size());
  ASSERT_EQ(smallDeque_.end() - smallDeque_.size(), smallDeque_.begin());

  {
    auto iter = smallDeque_.begin();
    for (size_t i = 0; i < smallDeque_.size(); ++i) {
      ASSERT_EQ(iter, smallDeque_.begin() + i);
      ++iter;
    }
    ASSERT_EQ(iter, smallDeque_.end());
  }

  {
    auto iter = smallDeque_.begin();
    for (size_t i = 0; i < smallDeque_.size(); ++i) {
      ASSERT_EQ(iter->number_, smallDeque_[i].number_);
      ++iter;
    }
  }

  {
    auto iter = smallDeque_.end() - 1;
    for (size_t i = 0; i < smallDeque_.size(); ++i) {
      ASSERT_EQ(iter->number_, smallDeque_[smallDeque_.size() - 1 - i].number_);
      if (i < smallDeque_.size() - 1) {
        --iter;
      }
    }
  }

  {
    auto iter_incr = smallDeque_.begin();
    auto iter_plus = smallDeque_.begin();
    for (; iter_incr != smallDeque_.end(); ++iter_incr, iter_plus += 1) {
      ASSERT_EQ(iter_incr, iter_plus);
    }
  }

  {
    for (size_t step = 1; step < smallDeque_.size(); ++step) {
      auto iter = smallDeque_.begin();
      for (size_t i = 0; i < smallDeque_.size(); i += step) {
        ASSERT_EQ(i, (iter + i)->number_);
      }
    }
  }
}

TEST_F(IteratorsTests, IteratorComparison) {
  {
    for (size_t i = 0; i < smallDeque_.size(); ++i) {
      ASSERT_GT(smallDeque_.end(), smallDeque_.begin() + i);
      ASSERT_GT(smallDeque_.cend(), smallDeque_.cbegin() + i);
      ASSERT_GT(smallDeque_.rend(), smallDeque_.rbegin() + i);
      ASSERT_GT(smallDeque_.crend(), smallDeque_.crbegin() + i);
    }

    ASSERT_EQ(smallDeque_.end(), smallDeque_.begin() + smallDeque_.size());
    ASSERT_EQ(smallDeque_.cend(), smallDeque_.cbegin() + smallDeque_.size());
    ASSERT_EQ(smallDeque_.rend(), smallDeque_.rbegin() + smallDeque_.size());
    ASSERT_EQ(smallDeque_.crend(), smallDeque_.crbegin() + smallDeque_.size());
  }

  {
    Deque<Unique> empty;
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
  Deque<int> d(1000);
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
    auto iter = smallDeque_.begin();
    for (size_t i = 0; i < smallDeque_.size(); ++i) {
      ASSERT_EQ(iter->number_, smallDeque_[i].number_);
      ++iter;
    }
  }

  {
    std::vector<Deque<UniqueDefaultConstructed>::iterator> iterators;
    for (size_t i = 0; i <= smallDeque_.size(); ++i) {
      iterators.push_back(smallDeque_.begin() + i);
    }

    size_t start = 0, end = smallDeque_.size();
    while (start < end) {
      smallDeque_.pop_front();
      ++start;
      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(iterators[i]->number_, i);
        ASSERT_EQ(iterators[i], smallDeque_.begin() + i - start);
        ASSERT_EQ(&*iterators[i], &smallDeque_[i - start]);
      }
      ASSERT_EQ(iterators[end], smallDeque_.end());
    }
  }

  {
    std::vector<Deque<UniqueDefaultConstructed>::iterator> iterators;
    for (size_t i = 0; i <= smallDeque_.size(); ++i) {
      iterators.push_back(smallDeque_.begin() + i);
    }

    size_t start = 0, end = smallDeque_.size();
    while (start < end) {
      smallDeque_.pop_back();
      --end;
      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(iterators[i]->number_, i);
        ASSERT_EQ(iterators[i], smallDeque_.begin() + i - start);
        ASSERT_EQ(&*iterators[i], &smallDeque_[i - start]);
      }
      ASSERT_EQ(iterators[end], smallDeque_.end());
    }
  }

  {
    std::vector<Deque<UniqueDefaultConstructed>::iterator> iterators;
    for (size_t i = 0; i <= smallDeque_.size(); ++i) {
      iterators.push_back(smallDeque_.begin() + i);
    }

    size_t start = 0, end = smallDeque_.size();
    for (size_t round = 0; start < end; ++round) {
      if (round % 2 == 0) {
        smallDeque_.pop_back();
        --end;
      } else {
        smallDeque_.pop_front();
        ++start;
      }

      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(iterators[i]->number_, i);
        ASSERT_EQ(iterators[i], smallDeque_.begin() + i - start);
        ASSERT_EQ(&*iterators[i], &smallDeque_[i - start]);
      }
      ASSERT_EQ(iterators[end], smallDeque_.end());
    }
  }
}

TEST_F(IteratorsTests, PointerAndReferenceInvalidationAfterPush) {
  {
    std::vector<UniqueDefaultConstructed *> pointers;
    for (size_t i = 0; i < smallDeque_.size(); ++i) {
      pointers.push_back(&smallDeque_[i]);
    }

    size_t start = 0, end = smallDeque_.size();
    for (size_t round = 0; round < 100; ++round) {
      if (round % 2 == 0) {
        smallDeque_.push_back(UniqueDefaultConstructed());
      } else {
        smallDeque_.push_front(UniqueDefaultConstructed());
        ++start;
        ++end;
      }

      for (size_t i = start; i < end; ++i) {
        ASSERT_EQ(pointers[i - start]->number_, i - start);
        ASSERT_EQ(pointers[i - start], &smallDeque_[i]);
        ASSERT_EQ(pointers[i - start], &*(smallDeque_.begin() + i));
      }
    }
  }
}

class InsertEraseTests : public ::testing::Test {
protected:
  const size_t kSmallSize = 10;
  const size_t kBigSize = 10000000;
  Deque<UniqueDefaultConstructed> smallDeque_;
  Deque<UniqueDefaultConstructed> bigDeque_;

  InsertEraseTests() {}
  virtual ~InsertEraseTests() {}
  virtual void SetUp() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
    smallDeque_ = Deque<UniqueDefaultConstructed>(kSmallSize);
    bigDeque_ = Deque<UniqueDefaultConstructed>(kBigSize);
  }
  virtual void TearDown() {
    Accountant::ResetAll();
    Unique::Reset();
    UniqueDefaultConstructed::Reset();
  }
};

TEST_F(InsertEraseTests, InsertErase) {
  {
    smallDeque_.insert(smallDeque_.begin() + kSmallSize / 2,
                       UniqueDefaultConstructed());
    ASSERT_EQ(kSmallSize + 1, smallDeque_.size());
    for (size_t i = 0; i < kSmallSize / 2; ++i) {
      ASSERT_EQ(i, smallDeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize, smallDeque_[kSmallSize / 2].number_);
    for (size_t i = kSmallSize / 2; i < kSmallSize; ++i) {
      ASSERT_EQ(i, smallDeque_[i + 1].number_);
    }
  }

  {
    bigDeque_.insert(bigDeque_.begin() + kBigSize / 2,
                     UniqueDefaultConstructed());
    ASSERT_EQ(kBigSize + 1, bigDeque_.size());
    for (size_t i = 0; i < kBigSize / 2; ++i) {
      ASSERT_EQ(i + kSmallSize, bigDeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize + 1, bigDeque_[kBigSize / 2].number_);
    for (size_t i = kBigSize / 2; i < kBigSize; ++i) {
      ASSERT_EQ(i + kSmallSize, bigDeque_[i + 1].number_);
    }
  }

  {
    smallDeque_.erase(smallDeque_.begin() + kSmallSize / 2 - 1);
    ASSERT_EQ(kSmallSize, smallDeque_.size());
    for (size_t i = 0; i < kSmallSize / 2 - 1; ++i) {
      ASSERT_EQ(i, smallDeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize, smallDeque_[kSmallSize / 2 - 1].number_);
    for (size_t i = kSmallSize / 2; i < kSmallSize; ++i) {
      ASSERT_EQ(i, smallDeque_[i].number_);
    }
  }

  {
    bigDeque_.erase(bigDeque_.begin() + kBigSize / 2 - 1);
    ASSERT_EQ(kBigSize, bigDeque_.size());
    for (size_t i = 0; i < kBigSize / 2 - 1; ++i) {
      ASSERT_EQ(i + kSmallSize, bigDeque_[i].number_);
    }
    ASSERT_EQ(kSmallSize + kBigSize + 1, bigDeque_[kBigSize / 2 - 1].number_);
    for (size_t i = kBigSize / 2; i < kBigSize; ++i) {
      ASSERT_EQ(i + kSmallSize, bigDeque_[i].number_);
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
  ASSERT_THROW([]() { Deque<Counted<17>> d(100); }(), CountedException);
  try {
    Deque<Counted<17>> d(100);
  } catch (CountedException &) {
    ASSERT_EQ(Counted<17>::counter, 0);
  }

  ASSERT_THROW([]() { Deque<Explosive> d(100); }(), int);
  ASSERT_NO_THROW([]() { Deque<Explosive> d; }());
  ASSERT_FALSE(Explosive::exploded);

  ASSERT_THROW(
      []() {
        Deque<Explosive> d;
        auto safe = Explosive(Explosive::Safeguard{});
        d.push_back(safe);
      }(),
      int);
  ASSERT_FALSE(Explosive::exploded);
}

TEST_F(ExceptionTests, StrongGuarantee) {
  const size_t size = 20'000;
  const size_t initial_data = 100;
  Deque<Fragile> d(size, Fragile(size, initial_data));

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