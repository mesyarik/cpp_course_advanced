#include <iostream>
//#include <variant>
#include <vector>
#include <type_traits>
#include <cassert>

#include "variant.h"

//template <typename... Args>
//using Variant = std::variant<Args...>;

//using std::get;
//using std::holds_alternative;
//using std::visit;

void BasicTest() {

    Variant<int, std::string, double> v = 5;

    static_assert(std::is_assignable_v<decltype(v), float>);
    static_assert(!std::is_assignable_v<decltype(v), std::vector<int>>);

    assert(get<int>(v) == 5);

    v = "abc";

    assert(get<std::string>(v) == "abc");

    v = "cde";
    assert(get<std::string>(v) == "cde");

    v = 5.0;
    assert(get<double>(v) == 5.0);

    static_assert(!std::is_assignable_v<Variant<float, int>, double>);

    const auto& cv = v;
    static_assert(!std::is_assignable_v<decltype(get<double>(cv)), double>);

    static_assert(std::is_rvalue_reference_v<decltype(get<double>(std::move(v)))>);
    static_assert(std::is_lvalue_reference_v<decltype(get<double>(v))>);

}

void TestOverloadingSelection() {
    
    Variant<int*, char*, std::vector<char>, const int*, bool> v = true;

    assert(holds_alternative<bool>(v));

    v = std::vector<char>();

    get<std::vector<char>>(v).push_back('x');
    get<std::vector<char>>(v).push_back('y');
    get<std::vector<char>>(v).push_back('z');

    assert(holds_alternative<std::vector<char>>(v));

    assert(get<std::vector<char>>(v).size() == 3);

    char c = 'a';
    v = &c;

    assert(holds_alternative<char*>(v));

    *get<char*>(v) = 'b';

    assert(*get<char*>(v) == 'b');

    try {
        get<int*>(v);
        assert(false);
    } catch (...) {
        // ok
    }

    const int x = 1;
    v = &x;

    assert((!std::is_assignable_v<decltype(*get<const int*>(v)), int>));
    assert((std::is_assignable_v<decltype(get<const int*>(v)), int*>));
    
    try {
        get<int*>(v);
        assert(false);
    } catch (...) {
        // ok
    }

    const int y = 2;
    get<const int*>(v) = &y;
    assert(*get<const int*>(v) == 2);

    assert(!holds_alternative<int*>(v));
    assert(holds_alternative<const int*>(v));

    int z = 3;
    
    get<const int*>(v) = &z;
    assert(!holds_alternative<int*>(v));
    assert(holds_alternative<const int*>(v));
    assert(*get<const int*>(v) == 3);

    v = &z;

    assert(holds_alternative<int*>(v));
    assert(!holds_alternative<const int*>(v));
   
    assert(*get<int*>(v) == 3);
    
    assert((!std::is_assignable_v<decltype(get<int*>(v)), const int*>));

    try {
        get<const int*>(v);
        assert(false);
    } catch (...) {
        // ok
    }
}

void TestCopyMoveConstructorsAssignments() {
    
    Variant<std::string, char, std::vector<int>> v = "abcdefgh";

    auto vv = v;

    assert(get<std::string>(vv).size() == 8);
    assert(get<std::string>(v).size() == 8);

    {
        auto vvv = std::move(v);

        assert(get<std::string>(v).size() == 0);
        v.emplace<std::vector<int>>({1, 2, 3});

        assert(get<std::string>(vv).size() == 8);
    }

    v = std::move(vv);
    assert(get<std::string>(v).size() == 8);

    assert(get<std::string>(vv).size() == 0);

    vv = 'a';

    assert(holds_alternative<char>(vv));
    assert(holds_alternative<std::string>(v));

    get<0>(v).resize(3);
    get<0>(v)[0] = 'b';
    assert(get<std::string>(v) == "bbc");

    {
        Variant<int, const std::string> vvv = std::move(get<0>(v));

        std::vector<int> vec = {1, 2, 3, 4, 5};

        v = vec;
        assert(get<2>(v).size() == 5);
        assert(vec.size() == 5);

        vec[1] = 0;
        assert(get<std::vector<int>>(v)[1] == 2);

        vvv.emplace<int>(1);
        assert(holds_alternative<int>(vvv));
    }

}

void TestVariantWithConstType() {

    int& (*get_ptr)(Variant<int, double>&) = &get<int, int, double>;

    static_assert(!std::is_invocable_v<decltype(get_ptr), Variant<const int, double>>);

    const int& (*get_const_ptr)(Variant<const int, double>&) = &get<const int, const int, double>;

    static_assert(!std::is_invocable_v<decltype(get_const_ptr), Variant<int, double>>);



    const int& (Variant<const int, double>::*method_const_ptr)(const int&) = &Variant<const int, double>::emplace<const int, const int&>;

    static_assert(!std::is_invocable_v<decltype(method_const_ptr), Variant<int, double>&, const int&>);

    int& (Variant<int, double>::*method_ptr)(const int&) = &Variant<int, double>::emplace<int, const int&>;

    static_assert(!std::is_invocable_v<decltype(method_ptr), Variant<const int, double>&, const int&>);

    

    Variant<const int, /*int,*/ std::string, const std::string, double> v = 1;
    
    //static_assert(!std::is_assignable_v<decltype(v), int>);

    assert(holds_alternative<const int>(v));

    v.emplace<std::string>("abcde");

    get<1>(v).resize(1);

    assert(!holds_alternative<const std::string>(v));


    v.emplace<0>(5);

    assert(get<0>(v) == 5);

}

struct VerySpecialType {};

template<typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

void TestVisit() {

    std::vector<Variant<char, long, float, int, double, long long>>
               vecVariant = {5, '2', 5.4, 100ll, 2011l, 3.5f, 2017};

    auto visitor = Overload {
        [](char) { return 1; },
        [](int) { return 2; },
        [](unsigned int) { return 3; },
        [](long int) { return 4; },
        [](long long int) { return 5; },
        [](auto) { return 6; },
    };

    std::string result;
    for (auto v : vecVariant) {
        result += std::to_string(visit(visitor, v));
    }

    assert(result == "2165462");

    std::vector<Variant<std::vector<int>, double, std::string>>
        vecVariant2 = { 1.5, std::vector<int>{1, 2, 3, 4, 5}, "Hello "};


    result.clear();
    auto DisplayMe = Overload {
        [&result](std::vector<int>& myVec) { 
                for (auto v: myVec) result += std::to_string(v);
            },
        [&result](auto& arg) { 
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) {
                result += "string";
            } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, double>) {
                result += "double";
            }
        },
    };

    for (auto v : vecVariant2) {
        visit(DisplayMe, v);
    }
    assert(result == "double12345string");

}

struct OneShot {
    OneShot() = default;
    OneShot(const OneShot&) = delete;
    OneShot(OneShot&&) = default;
    OneShot& operator=(const OneShot&) = delete;
    OneShot& operator=(OneShot&&) = default;

    int operator()(auto x) && {
        if constexpr (std::is_same_v<decltype(x), int>) {
            return x*x;
        } else {
            return 42;
        }
    }
};

void TestVisitOrGetRvalue() {
    
    std::vector<Variant<int, std::string>> vec = {"abc", 7};
    
    std::string result;
    result += std::to_string(visit(OneShot(), vec[0]));
    result += std::to_string(visit(OneShot(), vec[1]));
    
    assert(result == "4249");

    auto& varString = vec[0];
    auto newString = get<std::string>(std::move(varString));

    assert(get<std::string>(varString).size() == 0);
    assert(newString.size() == 3);

    newString.clear();
    varString = "cde";
    
    newString = get<1>(std::move(varString));

    assert(get<1>(varString).size() == 0);
    assert(newString.size() == 3);
}

void TestMultipleVisit() {
    // example taken from here https://www.cppstories.com/2018/09/visit-variants/
    struct LightItem { };
    struct HeavyItem { };
    struct FragileItem { };

    Variant<LightItem, HeavyItem> basicPackA;
    Variant<LightItem, HeavyItem, FragileItem> basicPackB;
    
    {
        auto visitor = Overload{
            [](LightItem&, LightItem&) { return "2 light items"; },
            [](LightItem&, HeavyItem&) { return "light & heavy items"; },
            [](HeavyItem&, LightItem&) { return "heavy & light items"; },
            [](HeavyItem&, HeavyItem&) { return "2 heavy items"; },
            [](auto, auto) { return "another"; }
        };

        std::string result = visit(visitor, basicPackA, basicPackB);
        assert(result == "2 light items");

        basicPackA = HeavyItem();

        result = visit(visitor, basicPackA, basicPackB);
        
        assert(result == "heavy & light items");

        basicPackB = FragileItem();
        result = visit(visitor, basicPackA, basicPackB);
        
        assert(result == "another");
    }

    {
        auto visitor = Overload{
            [](HeavyItem&, FragileItem&) { return "both lvalue"; },
            [](HeavyItem&&, FragileItem&) { return "rvalue and lvalue"; },
            [](const HeavyItem&, const FragileItem&) { return "both const"; },
            [](auto&&, auto&&) { return "another"; }
        };

        std::string result = visit(visitor, basicPackA, basicPackB);
        
        assert(result == "both lvalue");

        result = visit(visitor, std::move(basicPackA), basicPackB);
        assert(result == "rvalue and lvalue");

        result = visit(visitor, std::move(basicPackA), std::move(basicPackB));
        assert(result == "another");

        const auto& constPackA = basicPackA;
        const auto& constPackB = basicPackB;

        result = visit(visitor, constPackA, constPackB);
        assert(result == "both const");

        result = visit(visitor, basicPackA, constPackB);
        assert(result == "another");
    }    
}


int main() {

    std::cerr << "Tests started." << std::endl;

    BasicTest();
    std::cerr << "Test 1 (basic) passed." << std::endl;

    TestOverloadingSelection();
    std::cerr << "Test 2 (overloadings) passed." << std::endl;

    TestCopyMoveConstructorsAssignments();
    std::cerr << "Test 3 (copy-move ctors-assignments) passed." << std::endl;

    TestVariantWithConstType();
    std::cerr << "Test 4 (const types) passed." << std::endl;

    TestVisit(); 
    std::cerr << "Test 5 (visit) passed." << std::endl;
    
    TestVisitOrGetRvalue();
    std::cerr << "Test 6 (visit or get rvalue) passed." << std::endl;

    TestMultipleVisit();
    std::cerr << "Test 7 (multiple visit) passed." << std::endl;

    assert((!std::is_base_of_v<std::variant<VerySpecialType, int>, Variant<VerySpecialType, int>>)); 

    std::cout << 0;
}
