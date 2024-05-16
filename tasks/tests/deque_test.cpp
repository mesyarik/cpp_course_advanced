#include <type_traits>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <random>
#include <iostream>
#include <cassert>
#include <deque>

//#include "deque.h"

template <typename T>
using Deque = std::deque<T>;

namespace TestsByMesyarik {

void test1() {
    Deque<int> d(10, 3);

    d[3] = 5;

    d[7] = 8;

    d[9] = 10;

    std::string s = "33353338310";
    std::string ss;
    Deque<int> dd;

    {
        Deque<int> d2 = d;

        dd = d2;
    }

    d[1] = 2;

    d.at(2) = 1;

    try {
        d.at(10) = 0;
        assert(false);
    } catch (std::out_of_range&) {}

    const Deque<int>& ddd = dd;
    for (size_t i = 0; i < ddd.size(); ++i) {
        ss += std::to_string(ddd[i]);
    }

    assert(s == ss);
}

void test2() {
    Deque<int> d(1);

    d[0] = 0;
    
    for (int i = 0; i < 8; ++i) {
        d.push_back(i);
        d.push_front(i);
    }

    for (int i = 0; i < 12; ++i) {
        d.pop_front();
    }
    
    d.pop_back();
    assert(d.size() == 4);

    std::string ss;

    for (size_t i = 0; i < d.size(); ++i) {
        ss += std::to_string(d[i]);
    }
    
    assert(ss == "3456");
}

void test3() {
    Deque<int> d;

    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            
            if (j % 3 == 2) {
                d.pop_back();
            } else {
                d.push_front(i*j);
            }

        }
    }

    assert(d.size() == 334'000);

    Deque<int>::iterator left = d.begin() + 100'000;
    Deque<int>::iterator right = d.end() - 233'990;
    while (d.begin() != left) d.pop_front();
    while (d.end() != right) d.pop_back();

    assert(d.size() == 10);

    assert(right - left == 10);

    std::string s;
    for (auto it = left; it != right; ++it) {
        ++*it;
    }
    for (auto it = right - 1; it >= left; --it) {
        s += std::to_string(*it);
    }
    
    assert(s == "51001518515355154401561015695158651595016120162051");
}

struct S {
    int x = 0;
    double y = 0.0;
};

void test4() {
    
    Deque<S> d(5, {1, 2.0});
    const Deque<S>& cd = d;

    static_assert(!std::is_assignable_v<decltype(*cd.begin()), S>);
    static_assert(std::is_assignable_v<decltype(*d.begin()), S>);
    static_assert(!std::is_assignable_v<decltype(*d.cbegin()), S>);

    static_assert(!std::is_assignable_v<decltype(*cd.end()), S>);
    static_assert(std::is_assignable_v<decltype(*d.end()), S>);
    static_assert(!std::is_assignable_v<decltype(*d.cend()), S>);

    assert(cd.size() == 5);

    auto it = d.begin() + 2;
    auto cit = cd.end() - 3;

    it->x = 5;
    assert(cit->x == 5);

    d.erase(d.begin() + 1);
    d.erase(d.begin() + 3);
    assert(d.size() == 3);

    auto dd = cd;

    dd.pop_back();
    dd.insert(dd.begin(), {3, 4.0});
    dd.insert(dd.begin() + 2, {4, 5.0});

    std::string s;
    for (const auto& x: dd) {
        s += std::to_string(x.x);
    }
    assert(s == "3145");

    std::string ss;
    for (const auto& x: d) {
        ss += std::to_string(x.x);
    }
    assert(ss == "151");
}

void test5() {
    Deque<int> d;

    d.push_back(1);
    d.push_front(2);

    auto left_ptr = &*d.begin();
    auto right_ptr = &*(d.end()-1);

    d.push_back(3);
    d.push_front(4);
    auto left = *d.begin();
    auto right = *(d.end()-1);

    for (int i = 0; i < 10'000; ++i) {
        d.push_back(i);
    }
    for (int i = 0; i < 20'000; ++i) {
        d.push_front(i);
    }
    
    std::string s;
    s += std::to_string(left);
    s += std::to_string(right);

    s += std::to_string(*left_ptr);
    s += std::to_string(*right_ptr);
    //for (auto it = left; it <= right; ++it) {
    //    s += std::to_string(*it);
    //}
    assert(s == "4321");
}

struct VerySpecialType {
    int x = 0;

    explicit VerySpecialType(int x): x(x) {}
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;
    NotDefaultConstructible(const NotDefaultConstructible&) = default;
    NotDefaultConstructible& operator=(const NotDefaultConstructible&) = default;

    NotDefaultConstructible(VerySpecialType v): x(v.x) {}

public:
    int x = 0;
};

void test6() {
    Deque<NotDefaultConstructible> d;
    
    NotDefaultConstructible ndc = VerySpecialType(-1);

    for (int i = 0; i < 1500; ++i) {
        ++ndc.x;
        d.push_back(ndc);
    }

    assert(d.size() == 1500);

    for (int i = 0; i < 1300; ++i) {
        d.pop_front();
    }

    assert(d.size() == 200);

    assert(d[99].x == 1399);

    d[100] = VerySpecialType(0);
    assert(d[100].x == 0);
}


struct Explosive {
    int x = 0;
    Explosive(int x): x(x) {}
    Explosive(const Explosive&) {
        if (x) throw std::runtime_error("Boom!");
    }
};

void test7() {

    Deque<Explosive> d;
    d.push_back(Explosive(0));

    for (int i = 0; i < 30'000; ++i) {
        auto it = d.begin();
        auto x = it->x;
        size_t sz = d.size();
        try {
            if (i % 2)
                d.push_back(Explosive(1));
            else
                d.push_front(Explosive(1));
        } catch (...) {
            assert(it == d.begin());
            assert(d.begin()->x == x);
            assert(d.size() == sz);
        }

        d.push_back(Explosive(0));
    }

}

} // tests by mesyarik

namespace TestsByUnrealf1 {
    struct Fragile {
        Fragile(int durability, int data): durability(durability), data(data) {}
        ~Fragile() = default;
        
        // for std::swap
        Fragile(Fragile&& other): Fragile() {
            *this = other;
        }

        Fragile(const Fragile& other): Fragile() {
            *this = other;
        }

        Fragile& operator=(const Fragile& other) {
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
        Fragile() {

        }
    };

    struct Explosive {
        struct Safeguard {};

        inline static bool exploded = false;

        Explosive(): should_explode(true) {
            throw 1;
        }

        Explosive(Safeguard): should_explode(false) {

        }

        Explosive(const Explosive&): should_explode(true) {
            throw 2;
        }

        //TODO: is this ok..?
        Explosive& operator=(const Explosive&) {return *this;}

        ~Explosive() {
            exploded |= should_explode;
        }

    private:
        const bool should_explode;
    };

    struct DefaultConstructible {
        DefaultConstructible() {
            data = default_data;
        }

        int data = default_data;
        inline static const int default_data = 117;
    };

    struct NotDefaultConstructible {
        NotDefaultConstructible() = delete;
        NotDefaultConstructible(int input): data(input) {}
        int data;

        auto operator<=>(const NotDefaultConstructible&) const = default;
    };

    struct CountedException : public std::exception {

    };

    template<int when_throw>
    struct Counted {
        inline static int counter = 0;

        Counted() {
            ++counter;
            if (counter == when_throw) {
                --counter;
                throw CountedException();
            }
        }

        Counted(const Counted&): Counted() { }

        ~Counted() {
            --counter;
        }
    };

    template<typename iter, typename T>
    struct CheckIter{
        using traits = std::iterator_traits<iter>;

        static_assert(std::is_same_v<std::remove_cv_t<typename traits::value_type>, std::remove_cv_t<T>>);
        static_assert(std::is_same_v<typename traits::pointer, T*>);
        static_assert(std::is_same_v<typename traits::reference, T&>);
        static_assert(std::is_same_v<typename traits::iterator_category, std::random_access_iterator_tag>);

        static_assert(std::is_same_v<decltype(std::declval<iter>()++), iter>);
        static_assert(std::is_same_v<decltype(++std::declval<iter>()), iter&>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() + 5), iter>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() += 5), iter&>);

        static_assert(std::is_same_v<decltype(std::declval<iter>() - std::declval<iter>()), typename traits::difference_type>);
        static_assert(std::is_same_v<decltype(*std::declval<iter>()), T&>);
        
        static_assert(std::is_same_v<decltype(std::declval<iter>() < std::declval<iter>()), bool>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() <= std::declval<iter>()), bool>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() > std::declval<iter>()), bool>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() >= std::declval<iter>()), bool>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() == std::declval<iter>()), bool>);
        static_assert(std::is_same_v<decltype(std::declval<iter>() != std::declval<iter>()), bool>);
    };

    void testDefault() {
        Deque<int> defaulted;
        assert((defaulted.size() == 0));
        Deque<NotDefaultConstructible> without_default;
        assert((without_default.size() == 0));
    }

    void testCopy() {
        Deque<NotDefaultConstructible> without_default;
        Deque<NotDefaultConstructible> copy = without_default;
        assert((copy.size() == 0));
    }

    void testWithSize() {
        int size = 17;
        int value = 14;
        Deque<int> simple(size);
        assert((simple.size() == size_t(size)) && std::all_of(simple.begin(), simple.end(), [](int item){ return item == 0; }));
        Deque<NotDefaultConstructible> less_simple(size, value);
        assert((less_simple.size() == size_t(size)) && std::all_of(less_simple.begin(), less_simple.end(), [&](const auto& item){ 
                    return item.data == value; 
        }));
        Deque<DefaultConstructible> default_constructor(size);
        assert(std::all_of(default_constructor.begin(), default_constructor.end(), [](const auto& item) { 
                    return item.data == DefaultConstructible::default_data;
        }));
    }

    void testAssignment(){ 
        Deque<int> first(10, 10);
        Deque<int> second(9, 9);
        first = second;
        assert((first.size() == second.size()) && (first.size() == 9) && std::equal(first.begin(), first.end(), second.begin()));
    }

    void testStaticAsserts() {
        using T1 = int;
        using T2 = NotDefaultConstructible;

        static_assert(std::is_default_constructible_v<Deque<T1>>, "should have default constructor");
        static_assert(std::is_default_constructible_v<Deque<T2>>, "should have default constructor");
        static_assert(std::is_copy_constructible_v<Deque<T1> >, "should have copy constructor");
        static_assert(std::is_copy_constructible_v<Deque<T2> >, "should have copy constructor");
        //static_assert(std::is_constructible_v<Deque<T1>, int>, "should have constructor from int");
        //static_assert(std::is_constructible_v<Deque<T2>, int>, "should have constructor from int");
        static_assert(std::is_constructible_v<Deque<T1>, int, const T1&>, "should have constructor from int and const T&");
        static_assert(std::is_constructible_v<Deque<T2>, int, const T2&>, "should have constructor from int and const T&");

        static_assert(std::is_copy_assignable_v<Deque<T1>>, "should have assignment operator");
        static_assert(std::is_copy_assignable_v<Deque<T2>>, "should have assignment operator");
    }
        
    void testOperatorSubscript() {
        Deque<size_t> defaulted(1300, 43);
        assert((defaulted[0] == defaulted[1280]) && (defaulted[0] == 43));
        assert((defaulted.at(0) == defaulted[1280]) && (defaulted.at(0) == 43));
        int caught = 0;

        try {
            defaulted.at(size_t(-1));
        } catch (std::out_of_range& e) {
            ++caught;
        }

        try {
            defaulted.at(1300);
        } catch (std::out_of_range& e) {
            ++caught;
        }

        assert(caught == 2);
    }

    void testStaticAssertsAccess() {
        Deque<size_t> defaulted;
        const Deque<size_t> constant;
        static_assert(std::is_same_v<decltype(defaulted[0]), size_t&>);
        static_assert(std::is_same_v<decltype(defaulted.at(0)), size_t&>);
        static_assert(std::is_same_v<decltype(constant[0]), const size_t&>);
        static_assert(std::is_same_v<decltype(constant.at(0)), const size_t&>);

        //static_assert(noexcept(defaulted[0]), "operator[] should not throw");
        static_assert(!noexcept(defaulted.at(0)), "at() can throw");

    }


    void testStaticAssertsIterators() {
        CheckIter<Deque<int>::iterator, int> iter;
        std::ignore = iter;
        CheckIter<decltype(std::declval<Deque<int>>().rbegin()), int> reverse_iter;
        std::ignore = reverse_iter;
        CheckIter<decltype(std::declval<Deque<int>>().cbegin()), const int> const_iter;
        std::ignore = const_iter;

        static_assert(std::is_convertible_v<
                decltype(std::declval<Deque<int>>().begin()), 
                decltype(std::declval<Deque<int>>().cbegin()) 
                >, "should be able to construct const iterator from non const iterator");
        static_assert(!std::is_convertible_v<
                decltype(std::declval<Deque<int>>().cbegin()), 
                decltype(std::declval<Deque<int>>().begin()) 
                >, "should NOT be able to construct iterator from const iterator");
    }

    void testIteratorsArithmetic() {
        Deque<int> empty;
        assert((empty.end() - empty.begin()) == 0);
        assert((empty.begin() + 0 == empty.end()) && (empty.end() - 0 == empty.begin()));

        Deque<int> one(1);
        auto iter2 = one.end();
        assert(((--iter2) == one.begin()));

        assert((empty.rend() - empty.rbegin()) == 0);
        assert((empty.rbegin() + 0 == empty.rend()) && (empty.rend() - 0 == empty.rbegin()));
        auto r_iter = empty.rbegin();
        assert((r_iter++ == empty.rbegin()));

        assert((empty.cend() - empty.cbegin()) == 0);
        assert((empty.cbegin() + 0 == empty.cend()) && (empty.cend() - 0 == empty.cbegin()));
        auto c_iter = empty.cbegin();
        assert((c_iter++ == empty.cbegin()));

        Deque<int> d(1000, 3);
        assert(size_t((d.end() - d.begin())) == d.size());
        assert((d.begin() + d.size() == d.end()) && (d.end() - d.size() == d.begin()));
    }

    void testIteratorsComparison() {
        Deque<int> d(1000, 3);

        assert(d.end() > d.begin());
        assert(d.cend() > d.cbegin());
        assert(d.rend() > d.rbegin());
    }

    void testPlusMinusOne() {
        Deque<int> d(40);
        Deque<int>::iterator begin = d.begin();
        Deque<int>::iterator end = d.end();
        for (int i = 0; i < 100; ++i) {
            Deque<int>::iterator test = begin + i;
            Deque<int>::iterator test2 = end - (d.size() - i);
            assert(test == test2);
        }
        Deque<int> dd(1000, 3);
        for (int i = 2; i < 990; i++) {
            std::sort(dd.rbegin(), dd.rbegin() + i);
        }
    }

    void testIteratorsAlgorithms() {
        Deque<int> d(1000, 3);

        std::iota(d.begin(), d.end(), 13);
        std::mt19937 g(31415);
        std::shuffle(d.begin(), d.end(), g);
        std::sort(d.rbegin(), d.rbegin() + 500);
        std::reverse(d.begin(), d.end());
        auto sorted_border = std::is_sorted_until(d.begin(), d.end());
        //std::copy(d.begin(), d.end(), std::ostream_iterator<int>(std::cout, " "));
        //std::cout << std::endl;
        assert(sorted_border - d.begin() == 500);
    }

    void testPushAndPop() {
        Deque<NotDefaultConstructible> d(10000, { 1 });
        auto start_size = d.size();
        
        auto middle = &(*(d.begin() + start_size / 2)); // 5000
        auto& middle_element = *middle;
        auto begin = &(*d.begin());
        auto end = &(*d.rbegin());

        auto middle2 = &(*((d.begin() + start_size / 2) + 2000)); // 7000
        
        // remove 400 elements 
        for (size_t i = 0; i < 400; ++i) {
            d.pop_back();
        }
        
        // begin and middle pointers are still valid
        assert(begin->data == 1);
        assert(middle->data == 1);
        assert(middle_element.data == 1);
        assert(middle2->data == 1);

        end = &*d.rbegin();
        
        // 800 elemets removed in total
        for (size_t i = 0; i < 400; ++i) {
            d.pop_front();
        }

        // and and middle iterators are still valid
        assert(end->data == 1);
        assert(middle->data == 1);
        assert(middle_element.data == 1);
        assert(middle2->data == 1);
        
        // removed 9980 items in total
        for (size_t i = 0; i < 4590; ++i) {
            d.pop_front();
            d.pop_back();
        }

        assert(d.size() == 20);
        assert(middle_element.data == 1);
        assert(middle->data == 1 && middle->data == 1);
        assert(std::all_of(d.begin(), d.end(), [](const auto& item) { return item.data == 1; } ));

        begin = &*d.begin();
        end = &*d.rbegin();

        for (size_t i = 0; i < 5500; ++i) {
            d.push_back({2});
            d.push_front({2});
        }

        assert((*begin).data == 1);
        assert((*end).data == 1);
        assert(d.begin()->data == 2);
        assert(d.size() == 5500 * 2 + 20);
        assert(std::count(d.begin(), d.end(), NotDefaultConstructible{1}) == 20);
        assert(std::count(d.begin(), d.end(), NotDefaultConstructible{2}) == 11000);
    }

    void testInsertAndErase() {
        Deque<NotDefaultConstructible> d(10000, { 1 });
        auto start_size = d.size();
        
        d.insert(d.begin() + start_size / 2, NotDefaultConstructible{2});
        assert(d.size() == start_size + 1);
        d.erase(d.begin() + start_size / 2 - 1);
        assert(d.size() == start_size);

        assert(size_t(std::count(d.begin(), d.end(), NotDefaultConstructible{1})) == start_size - 1);
        assert(std::count(d.begin(), d.end(), NotDefaultConstructible{2}) == 1);

        Deque<NotDefaultConstructible> copy;
        for (const auto& item : d) {
            copy.insert(copy.end(), item);
        }
        // std::copy(d.cbegin(), d.cend(), std::inserter(copy, copy.begin()));
        
        assert(d.size() == copy.size());
        assert(std::equal(d.begin(), d.end(), copy.begin()));
    }

    void testExceptions() {
        try {
            Deque<Counted<17>> d(100);
        } catch (CountedException& e) {
            assert(Counted<17>::counter == 0);
        } catch (...) {
            // should have caught same exception as thrown by Counted
            assert(false);
        }

        try {
            Deque<Explosive> d(100);
        } catch (...) {
            
        }

        try {
            Deque<Explosive> d;
        } catch (...) {
            // no objects should have been created
            assert(false);
        }
        assert(Explosive::exploded == false);

        try {
            Deque<Explosive> d;
            auto safe = Explosive(Explosive::Safeguard{});
            d.push_back(safe);
        } catch (...) {

        }

        // Destructor should not be called for an object
        // with no finihshed constructor
        // the only destructor called - safe explosive with the safeguard
        assert(Explosive::exploded == false);
    }

    void testStrongGuarantee() {
        const size_t size = 20'000;
        const size_t initial_data = 100;
        Deque<Fragile> d(size, Fragile(size, initial_data));

        auto is_intact = [&] {
            return d.size() == size && std::all_of(d.begin(), d.end(), [initial_data](const auto& item) {return item.data == initial_data;} );
        };
        try {
            d.insert(d.begin() + size / 2, Fragile(0, initial_data + 1));
        } catch (...) {
            // have to throw
            assert(is_intact());
        }
        try {
            // for those who like additional copies...
            d.insert(d.begin() + size / 2, Fragile(3, initial_data + 2));
        } catch (...) {
            // might throw depending on the implementation
            // if it DID throw, then deque should be untouched
            assert(is_intact());
        }
    }
} // namespace TestsByUnrealf1

int main() {
    
    //static_assert(!std::is_same_v<std::deque<VerySpecialType>,
    //        Deque<VerySpecialType>>, "You cannot use std::deque, cheater!");
    //static_assert(!std::is_base_of_v<std::deque<VerySpecialType>,
    //        Deque<VerySpecialType>>, "You cannot use std::deque, cheater!");
     
    TestsByMesyarik::test1();
    TestsByMesyarik::test2();
    TestsByMesyarik::test3();
    TestsByMesyarik::test4();
    TestsByMesyarik::test5();
    TestsByMesyarik::test6();
    TestsByMesyarik::test7();

    TestsByUnrealf1::testDefault();
    TestsByUnrealf1::testCopy();
    TestsByUnrealf1::testWithSize();
    TestsByUnrealf1::testAssignment();
    TestsByUnrealf1::testStaticAsserts();
    TestsByUnrealf1::testOperatorSubscript();
    TestsByUnrealf1::testStaticAssertsAccess();
    TestsByUnrealf1::testStaticAssertsIterators();
    TestsByUnrealf1::testIteratorsArithmetic();
    TestsByUnrealf1::testIteratorsComparison();
    TestsByUnrealf1::testPlusMinusOne();
    TestsByUnrealf1::testIteratorsAlgorithms();
    TestsByUnrealf1::testPushAndPop();
    TestsByUnrealf1::testInsertAndErase();
    TestsByUnrealf1::testExceptions();
    TestsByUnrealf1::testStrongGuarantee();

    std::cout << 0;
}
