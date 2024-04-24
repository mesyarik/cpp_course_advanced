// #include <unordered_map>
#include "unordered_map.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include "tiny_test.hpp"

using testing::make_test;
using testing::PrettyTest;
using testing::TestGroup;
using groups_t = std::vector<TestGroup>;
using namespace std::views;
namespace rng = std::ranges;

// template <typename Key, typename Value, typename Hash = std::hash<Key>,
//           typename EqualTo = std::equal_to<Key>,
//           typename Alloc = std::allocator<std::pair<const Key, Value>>>
// using UnorderedMap = std::unordered_map<Key, Value, Hash, EqualTo, Alloc>;


namespace TestsByMesyarik {

// Just a simple SFINAE trick to check CE presence when it's necessary
// Stay tuned, we'll discuss this kind of tricks in our next lectures ;)
template <typename T>
decltype(UnorderedMap<T, T>().cbegin()->second = 0, int())
TestConstIteratorDoesntAllowModification(T) {
  assert(false);
}
template <typename... FakeArgs>
void TestConstIteratorDoesntAllowModification(FakeArgs...) {}

struct VerySpecialType {
  int x = 0;
  explicit VerySpecialType(int x)
    : x(x) {}
  VerySpecialType(const VerySpecialType&) = delete;
  VerySpecialType& operator=(const VerySpecialType&) = delete;

  VerySpecialType(VerySpecialType&&) = default;
  VerySpecialType& operator=(VerySpecialType&&) = default;
};

struct NeitherDefaultNorCopyConstructible {
  VerySpecialType x;

  NeitherDefaultNorCopyConstructible() = delete;
  NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
  NeitherDefaultNorCopyConstructible& operator=(const NeitherDefaultNorCopyConstructible&) = delete;

  NeitherDefaultNorCopyConstructible(VerySpecialType&& x)
    : x(std::move(x)) {}
  NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&&) = default;
  NeitherDefaultNorCopyConstructible& operator=(NeitherDefaultNorCopyConstructible&&) = default;

  bool operator==(const NeitherDefaultNorCopyConstructible& other) const {
    return x.x == other.x.x;
  }
};
}  // namespace TestsByMesyarik

namespace std {
template <>
struct hash<TestsByMesyarik::NeitherDefaultNorCopyConstructible> {
  size_t operator()(const TestsByMesyarik::NeitherDefaultNorCopyConstructible& x) const {
    return std::hash<int>()(x.x.x);
  }
};
}  // namespace std

namespace TestsByMesyarik {

template <typename T>
struct MyHash {
  size_t operator()(const T& p) const { return std::hash<int>()(p.second / p.first); }
};

template <typename T>
struct MyEqual {
  bool operator()(const T& x, const T& y) const { return y.second / y.first == x.second / x.first; }
};

struct OneMoreStrangeStruct {
  int first;
  int second;
};

bool operator==(const OneMoreStrangeStruct&, const OneMoreStrangeStruct&) = delete;

TestGroup create_basic_tests() {
  return {
    "Basic implementation tests",
    make_test<PrettyTest>("simple",
                          [&](auto& test) {
                            // std::cerr << "starting simple test" << std::endl;
                            UnorderedMap<std::string, int> m;

                            m["aaaaa"] = 5;
                            m["bbb"] = 6;
                            // std::cerr << "two first assignments with [] passed" <<
                            // std::endl;
                            m.at("bbb") = 7;
                            // std::cerr << "assignment with at() passed" << std::endl;
                            test.equals(m.size(), static_cast<size_t>(2));

                            test.equals(m["aaaaa"], 5);
                            test.equals(m["bbb"], 7);
                            test.equals(m["ccc"], 0);

                            test.equals(m.size(), static_cast<size_t>(3));

                            // std::cerr << "assertions passed; before try block now" <<
                            // std::endl;

                            try {
                              m.at("xxxxxxxx");
                              test.check(false);
                            } catch (...) {
                              // std::cerr << "in catch block" << std::endl;
                            }

                            auto it = m.find("dddd");
                            test.equals(it, m.end());
                            // std::cerr << "finding dddd passed, it == m.end()" << std::endl;

                            it = m.find("bbb");
                            test.equals(it->second, 7);
                            // std::cerr << "finding bbb passed, it->second == 7" <<
                            // std::endl;
                            ++it->second;
                            test.equals(it->second, 8);
                            // std::cerr << "incrementing it->second passed, it->second==8" <<
                            // std::endl;

                            for (auto& item : m) {
                              --item.second;
                            }
                            test.equals(m.at("aaaaa"), 4);
                            // std::cerr << "decrementing each value passed, value at aaaaa ==
                            // 4" << std::endl;

                            {
                              auto mm = m;
                              // std::cerr << "m copied into mm" << std::endl;
                              m = std::move(mm);
                              // std::cerr << "mm moved into m" << std::endl;
                            }
                            // std::cerr << "copying and moving passed" << std::endl;

                            auto res = m.emplace("abcde", 2);
                            test.check(res.second);
                            // std::cerr << "emplacing abcde passed, value at abcde == 2" <<
                            // std::endl;
                          }),

    make_test<PrettyTest>(
      "iterators",
      [&](auto& test) {
        UnorderedMap<double, std::string> m;

        std::vector<double> keys = {0.4, 0.3, -8.32, 7.5, 10.0, 0.0};
        std::vector<std::string> values = {
          "Summer has come and passed",     "The innocent can never last",
          "Wake me up when September ends", "Like my fathers come to pass",
          "Seven years has gone so fast",   "Wake me up when September ends",
        };

        m.reserve(1'000'000);

        for (int i = 0; i < 6; ++i) {
          m.insert({keys[i], values[i]});
        }

        auto beg = m.cbegin();
        std::string s = beg->second;
        auto it = m.begin();
        ++it;
        m.erase(it++);
        it = m.begin();
        m.erase(++it);

        test.equals(beg->second, s);
        test.equals(m.size(), static_cast<size_t>(4));

        UnorderedMap<double, std::string> mm;
        std::vector<std::pair<const double, std::string>> elements = {
          {3.0, values[0]}, {5.0, values[1]}, {-10.0, values[2]}, {35.7, values[3]}};
        mm.insert(elements.cbegin(), elements.cend());
        s = mm.begin()->second;

        m.insert(mm.begin(), mm.end());
        test.equals(mm.size(), static_cast<size_t>(4));
        test.equals(mm.begin()->second, s);


        // Test traverse efficiency
        m.reserve(1'000'000);  // once again, nothing really should happen
        test.equals(m.size(), static_cast<size_t>(8));
        // Actions below must be quick (~ 1000 * 8 operations) despite reserving space for 1M
        // elements
        for (int i = 0; i < 10000; ++i) {
          long long h = 0;
          for (auto it = m.cbegin(); it != m.cend(); ++it) {
            // just some senseless action
            h += int(it->first) + int((it->second)[0]);
          }
          std::ignore = h;
        }

        it = m.begin();
        ++it;
        s = it->second;
        // I asked to reserve space for 1M elements so actions below adding 100'000 elements
        // mustn't cause reallocation
        for (double d = 100.0; d < 10100.0; d += 0.1) {
          m.emplace(d, "a");
        }
        // And my iterator must point to the same object as before
        test.equals(it->second, s);

        auto dist = std::distance(it, m.end());
        auto sz = m.size();
        m.erase(it, m.end());
        test.equals(sz - dist, m.size());

        // Must be also fast
        for (double d = 200.0; d < 10200.0; d += 0.35) {
          auto it = m.find(d);
          if (it != m.end()) m.erase(it);
        }
      }),

    make_test<PrettyTest>(
      "no redundant copies",
      [&](auto& test) {
        // std::cerr << "Test no redundant copies started" << std::endl;
        UnorderedMap<NeitherDefaultNorCopyConstructible, NeitherDefaultNorCopyConstructible> m;
        // std::cerr << "m created" << std::endl;
        m.reserve(10);
        // std::cerr << "m.reserve(10) done" << std::endl;
        m.emplace(VerySpecialType(0), VerySpecialType(0));
        // std::cerr << "m.emplace(VerySpecialType(0), VerySpecialType(0)) done" << std::endl;
        m.reserve(1'000'000);
        // std::cerr << "m.reserve(1000000) done" << std::endl;
        std::pair<NeitherDefaultNorCopyConstructible, NeitherDefaultNorCopyConstructible> p{
          VerySpecialType(1), VerySpecialType(1)};

        m.insert(std::move(p));
        // std::cerr << "m.insert(std::move(p)) done" << std::endl;

        test.equals(m.size(), static_cast<size_t>(2));

        // this shouldn't compile
        // m[VerySpecialType(0)] = VerySpecialType(1);

        // but this should
        m.at(VerySpecialType(1)) = VerySpecialType(0);
        // std::cerr << "m.at(VerySpecialType(1)) = VerySpecialType(0) done" << std::endl;

        {
          auto mm = std::move(m);
          // std::cerr << "m moved to mm" << std::endl;
          m = std::move(mm);
          // std::cerr << "mm moved to m" << std::endl;
        }
        // std::cerr << "the scope of mm has finished" << std::endl;
        m.erase(m.begin());
        // std::cerr << "m.erase(m.begin()) done" << std::endl;
        m.erase(m.begin());
        // std::cerr << "m.erase(m.begin()) done once again" << std::endl;
        test.equals(m.size(), static_cast<size_t>(0));
      }),
    make_test<PrettyTest>("custom hash and compare",
                          [&](auto& test) {
                            UnorderedMap<std::pair<int, int>, char, MyHash<std::pair<int, int>>,
                                         MyEqual<std::pair<int, int>>>
                              m;

                            m.insert({{1, 2}, 0});
                            m.insert({{2, 4}, 1});
                            test.equals(m.size(), static_cast<size_t>(1));

                            m[{3, 6}] = 3;
                            test.equals(m.at({4, 8}), 3);

                            UnorderedMap<OneMoreStrangeStruct, int, MyHash<OneMoreStrangeStruct>,
                                         MyEqual<OneMoreStrangeStruct>>
                              mm;
                            mm[{1, 2}] = 3;
                            test.equals(mm.at({5, 10}), 3);

                            mm.emplace(OneMoreStrangeStruct{3, 9}, 2);
                            test.equals(mm.size(), static_cast<size_t>(2));
                            mm.reserve(1'000);
                            mm.erase(mm.begin());
                            mm.erase(mm.begin());
                            for (int k = 1; k < 100; ++k) {
                              for (int i = 1; i < 10; ++i) {
                                mm.insert({{i, k * i}, 0});
                              }
                            }
                            std::string ans;
                            std::string myans;
                            for (auto it = mm.cbegin(); it != mm.cend(); ++it) {
                              ans += std::to_string(it->second);
                              myans += '0';
                            }
                            test.equals(ans, myans);
                          })

  };
}

// Finally, some tricky fixtures to test custom allocator.
// Done by professional, don't try to repeat
class Chaste {
private:
  int x = 0;
  Chaste() = default;
  Chaste(int x)
    : x(x) {}

  // Nobody can construct me except this guy
  template <typename T>
  friend struct TheChosenOne;

public:
  Chaste(const Chaste&) = default;
  Chaste(Chaste&&) = default;

  bool operator==(const Chaste& other) const { return x == other.x; }
};
}  // namespace TestsByMesyarik

namespace std {
template <>
struct hash<TestsByMesyarik::Chaste> {
  size_t operator()(const TestsByMesyarik::Chaste& x) const noexcept {
    return std::hash<int>()(reinterpret_cast<const int&>(x));
  }
};
}  // namespace std

namespace TestsByMesyarik {

template <typename T>
struct TheChosenOne : public std::allocator<T> {
  TheChosenOne() {}

  template <typename U>
  TheChosenOne(const TheChosenOne<U>&) {}

  template <typename... Args>
  void construct(T* p, Args&&... args) const {
    new (p) T(std::forward<Args>(args)...);
  }

  void construct(std::pair<const Chaste, Chaste>* p, int a, int b) const {
    new (p) std::pair<const Chaste, Chaste>(Chaste(a), Chaste(b));
  }

  void destroy(T* p) const { p->~T(); }

  template <typename U>
  struct rebind {
    using other = TheChosenOne<U>;
  };
};

/*
template<>
struct TheChosenOne<std::pair<const Chaste, Chaste>>
        : public std::allocator<std::pair<const Chaste, Chaste>> {
    using PairType = std::pair<const Chaste, Chaste>;

    TheChosenOne() {}

    template<typename U>
    TheChosenOne(const TheChosenOne<U>&) {}

    void construct(PairType* p, int a, int b) const {
        new(p) PairType(Chaste(a), Chaste(b));
    }

    void destroy(PairType* p) const {
        p->~PairType();
    }

    template<typename U>
    struct rebind {
        using other = TheChosenOne<U>;
    };
};
*/

TestGroup create_special_allocator_tests() {
  return {"Special allocator tests", make_test<PrettyTest>("special allocator", [&](auto&) {
            // This container mustn't construct or destroy any objects without using TheChosenOne
            // allocator
            UnorderedMap<Chaste, Chaste, std::hash<Chaste>, std::equal_to<Chaste>,
                         TheChosenOne<std::pair<const Chaste, Chaste>>>
              m;

            m.emplace(0, 0);

            {
              auto mm = m;
              mm.reserve(1'000);
              mm.erase(mm.begin());
            }

            for (int i = 0; i < 1'000'000; ++i) {
              m.emplace(i, i);
            }

            for (int i = 0; i < 500'000; ++i) {
              auto it = m.begin();
              ++it, ++it;
              m.erase(m.begin(), it);
            }
          })};
}
}  // namespace TestsByMesyarik

namespace TestsByUnrealf1 {
struct Data {
  int data;

  auto operator<=>(const Data&) const = default;
};

struct Trivial : Data {};
constexpr Trivial operator""_tr(unsigned long long int x) { return Trivial{int(x)}; }

struct NonTrivial : Data {
  NonTrivial() {}
  NonTrivial(int x) { data = x; }
};
NonTrivial operator""_ntr(unsigned long long int x) { return NonTrivial{int(x)}; }

struct NotDefaultConstructible {
  NotDefaultConstructible() = delete;
  NotDefaultConstructible(int input)
    : data(input) {}
  int data;

  auto operator<=>(const NotDefaultConstructible&) const = default;
};

struct NeitherDefaultNorCopyConstructible {
  NeitherDefaultNorCopyConstructible() = delete;
  NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
  NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&& other)
    : data(other.data),
      moved(true){};
  NeitherDefaultNorCopyConstructible(int input)
    : data(input),
      moved(false) {}
  int data;
  const bool moved = false;

  auto operator<=>(const NeitherDefaultNorCopyConstructible&) const = default;
};

template <typename T>
struct NotPropagatedCountingAllocator {
  size_t allocates_counter = 0;
  size_t deallocates_counter = 0;

  using propagate_on_container_move_assignment = std::false_type;
  using value_type = T;

  NotPropagatedCountingAllocator() = default;

  template <typename U>
  NotPropagatedCountingAllocator(const NotPropagatedCountingAllocator<U>& other)
    : allocates_counter(other.allocates_counter),
      deallocates_counter(other.deallocates_counter) {}

  template <typename U>
  NotPropagatedCountingAllocator(NotPropagatedCountingAllocator<U>&& other)
    : allocates_counter(other.allocates_counter),
      deallocates_counter(other.deallocates_counter) {
    other.allocates_counter = 0;
    other.deallocates_counter = 0;
  }

  bool operator==(const NotPropagatedCountingAllocator<T>&) const { return false; }

  T* allocate(size_t n) {
    ++allocates_counter;
    return std::allocator<T>().allocate(n);
  }

  void deallocate(T* pointer, size_t n) {
    ++deallocates_counter;
    return std::allocator<T>().deallocate(pointer, n);
  }
};

const size_t small_size = 17;
const size_t medium_size = 100;

constexpr size_t operator""_sz(unsigned long long int x) { return size_t(x); }

template <typename Value>
auto make_small_map() {
  UnorderedMap<int, Value> map;
  for (int i = 0; i < int(small_size); ++i) {
    map.emplace(i, Value{i});
  }
  return map;
}

bool maps_equal(const auto& left, const auto& right) {
  return rng::all_of(left, [&right](const auto& pr) { return right.at(pr.first) == pr.second; });
}

// NOLINTNEXTLINE
TestGroup create_constructor_tests() {
  return {
    "construction and assignment",
    make_test<PrettyTest>("default",
                          [](auto& test) {
                            UnorderedMap<int, int> defaulted;
                            test.equals(defaulted.size(), 0_sz);
                            UnorderedMap<int, NotDefaultConstructible> without_default;
                            test.equals(without_default.size(), 0_sz);
                          }),

    make_test<PrettyTest>("copy and move",
                          [&](auto& test) {
                            {
                              auto map = make_small_map<Trivial>();
                              auto copy = map;
                              test.check(maps_equal(copy, map));
                              auto move_copy = std::move(map);
                              test.check(maps_equal(copy, move_copy));
                              test.equals(map.size(), 0_sz);
                            }
                            {
                              auto map = make_small_map<NonTrivial>();
                              auto copy = map;
                              test.check(maps_equal(copy, map));
                              auto move_copy = std::move(map);
                              test.check(maps_equal(copy, move_copy));
                              test.equals(map.size(), 0_sz);
                            }
                          }),

    make_test<PrettyTest>("assignment operators",
                          [](auto& test) {
                            auto map = make_small_map<Trivial>();
                            test.equals(map.size(), small_size);
                            UnorderedMap<int, Trivial> map2;
                            test.equals(map2.size(), 0_sz);

                            map2 = map;
                            test.check(maps_equal(map, map2));
                            map2 = std::move(map);
                            test.equals(map.size(), 0_sz);
                            test.equals(map2.size(), small_size);
                          }),

    make_test<PrettyTest>(
      "move assignment with unequal and not propagating allocator",
      [](auto& test) {
        using custom_alloc_type =
          NotPropagatedCountingAllocator<std::pair<const int, NeitherDefaultNorCopyConstructible>>;
        using special_map_type =
          UnorderedMap<int, NeitherDefaultNorCopyConstructible, std::hash<int>, std::equal_to<int>,
                       custom_alloc_type>;
        special_map_type map;
        map.emplace(1, NeitherDefaultNorCopyConstructible(1));
        auto address1 = std::addressof(*map.begin());
        special_map_type map_moved;
        test.check(map_moved.get_allocator() != map.get_allocator());
        map_moved = std::move(map);
        auto address2 = std::addressof(*map_moved.begin());
        test.check(map_moved.at(1).moved);
        test.check(address1 != address2);
      }),

    make_test<PrettyTest>("swap",
                          [](auto& test) {
                            auto map = make_small_map<Trivial>();
                            decltype(map) another;
                            auto it = map.find(1);
                            auto address = &(*it);
                            test.equals(it->second, 1_tr);
                            map.swap(another);
                            test.equals(it->second, 1_tr);
                            test.equals(address->second, 1_tr);
                          })

  };
}

// NOLINTNEXTLINE
TestGroup create_modification_tests() {
  return {
    "modification",
    make_test<PrettyTest>("emplace",
                          [](auto& test) {
                            UnorderedMap<int, NonTrivial> map;
                            auto [place, did_insert] = map.emplace(1, 1_ntr);
                            test.equals(map.at(1), 1_ntr);
                            test.equals(place, map.begin());
                            test.check(did_insert);
                            auto [new_place, new_did_insert] = map.emplace(2, 2_ntr);
                            // update place as it could be invalidated by rehash
                            place = map.find(1);
                            test.check(place != new_place);
                            test.check(new_did_insert);
                            test.equals(map.at(2), 2_ntr);
                            test.equals(map.at(1), 1_ntr);
                            auto [old_place, reinsert] = map.emplace(1, 3_ntr);
                            test.check(!reinsert);
                            test.equals(old_place, place);
                            test.equals(map.at(1), 1_ntr);
                            test.equals(map.at(2), 2_ntr);
                          }),

    make_test<PrettyTest>("emplace move",
                          [](auto& test) {
                            UnorderedMap<std::string, std::string> moving_map;
                            std::string a = "a";
                            std::string b = "b";
                            std::string c = "c";
                            moving_map.emplace(a, a);
                            test.equals(a, "a");
                            moving_map.emplace(std::move(b), a);
                            test.equals(a, "a");
                            test.equals(b, "");
                            moving_map.emplace(std::move(c), std::move(a));
                            test.equals(a, "");
                            test.equals(c, "");
                            test.equals(moving_map.size(), 3_sz);
                            test.equals(moving_map.at("a"), "a");
                            test.equals(moving_map.at("b"), "a");
                            test.equals(moving_map.at("c"), "a");
                          }),

    make_test<PrettyTest>("insert nontrivial",
                          [](auto& test) {
                            UnorderedMap<int, NonTrivial> map;
                            auto [place, did_insert] = map.insert({1, 1_ntr});
                            test.equals(map.at(1), 1_ntr);
                            test.equals(place, map.begin());
                            test.check(did_insert);
                            auto [new_place, new_did_insert] = map.insert({2, 2_ntr});
                            place = map.find(1);
                            test.check(place != new_place);
                            test.check(new_did_insert);
                            test.equals(map.at(2), 2_ntr);
                            test.equals(map.at(1), 1_ntr);
                            auto [old_place, reinsert] = map.insert({1, 3_ntr});
                            test.check(!reinsert);
                            test.equals(old_place, place);
                            test.equals(map.at(1), 1_ntr);
                            test.equals(map.at(2), 2_ntr);
                          }),

    make_test<PrettyTest>("insert move",
                          [](auto& test) {
                            UnorderedMap<std::string, std::string> moving_map;
                            using node = std::pair<std::string, std::string>;

                            node a{"a", "a"};
                            node b{"b", "b"};

                            moving_map.insert(a);
                            test.equals("a", a.first);
                            test.equals(moving_map.size(), 1_sz);

                            moving_map.insert(std::move(b));
                            test.equals("", b.first);
                            test.equals(moving_map.size(), 2_sz);

                            test.equals(moving_map.at("a"), "a");
                            test.equals(moving_map.at("b"), "b");
                          }),

    make_test<PrettyTest>("insert range",
                          [](auto& test) {
                            UnorderedMap<int, NonTrivial> map;
                            /*auto range = iota(0, int(medium_size))
                                | transform([](int item) -> std::pair<const int, NonTrivial> {
                               return {item, {item}}; } ) | common;*/
                            std::vector<std::pair<int, NonTrivial>> range;
                            for (int i = 0; i < int(medium_size); ++i) {
                              range.emplace_back(i, NonTrivial{i});
                            }
                            map.insert(range.begin(), range.end());
                            /*test.check(rng::all_of(iota(0, int(medium_size)), [&](int item) {
                                return map.at(item) == NonTrivial{item};
                            }));*/
                            std::vector<int> indices(small_size);
                            std::iota(indices.begin(), indices.end(), 0);
                            test.check(std::all_of(indices.begin(), indices.end(), [&](int item) {
                              return map.at(item) == NonTrivial{item};
                            }));
                          }),

    make_test<PrettyTest>("move insert range", [](auto& test) {
      UnorderedMap<int, std::string> map;
      std::vector<int> indices(small_size);
      std::iota(indices.begin(), indices.end(), 0);
      /*auto range = iota(0, int(small_size))
          | transform([](int item) -> std::pair<const int, std::string> {
                  return {item, std::to_string(item)};
          })
          | common;*/
      std::vector<std::pair<int, std::string>> storage;
      std::transform(indices.begin(), indices.end(), std::back_inserter(storage), [](int idx) {
        return std::pair<const int, std::string>{idx, std::to_string(idx)};
      });
      map.insert(storage.begin(), storage.end());
      test.check(
        rng::all_of(storage, [](auto& pr) { return std::to_string(pr.first) == pr.second; }));
      map = UnorderedMap<int, std::string>();
      map.insert(std::move_iterator(storage.begin()), std::move_iterator(storage.end()));
      test.equals(storage.size(), small_size);
      test.check(rng::all_of(storage, [&](auto& p) { return test.equals(p.second, ""); }));
    })};
}

TestGroup create_access_tests() {
  return {"access",
          make_test<PrettyTest>(
            ".at and []",
            [](auto& test) {
              /*auto range = iota(0, int(small_size))
                  | transform([](int item) -> std::pair<const int, std::string> {
                          return {item, std::to_string(item)};
                  })
                  | common;*/
              std::vector<int> indices(small_size);
              std::iota(indices.begin(), indices.end(), 0);
              std::vector<std::pair<int, std::string>> range;
              std::transform(indices.begin(), indices.end(), std::back_inserter(range),
                             [](int idx) {
                               return std::pair<const int, std::string>{idx, std::to_string(idx)};
                             });

              std::iota(indices.begin(), indices.end(), 0);
              UnorderedMap<int, std::string> map;
              map.insert(range.begin(), range.end());
              for (int idx : indices) {
                test.equals(std::to_string(idx), map.at(idx));
                test.equals(std::to_string(idx), map[idx]);
              }
              try {
                map.at(-1) = "abacaba";
                test.fail();
              } catch (...) {}
              map[-1] = "abacaba";
              test.equals(map.at(-1), "abacaba");
              map.at(-1) = "qwerty";
              test.equals(map[-1], "qwerty");
            }),

          make_test<PrettyTest>("[] move",
                                [](auto& test) {
                                  UnorderedMap<std::string, std::string> map;
                                  /*auto range = iota(0, int(small_size))
                                      | transform([](int item) {
                                              return std::to_string(item);
                                      })
                                      | common;*/
                                  std::vector<int> indices(small_size);
                                  std::iota(indices.begin(), indices.end(), 0);
                                  std::vector<std::string> storage;
                                  std::transform(indices.begin(), indices.end(),
                                                 std::back_inserter(storage),
                                                 [](int idx) { return std::to_string(idx); });
                                  test.check(rng::all_of(indices, [&](int i) {
                                    return std::to_string(i) == storage[size_t(i)];
                                  }));
                                  map[std::move(storage[0])] = std::move(storage[1]);
                                  test.equals(storage[0], "");
                                  test.equals(storage[1], "");
                                  map[std::move(storage[2])] = storage[3];
                                  test.equals(storage[2], "");
                                  test.equals(storage[3], "3");
                                  map[storage[3]] = std::move(storage[4]);
                                  test.equals(storage[3], "3");
                                  test.equals(storage[4], "");
                                }),

          make_test<PrettyTest>("find",
                                [](auto& test) {
                                  auto map = make_small_map<Trivial>();
                                  auto existing = map.find(1);
                                  test.equals(existing->second, 1_tr);
                                  auto non_existing = map.find(-1);
                                  test.equals(non_existing, map.end());
                                }),

          make_test<PrettyTest>("bucket borders check", [](auto& test) {
            UnorderedMap<int, int, decltype([](int element) -> size_t {
                           return static_cast<size_t>(abs(element) % 10);
                         })>
              map;
            map.emplace(1, 1);
            map.emplace(11, 11);
            test.check(map.find(1) != map.end());
            test.check(map.find(11) != map.end());
            test.check(map.find(11) != map.find(1));
          })};
}

TestGroup create_misc_tests() {
  return {"misc", make_test<PrettyTest>("load factor", [](auto& test) {
            auto map = make_small_map<Trivial>();
            // auto max_val = rng::max(map | keys);
            auto max_val = std::max_element(map.begin(), map.end(), [](auto& left, auto& right) {
                             return left.first < right.first;
                           })->first;
            test.check(map.load_factor() > 0.0f);
            auto new_load_factor = map.load_factor() / 2.0f;
            map.max_load_factor(new_load_factor);
            // for (auto i : iota(max_val + 1) | take(medium_size)) {
            for (auto i = max_val + 1; i < max_val + 1 + int(medium_size); ++i) {
              auto [_, inserted] = map.emplace(i, Trivial{i});
              test.check(inserted);
              test.check(map.load_factor() > 0.0f);
              test.check(map.load_factor() <= new_load_factor);
            }
          })};
}
}  // namespace TestsByUnrealf1

namespace TestsByDarkCodeForce {
struct NeitherDefaultNorCopyConstructible {
  int data;
  const bool moved = false;

  NeitherDefaultNorCopyConstructible() = delete;
  NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
  NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&& other)
    : data(other.data),
      moved(true) {
    other.data = 0;
  };
  NeitherDefaultNorCopyConstructible(int input)
    : data(input),
      moved(false) {}

  auto operator<=>(const NeitherDefaultNorCopyConstructible&) const = default;
};


template <typename T, typename POCMA, typename AlwaysEqual>
struct POCMAConfigurableAllocator {
  size_t allocates_counter = 0;
  size_t deallocates_counter = 0;

  using propagate_on_container_move_assignment = POCMA;
  using value_type = T;

  POCMAConfigurableAllocator() = default;

  template <typename U>
  POCMAConfigurableAllocator(const POCMAConfigurableAllocator<U, POCMA, AlwaysEqual>& other)
    : allocates_counter(other.allocates_counter),
      deallocates_counter(other.deallocates_counter) {}

  template <typename U>
  POCMAConfigurableAllocator(POCMAConfigurableAllocator<U, POCMA, AlwaysEqual>&& other)
    : allocates_counter(other.allocates_counter),
      deallocates_counter(other.deallocates_counter) {
    other.allocates_counter = 0;
    other.deallocates_counter = 0;
  }

  using is_always_equal = AlwaysEqual;
  bool operator==(const POCMAConfigurableAllocator<T, POCMA, AlwaysEqual>&) const { return false; }

  T* allocate(size_t n) {
    ++allocates_counter;
    return std::allocator<T>().allocate(n);
  }

  void deallocate(T* pointer, size_t n) {
    ++deallocates_counter;
    return std::allocator<T>().deallocate(pointer, n);
  }
};

template <typename POCMA, typename AlwaysEqual>
      using pocma_map_type = UnorderedMap<
        NeitherDefaultNorCopyConstructible, int,
        decltype([](const NeitherDefaultNorCopyConstructible&) { return 1; }),
        std::equal_to<NeitherDefaultNorCopyConstructible>,
        POCMAConfigurableAllocator<std::pair<const NeitherDefaultNorCopyConstructible, int>, POCMA,
                                   AlwaysEqual>>;

TestGroup create_pocma_allocator_tests() {
  return {
    "POCMA Allocator tests", make_test<PrettyTest>("POCMA and AlwaysEqual", [&](auto&) {
      pocma_map_type<std::true_type, std::true_type> mtt1;
      pocma_map_type<std::true_type, std::true_type> mtt2;
      mtt2 = std::move(mtt1); // just as with std::allocator

      pocma_map_type<std::true_type, std::false_type> mtf1;
      pocma_map_type<std::true_type, std::false_type> mtf2;
      mtf2 = std::move(mtf1); // just as with std::allocator but now they are not equal

      pocma_map_type<std::false_type, std::true_type> mft1;
      pocma_map_type<std::false_type, std::true_type> mft2;
      mft2 = std::move(mft1); // just as with std::allocator but now they are not propagated

      // pocma_map_type<std::false_type, std::false_type> mff1;
      // pocma_map_type<std::false_type, std::false_type> mff2;
      // mff2 = std::move(mff1); // and this should not compile
    })
  };
}

};  // namespace TestsByDarkCodeForce

int main() {
  groups_t groups{};

  groups.push_back(TestsByMesyarik::create_basic_tests());
  groups.push_back(TestsByMesyarik::create_special_allocator_tests());

  groups.push_back(TestsByUnrealf1::create_constructor_tests());
  groups.push_back(TestsByUnrealf1::create_modification_tests());
  groups.push_back(TestsByUnrealf1::create_access_tests());
  groups.push_back(TestsByUnrealf1::create_misc_tests());

  groups.push_back(TestsByDarkCodeForce::create_pocma_allocator_tests());

  bool res = true;
  for (auto& group : groups) {
    res &= group.run();
  }

  return res ? 0 : 1;
}
