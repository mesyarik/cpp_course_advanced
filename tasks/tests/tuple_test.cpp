#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <functional>
#include <algorithm>
#include <type_traits>

#include "tuple.h"

//#include <tuple>

/*template <typename... Types>
using Tuple = std::tuple<Types...>;

template <typename... Types>
constexpr auto makeTuple(Types&&... args) {
    return std::make_tuple(std::forward<Types>(args)...);
}

template <typename... Tuples>
constexpr auto tupleCat(Tuples&&... args) {
    return std::tuple_cat(std::forward<Tuples>(args)...);
}*/

int new_called = 0;
int delete_called = 0;

void* operator new(size_t n) {
    ++new_called;
    return std::malloc(n);
}

void operator delete(void* ptr) noexcept {
    ++delete_called;
    std::free(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    ++delete_called;
    std::free(ptr);
}

struct NeitherDefaultNorCopyConstructible {
    double d;
    NeitherDefaultNorCopyConstructible() = delete;
    NeitherDefaultNorCopyConstructible(double d): d(d) {}
    NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
    NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&&) = delete;
    NeitherDefaultNorCopyConstructible& operator =(NeitherDefaultNorCopyConstructible&&) = delete;
    NeitherDefaultNorCopyConstructible& operator =(const NeitherDefaultNorCopyConstructible&) = delete;
};

struct NotDefaultConstructible {
    int c;
    NotDefaultConstructible() = delete;
    NotDefaultConstructible(int c): c(c) {}
};

struct NotCopyableOnlyMovable {
    NotCopyableOnlyMovable(const NotCopyableOnlyMovable&) = delete;
    NotCopyableOnlyMovable& operator=(const NotCopyableOnlyMovable&) = delete;
    NotCopyableOnlyMovable(NotCopyableOnlyMovable&&) = default;
    NotCopyableOnlyMovable& operator=(NotCopyableOnlyMovable&&) = default;
};

struct Accountant {
    inline static int constructed = 0;
    inline static int destructed = 0;
    inline static int copy_constructed = 0;
    inline static int move_constructed = 0;
    inline static int copy_assigned = 0;
    inline static int move_assigned = 0;

    Accountant() {
        ++constructed;
    }
    Accountant(const Accountant&) {
        ++constructed;
        ++copy_constructed;
    }
    Accountant(Accountant&&) {
        ++constructed;
        ++move_constructed;
    }
    Accountant& operator=(const Accountant&) {
        ++copy_assigned;
        return *this;
    }
    Accountant& operator=(Accountant&&) {
        ++move_assigned;
        return *this;
    }
    ~Accountant() {
        ++destructed;
    }
};

template <typename T>
class Debug {
    Debug() = delete;
};



template <typename T>
void explicit_test_helper(Tuple<int, T> t = {11, 22}) {}

template <typename T>
constexpr bool explicit_test() {
    return requires(T) {
        explicit_test_helper<T>();
    };
}

static_assert(explicit_test<int>());
static_assert(!explicit_test<std::vector<int>>());

struct ExplicitDefault {
    explicit ExplicitDefault() {}
};

template <typename T>
void explicit_default_test_helper(Tuple<int, T> t = {{}, {}}) {}

template <typename T>
constexpr bool explicit_default_test() {
    return requires(T) {
        explicit_default_test_helper<T>();
    };
}

static_assert(explicit_default_test<int>());
static_assert(!explicit_default_test<ExplicitDefault>());


template <typename T>
void explicit_conversion_test_helper(Tuple<int, T> t = Tuple<double, int>(3.14, 0)) {}

template <typename T>
constexpr bool explicit_conversion_test() {
    return requires(T) {
        explicit_conversion_test_helper<T>();
    };
}

static_assert(explicit_conversion_test<int>());
static_assert(!explicit_conversion_test<std::vector<int>>());


// Unfortunately doesn't work with std::get
/*
template <typename T>
constexpr int get_another_test(T& (*ptr)(Tuple<int, double, char>&) 
        = static_cast<T& (*)(Tuple<int, double, char>&)> ( &std::get<T, int, double, char>)) {
    return 0;
}

template <typename T, int = get_another_test<std::string>()>
constexpr bool test(int) {
    return false;
}

template <typename T>
constexpr bool test(...) {
    return true;
}

static_assert(test<std::string>(0));
*/


void test_tuple() {

    static_assert(std::is_default_constructible_v<Tuple<int, int>>);
    static_assert(!std::is_default_constructible_v<Tuple<NotDefaultConstructible, std::string>>);
    static_assert(!std::is_default_constructible_v<Tuple<int, NotDefaultConstructible>>);
    
    {
        Tuple<int, int> t1;
        assert(get<0>(t1) == 0);
        assert(get<1>(t1) == 0);
    }

    {
        Tuple<Accountant, Accountant> t2;
        assert(Accountant::constructed == 2);
    }
    assert(Accountant::destructed == 2);
    Accountant::constructed = Accountant::destructed = 0;


    static_assert(std::is_constructible_v<Tuple<int, double, char>, int, double, char>);
    static_assert(!std::is_constructible_v<Tuple<NotCopyableOnlyMovable, int>, const NotCopyableOnlyMovable&, int>);
    static_assert(std::is_constructible_v<Tuple<NotCopyableOnlyMovable, int>, NotCopyableOnlyMovable&&, int>);

    static_assert(std::is_constructible_v<Tuple<std::string, int>, const char*, int>);

    static_assert(std::is_constructible_v<Tuple<int&, char&>, int&, char&>);
    static_assert(std::is_constructible_v<Tuple<int&, const char&>, int&, char&>);
    static_assert(!std::is_constructible_v<Tuple<int&, char&>, int&, const char&>);
    static_assert(std::is_constructible_v<Tuple<int&, const char&>, int&, const char&>);
    static_assert(std::is_constructible_v<Tuple<int&, const char&>, int&, char>);

    static_assert(std::is_constructible_v<Tuple<int&&, char&>, int, char&>);
    static_assert(std::is_constructible_v<Tuple<int&&, char&>, int&&, char&>);
    static_assert(!std::is_constructible_v<Tuple<int&, char&&>, int&, char&>);
    static_assert(std::is_constructible_v<Tuple<int&, const char&>, int&, char&&>);

    {
        Accountant a;
        Tuple<Accountant, int> t(a, 5);
        assert(Accountant::copy_constructed == 1);
        
        Tuple t2 = t;
        assert(Accountant::copy_constructed == 2);

        get<1>(t) = 7;
        assert(get<1>(t) == 7);
        assert(get<1>(t2) == 5);

        t = t2;
        assert(Accountant::copy_assigned == 1);
        assert(Accountant::copy_constructed == 2);
        assert(get<1>(t) == 5);
        assert(get<1>(t2) == 5);

        t2 = std::move(t);
        assert(Accountant::move_assigned == 1);
        assert(Accountant::copy_assigned == 1);
        assert(Accountant::move_constructed == 0);
        assert(Accountant::copy_constructed == 2);
    
        const auto& t3 = t;
        t2 = std::move(t3);
        assert(Accountant::move_assigned == 1);
        assert(Accountant::copy_assigned == 2);
    }
    assert(Accountant::destructed == 3);
    Accountant::copy_constructed = Accountant::destructed = 0;
    Accountant::copy_assigned = Accountant::move_assigned = 0;

    {
        int x = 5;
        double d = 3.14;
        Tuple<int&, double&> t{x, d};

        int y = 6;
        double e = 2.72;
        Tuple<int&, double&> t2{y, e};

        t = t2; // x changed to 6
        assert(get<0>(t) == 6);
        assert(x == 6);

        y = 7;
        assert(get<0>(t) == 6);
        assert(get<0>(t2) == 7);

        get<0>(t) = 8; // x changed to 8
        assert(x == 8);
        assert(y == 7);
        assert(get<0>(t2) == 7);
    }


    {
        Accountant a;
        int x = 5;
        Tuple<Accountant&, const int&> t(a, x);
        assert(Accountant::copy_constructed == 0);
        assert(get<1>(t) == 5);

        x = 7;
        assert(get<1>(t) == 7);

        Tuple<Accountant, const int> t2 = t;
        assert(Accountant::copy_constructed == 1);

        static_assert(!std::is_assignable_v<decltype(t2), decltype(t2)>);
        static_assert(!std::is_assignable_v<decltype(t2), decltype(t)>);
        static_assert(!std::is_assignable_v<decltype(get<1>(t)), int>);
        assert(get<1>(t2) == 7);

        Tuple<Accountant, int> t3 = std::move(t);
        assert(Accountant::move_constructed == 0);
        assert(Accountant::copy_constructed == 2);
        assert(get<1>(t3) == 7);

        Tuple<Accountant&, const int> t4{a, x};
        Tuple<Accountant, int> t5 = std::move(t4);
        assert(Accountant::move_constructed == 0);
        assert(Accountant::copy_constructed == 3);

        Tuple<Accountant, int> t6 = std::move(t2);
        assert(Accountant::move_constructed == 1);
        assert(Accountant::copy_constructed == 3);
       
        Tuple<Accountant, const int&> t7{a, x};
        Tuple<Accountant, int> t8 = std::move(t7);
        assert(Accountant::move_constructed == 2);
        assert(Accountant::copy_constructed == 4);

        static_assert(!std::is_copy_assignable_v<decltype(t7)>);
        static_assert(!std::is_move_assignable_v<decltype(t7)>);

        t3 = std::move(t2);
        assert(Accountant::move_assigned == 1);
        assert(Accountant::copy_assigned == 0);

        get<1>(t3) = 8;

        t3 = std::move(t4);
        assert(Accountant::move_assigned == 1);
        assert(Accountant::copy_assigned == 1);
        assert(get<1>(t4) == 7);
    }
    assert(Accountant::destructed == 7);
    Accountant::copy_constructed = Accountant::move_constructed = Accountant::destructed = 0;
    Accountant::copy_assigned = Accountant::move_assigned = 0;

    static_assert(!std::is_default_constructible_v<Tuple<Accountant, int&>>);
    static_assert(std::is_copy_constructible_v<Tuple<Accountant, int&>>);
    static_assert(!std::is_copy_constructible_v<Tuple<Accountant, int&&>>);
    
    {
        Accountant a;
        int x = 5;
        Tuple<Accountant, int&&> t(std::move(a), std::move(x));
        assert(Accountant::move_constructed == 1);
        assert(Accountant::copy_constructed == 0);
        assert(get<1>(t) == 5);
       
        x = 7;
        assert(get<1>(t) == 7);
        
        get<1>(t) = 8;
        assert(x == 8);
 
        Tuple t2 = std::move(t);
        assert(Accountant::copy_constructed == 0);
        assert(Accountant::move_constructed == 2);

        assert(get<1>(t2) == 8);

        Tuple<Accountant&&, int&> t3{std::move(a), x};
        Tuple<Accountant, int> t4 = t3;
        assert(Accountant::copy_constructed == 1);
        assert(Accountant::move_constructed == 2);

        Tuple<Accountant, int> t5 = std::move(t3);
        assert(Accountant::copy_constructed == 1);
        assert(Accountant::move_constructed == 3);

        x = 15;
        t4 = t3;
        assert(Accountant::copy_assigned == 1);
        assert(Accountant::move_assigned == 0);
        assert(get<1>(t4) == 15);

        t5 = std::move(t3);
        assert(Accountant::copy_assigned == 1);
        assert(Accountant::move_assigned == 1);
        assert(get<1>(t5) == 15);

        get<int>(t4) = 22;

        t3 = t4;
        assert(Accountant::copy_assigned == 2);
        assert(Accountant::move_assigned == 1);
        assert(x == 22);        

        get<int>(t5) = 33;

        t3 = std::move(t5);
        assert(Accountant::copy_assigned == 2);
        assert(Accountant::move_assigned == 2);
        assert(x == 33);
    }
    assert(Accountant::destructed == 5);
    Accountant::copy_constructed = Accountant::move_constructed = Accountant::destructed = 0;
    Accountant::copy_assigned = Accountant::move_assigned = 0;


    {
        int x = 5;
        Tuple<const int, int&, NeitherDefaultNorCopyConstructible> t(1, x, 3.14);
        
        static_assert(!std::is_assignable_v<decltype(get<0>(t)), int>);
        static_assert(!std::is_assignable_v<decltype(get<0>(t)), NeitherDefaultNorCopyConstructible>);

        get<1>(t) = 7;
        assert(x == 7);
    }

    static_assert(!std::is_copy_constructible_v<Tuple<int&, NeitherDefaultNorCopyConstructible>>);
    static_assert(std::is_move_constructible_v<Tuple<int&, NeitherDefaultNorCopyConstructible>>);

    {
        int x = 5;
        NotDefaultConstructible ndc{1};

        Tuple<NotDefaultConstructible&, int&> tr{ndc, x};

        Tuple<NotDefaultConstructible, int> t2 = tr;

        x = 3;
        assert(get<1>(tr) == 3);
        assert(get<1>(t2) == 5);
    }

    static_assert(!std::is_constructible_v<Tuple<std::string&, int&>, Tuple<std::string, int>>);
    static_assert(!std::is_constructible_v<Tuple<std::string&&, int&&>, Tuple<std::string&, int&>>);
    static_assert(!std::is_constructible_v<Tuple<std::string&, int&&>, Tuple<std::string&, int&>>);

    static_assert(std::is_constructible_v<Tuple<const std::string&, int&>, Tuple<std::string&, int&>>);
    static_assert(!std::is_constructible_v<Tuple<std::string&, int&>, Tuple<const std::string&, int&>>);

    assert(new_called == 0);
    assert(delete_called == 0);
    
    // constructor from pair
    {
        std::pair<int, double> p{1, 3.14};
        Tuple t = p;
        get<int>(t) = 2;
        get<double>(t) = 2.72;

        int x = 1;
        double d = 3.14;
        std::pair<int&, double&> p2{x, d};
        Tuple t2 = p2;
        get<int&>(t2) = 2;
        assert(x == 2);
    
        Tuple<int, double> t3 = p2;
        get<int>(t3) = 3;
        assert(x == 2);

        Tuple<int&&, double&&> t4 = std::move(p);
        get<int&&>(t4) = 5;
        assert(p.first == 5);
    }

    // two references on the same object
    {
        int x = 1;
        Tuple<int, int&, const int&, int&&> t{x, x, x, std::move(x)};

        ++get<int>(t);
        ++get<int&>(t);
        ++get<int&&>(t);
        assert(get<const int&>(t) == 3);
        assert(get<int>(t) == 2);
        assert(get<3>(t) == 3);
        assert(x == 3);
    }

    // rvalue get
    {
        int x = 3;
        int y = 4;
        Tuple<int, long, int&, const int&, int&&> tuple(1, 2, x, x, std::move(y));

        static_assert(std::is_same_v<decltype(get<int&&>(tuple)), int&>);
        
        static_assert(std::is_same_v<decltype(get<int>(std::move(tuple))), int&&>);
        static_assert(std::is_same_v<decltype(get<int&>(std::move(tuple))), int&>);
        static_assert(std::is_same_v<decltype(get<const int&>(std::move(tuple))), const int&>);

        const auto& const_tuple = tuple;

        static_assert(std::is_same_v<decltype(get<int>(std::move(const_tuple))), const int&&>);
        static_assert(std::is_same_v<decltype(get<int&>(std::move(const_tuple))), int&>);
        static_assert(std::is_same_v<decltype(get<const int&>(std::move(const_tuple))), const int&>);
        static_assert(std::is_same_v<decltype(get<int&&>(std::move(const_tuple))), int&&>);

    }

    // tuple with one argument
    // get by type, CE if type is absent or ambiguous
    {
        Tuple<int> first(12);
        auto second = first;
        assert(get<0>(first) == get<0>(second));

        second = Tuple<int>(14);
        assert(get<0>(second) == 14);

        second = first;

        Tuple<double> third{3.14};
        second = third;

        assert(get<int>(second) == 3);

        first = 1;
        assert(get<int>(first) == 1);
    }

    // make_tuple, tie, forward_as_tuple
    {
        Tuple tuple = makeTuple(5, std::string("test"), std::vector<int>(2, 5));
        get<2>(tuple)[1] = 2;
        assert(get<2>(tuple)[1] == 2);


    }

    
    // tuple_cat
    {
        Tuple<Accountant, Accountant, int> t1;

        Tuple<Accountant, Accountant, int, Accountant> t2;

        Tuple<Accountant, Accountant, Accountant> t3;

        auto cat = tupleCat(t1, t2, std::move(t3));

        assert(Accountant::move_constructed == 3);
        assert(Accountant::copy_constructed == 5);
    }

}

//#include <tuple>

//struct MyType {};

int main() {
    //static_assert(!std::is_base_of<std::tuple<MyType>, Tuple<MyType>>::value, "Don't try to use std::tuple");
    test_tuple();
    std::cout << 0;
    return 0;
}
