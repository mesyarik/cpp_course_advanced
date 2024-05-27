#include <functional>
#include <numeric>
#include <vector>
#include <cassert>
#include <iostream>
#include <memory>

#define virtual VIRTUAL_FUNCTIONS_ARE_NOT_ALLOWED
#define override VIRTUAL_FUNCTIONS_ARE_NOT_ALLOWED

#include "function.hpp"

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

struct Helper {
  int method_1(int z) { return x + z; }
  int method_2(int z) { return y + z; }

  int x = 3;
  int y = 30;
};

int sum(int x, int y) {
  return x + y;
}

int multiply(int x, int y) {
  return x * y;
}

template <typename R, typename T>
R template_test_function(T t) {
  return static_cast<R>(t);
}

using namespace std::placeholders;

template <template <typename> typename Func, bool IsMoveOnly>
void test_function() {
  {
    Func<int(int, int)> func = sum;
    assert(func(5, 10) == 15);
  }

  {
    Func<int(double)> func = template_test_function<double, int>;
    assert(func(123.456) == 123);
  }

  auto lambda = [](int a, int b, int c, int d){
    return a * b + c * d;
  };

  {
    Func<int(int, int, int, int)> func = lambda;
    assert(func(2, 3, 20, 30) == 606);
  }

  auto func_ptr = +[]() {
    return 10;
  };

  {
    Func<int()> func = func_ptr;
    assert(func() == 10);
  }

  {
    Helper helper;

    Func<int(Helper&, int)> func = &Helper::method_1;
    assert(func(helper, 10) == 13);

    Func<int&(Helper&)> attr = &Helper::x;
    assert(attr(helper) == 3);

    attr(helper) = 10;
    assert(func(helper, 10) == 20);
  }

  {
    Func<int(int, int)> func = sum;
    assert(func(5, 10) == 15);
    func = multiply;
    assert(func(5, 10) == 50);
  }

  {
    Func<int(int, int)> func_1 = sum;
    Func<int(int, int)> func_2 = multiply;
    assert(func_1(5, 10) == 15);

    if constexpr (!IsMoveOnly) {
        func_1 = func_2;
        assert(func_1(5, 10) == 50);
        assert(func_2(5, 10) == 50);
    }

    func_1 = std::move(func_2);
    assert(func_1(5, 10) == 50);
    assert(!func_2);

    assert(std::move(func_1)(5, 10) == 50);
    assert(func_1(5, 10) == 50);
  }

  {
    Helper helper;
    Func<int(Helper&, int)> func = &Helper::method_1;
    Func<int&(Helper&)> attr = &Helper::x;

    assert(func(helper, 10) == 13);
    attr(helper) = 10;
    assert(func(helper, 10) == 20);

    func = &Helper::method_2;
    assert(func(helper, 10) == 40);

    attr = &Helper::y;
    assert(attr(helper) == 30);

    attr(helper) = 100;
    assert(func(helper, 10) == 110);
    
    int x = 25;
    auto lambda = [&x](Helper&) -> int& {
        return x;
    };
    attr = lambda;

    attr(helper) = 55;
    assert(x == 55);
  }

  {
    Func<int(int, int)> func;
    assert(func == nullptr);
    assert(!func);

    func = multiply;
    assert(func != nullptr);
    assert(func);

    auto f = std::move(func);
    assert(func == nullptr);
    assert(!func);
  }
 
  {
    Func<int(int, int)> func = sum;
    assert(std::invoke(func, 5, 10) == 15);
  }

  if constexpr (!IsMoveOnly) {
    Func func = sum;
    assert(func(5, 10) == 15);
  }

  if constexpr (!IsMoveOnly) {
    Func func = [](int x) { return x * x * x; };
    assert(func(10) == 1000);
  }

  {
    Func<int(int)> func = [&](int x) {
        return x == 0 ? 1 : x * func(x - 1);
    };
    assert(func(5) == 120);
  }

  // test that function does not allocate for small objects
  assert(new_called == 0);
  assert(delete_called == 0);

  {
    std::vector<int> vec = {1, 2, 3};
    Func<int()> func = [&]() {
      return std::accumulate(vec.begin(), vec.end(), 0);
    };

    assert(func() == 6);
    vec.push_back(4);
    assert(func() == 10);

    Func<int()> func2 = std::move(func);
    assert(func2() == 10);
  }

  if constexpr (!IsMoveOnly) {
    Func<void()> func;
    bool thrown = false;
    try {
        func();
    } catch (...) {
        thrown = true;
    }
    assert(thrown);
  }

  {
    Func<int(int, int)> func = std::bind(lambda, 2, _2, _1, 30);
    assert(func(20, 3) == 606);
  }

  struct TestStructConst {
      void const_method(int) const {}
      void nonconst_method(int) {}
  };

  static_assert(std::is_constructible_v<Func<void(TestStructConst&, int)>, 
              decltype(&TestStructConst::const_method)>);
  static_assert(!std::is_constructible_v<Func<void(const TestStructConst&, int)>, 
              decltype(&TestStructConst::nonconst_method)>);

  static_assert(std::is_assignable_v<Func<void(TestStructConst&, int)>, 
              decltype(&TestStructConst::const_method)>);
  static_assert(!std::is_assignable_v<Func<void(const TestStructConst&, int)>, 
              decltype(&TestStructConst::nonconst_method)>);

  static_assert(std::is_assignable_v<Func<void(TestStructConst&, int)>, 
              Func<void(const TestStructConst&, int)>>);
  static_assert(!std::is_assignable_v<Func<void(const TestStructConst&, int)>, 
              Func<void(TestStructConst&, int)>>);

  struct TestStructRvalue {
      void usual_method(int) {}
      void rvalue_method(int) && {}
      void lvalue_method(int) & {}
      void const_lvalue_method(int) const & {}
  };

  static_assert(std::is_constructible_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::usual_method)>);
  static_assert(std::is_constructible_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::rvalue_method)>);
  static_assert(!std::is_constructible_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::lvalue_method)>);
  static_assert(std::is_constructible_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::const_lvalue_method)>);

  static_assert(!std::is_invocable_v<Func<void(TestStructRvalue&&, int)>, 
              TestStructRvalue&, int>);
  static_assert(std::is_invocable_v<Func<void(TestStructRvalue&&, int)>, 
              TestStructRvalue&&, int>);
  static_assert(!std::is_invocable_v<Func<void(TestStructRvalue&, int)>, 
              TestStructRvalue&&, int>);

  static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::usual_method)>);
  static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::rvalue_method)>);
  static_assert(!std::is_assignable_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::lvalue_method)>);
  static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>, 
              decltype(&TestStructRvalue::const_lvalue_method)>);

  static_assert(!std::is_assignable_v<Func<void(TestStructRvalue&&, int)>, 
              Func<void(TestStructRvalue&, int)>>);
  static_assert(!std::is_assignable_v<Func<void(TestStructRvalue&, int)>, 
              Func<void(TestStructRvalue&&, int)>>);
  static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>, 
              Func<void(const TestStructRvalue&, int)>>);
  static_assert(!std::is_assignable_v<Func<void(const TestStructRvalue&, int)>, 
              Func<void(TestStructRvalue&&, int)>>);

  struct NotCopyable {
    std::unique_ptr<int> p{new int(5)};
    int operator()(int x) {
      return x + *p;
    }
  };

  if constexpr (IsMoveOnly) {
    NotCopyable nc;
    Func<int(int)> f = std::move(nc);
    assert(f(5) == 10);

    Func<int(int)> f2 = std::move(f);
    f = NotCopyable{std::make_unique<int>(7)};
    assert(f(5) == 12);
    assert(f2(5) == 10);
  }

  // doesn't work for std::function
  //static_assert(IsMoveOnly || !std::is_constructible_v<Func<int(int)>, NotCopyable>);
  //static_assert(IsMoveOnly || !std::is_assignable_v<Func<int(int)>, NotCopyable&>);

  new_called = 0;
  delete_called = 0;
}

//template <typename T>
//using Function = std::function<T>;

//template <typename T>
//using MoveOnlyFunction = std::move_only_function<T>;


int main() {
    test_function<Function, false>();
    test_function<MoveOnlyFunction, true>();

    std::cout << 0;
}
