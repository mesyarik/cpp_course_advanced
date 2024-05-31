#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cassert>

#include "smart_pointers.h"


/*template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

template<typename T>
using EnableSharedFromThis = std::enable_shared_from_this<T>;

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    return std::allocate_shared<T>(std::forward<const Alloc>(alloc), std::forward<Args>(args)...);
}
*/

struct Base {
    virtual ~Base() {}
};

struct Derived: public Base {};



void test_shared_ptr() {
    {
      auto empty = SharedPtr<Derived>();
      SharedPtr<Base> ptr = empty;
      auto empty2 = SharedPtr<int>();
      auto ptr2 = empty2;
      std::ignore = ptr2;
    }

    {
      auto ptr = makeShared<int>(1);
      auto ptr2 = std::move(ptr);
      // test nullptr checking presence 
      assert(ptr.use_count() == 0);
      auto wptr = WeakPtr<int>(ptr);
      assert(wptr.expired());
    }

    using std::vector;

    auto first_ptr = SharedPtr<vector<int>>(new vector<int>(1'000'000));

    (*first_ptr)[0] = 1;

    vector<int>& vec = *first_ptr;
    auto second_ptr = SharedPtr<vector<int>>(new vector<int>(vec));

    (*second_ptr)[0] = 2;

    for (int i = 0; i < 1'000'000; ++i)
        first_ptr.swap(second_ptr);
    first_ptr->swap(*second_ptr);

    assert(first_ptr->front() == 2);
    assert(second_ptr->front() == 1);

    assert(first_ptr.use_count() == 1);
    assert(second_ptr.use_count() == 1);

    for (int i = 0; i < 10; ++i) {
        auto third_ptr = SharedPtr<vector<int>>(new vector<int>(vec));
        auto fourth_ptr = second_ptr;
        fourth_ptr.swap(third_ptr);
        assert(second_ptr.use_count() == 2);
    }

    assert(second_ptr.use_count() == 1);

    {
        vector<SharedPtr<vector<int>>> ptrs(10, SharedPtr<vector<int>>(first_ptr));
        for (int i = 0; i < 100'000; ++i) {
            ptrs.push_back(ptrs.back());
            ptrs.push_back(SharedPtr<vector<int>>(ptrs.back()));
        }
        assert(first_ptr.use_count() == 1 + 10 + 200'000);
    }

    first_ptr.reset(new vector<int>());
    second_ptr.reset();
    SharedPtr<vector<int>>().swap(first_ptr);

    assert(second_ptr.get() == nullptr);
    assert(second_ptr.get() == nullptr);

    for (int k = 0; k < 2; ++k) {
        vector<SharedPtr<int>> ptrs;
        for (int i = 0; i < 100'000; ++i) {
            int* p = new int(rand() % 99'999);
            ptrs.push_back(SharedPtr<int>(p));
        }
        std::sort(ptrs.begin(), ptrs.end(), [](auto&& x, auto&& y){return *x < *y;});
        for (int i = 0; i + 1 < 100'000; ++i) {
            assert(*(ptrs[i]) <= *(ptrs[i+1]));
        }
        while (!ptrs.empty()) {
            ptrs.pop_back();
        }
    }

    // test const
    {
        const SharedPtr<int> sp(new int(42));
        assert(sp.use_count() == 1);
        assert(*sp.get() == 42);
        assert(*sp == 42);
    }
}

struct Node;

struct Next {
    SharedPtr<Node> shared;
    WeakPtr<Node> weak;
    Next(const SharedPtr<Node>& shared): shared(shared) {}
    Next(const WeakPtr<Node>& weak): weak(weak) {}
};

struct Node {
    static int constructed;
    static int destructed;

    int value;
    Next next;
    Node(int value): value(value), next(SharedPtr<Node>()) {
        ++constructed;
    }
    Node(int value, const SharedPtr<Node>& next): value(value), next(next) {
        ++constructed;
    }
    Node(int value, const WeakPtr<Node>& next): value(value), next(next) {
        ++constructed;
    }
    ~Node() {
        ++destructed;
    }
};

int Node::constructed = 0;
int Node::destructed = 0;

SharedPtr<Node> getCyclePtr(int cycleSize) {
    SharedPtr<Node> head(new Node(0));
    SharedPtr<Node> prev(head);
    for (int i = 1; i < cycleSize; ++i) {
        SharedPtr<Node> current(new Node(i));
        prev->next.shared = current;
        prev = current;
        // std::cout << prev.use_count() << '\n';
    }
    //prev->next.shared.~SharedPtr<Node>();
    //new (&prev->next.weak) WeakPtr<Node>(head);
    prev->next.weak = head;
    //prev->next.isLast = true;
    return head;
}

void test_weak_ptr() {
    auto sp = SharedPtr<int>(new int(23));
    WeakPtr<int> weak = sp;
    {
        auto shared = SharedPtr<int>(new int(42));
        weak = shared;
        assert(weak.use_count() == 1);
        assert(!weak.expired());
    }
    assert(weak.use_count() == 0);
    assert(weak.expired());
    
    weak = sp;
    auto wp = weak;
    assert(weak.use_count() == 1);
    assert(wp.use_count() == 1);
    auto wwp = std::move(weak);
    //assert(weak.use_count() == 0);
    assert(wwp.use_count() == 1);

    auto ssp = wwp.lock();
    assert(sp.use_count() == 2);

    sp = ssp;
    ssp = sp;
    assert(ssp.use_count() == 2);

    sp = std::move(ssp);
    assert(sp.use_count() == 1);

    ssp.reset(); // nothing should happen
    sp.reset();

    unsigned int useless_value = 0;
    for (int i = 0; i < 100'000; ++i) {
        SharedPtr<Node> head = getCyclePtr(8);
        SharedPtr<Node> nextHead = head->next.shared;
        assert(nextHead.use_count() == 2);
        useless_value += 19'937 * i * nextHead.use_count();
        
        head.reset();
        assert(nextHead.use_count() == 1);
    }
    std::ignore = useless_value;

    assert(Node::constructed == 800'000);
    assert(Node::destructed == 800'000);

    // test inheritance
    {
        SharedPtr<Derived> dsp(new Derived());

        SharedPtr<Base> bsp = dsp;

        WeakPtr<Derived> wdsp = dsp;
        WeakPtr<Base> wbsp = dsp;
        WeakPtr<Base> wwbsp = wdsp;

        assert(dsp.use_count() == 2);
        
        bsp = std::move(dsp);
        assert(bsp.use_count() == 1);
        
        bsp.reset();
        assert(wdsp.expired());
        assert(wbsp.expired());
        assert(wwbsp.expired());
    }

    // test const
    {
        SharedPtr<int> sp(new int(42));
        const WeakPtr<int> wp(sp);
        assert(!wp.expired());
        auto ssp = wp.lock();
    }
}

struct NeitherDefaultNorCopyConstructible {
    NeitherDefaultNorCopyConstructible() = delete;
    NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
    NeitherDefaultNorCopyConstructible& operator=(const NeitherDefaultNorCopyConstructible&) = delete;

    NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&&) = default;
    NeitherDefaultNorCopyConstructible& operator=(NeitherDefaultNorCopyConstructible&&) = default;

    explicit NeitherDefaultNorCopyConstructible(int x): x(x) {}
    int x;
};

struct Accountant {
    static int constructed;
    static int destructed;

    Accountant() {
        ++constructed;
    }
    Accountant(const Accountant&) {
        ++constructed;
    }
    ~Accountant() {
        ++destructed;
    }
};

int Accountant::constructed = 0;
int Accountant::destructed = 0;

int allocated = 0;
int deallocated = 0;

int allocate_called = 0;
int deallocate_called = 0;

int new_called = 0;
int delete_called = 0;

int construct_called = 0;
int destroy_called = 0;

void* operator new(size_t n) {
    ++new_called;
    return std::malloc(n);
}

void operator delete(void* ptr) noexcept {
    ++delete_called;
    std::free(ptr);
}

struct VerySpecialType {};

void* operator new(size_t n, VerySpecialType) {
    return std::malloc(n);
}

void operator delete(void* ptr, VerySpecialType) {
    std::free(ptr);
}

// to prevent compiler warnings
void operator delete(void* ptr, size_t) noexcept {
    ++delete_called;
    std::free(ptr);
}

template<typename T>
struct MyAllocator {
    using value_type = T;
    
    MyAllocator() = default;
    
    template<typename U>
    MyAllocator(const MyAllocator<U>&) {}

    T* allocate(size_t n) {
        ++allocate_called;
        allocated += n * sizeof(T);;
        return (T*) ::operator new(n * sizeof(T), VerySpecialType());
    }

    void deallocate(T* p, size_t n) {
        ++deallocate_called;
        deallocated += n * sizeof(T);
        ::operator delete((void*)p, VerySpecialType());
    }

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        ++construct_called;
        ::new ((void*)ptr) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* ptr) {
        ++destroy_called;
        ptr->~U();
    }
};

void test_make_allocate_shared() {
    new_called = 0;
    delete_called = 0;

    {
        auto sp = makeShared<NeitherDefaultNorCopyConstructible>(
                NeitherDefaultNorCopyConstructible(0));
        WeakPtr<NeitherDefaultNorCopyConstructible> wp = sp;
        auto ssp = sp;
        sp.reset();
        assert(!wp.expired());
        ssp.reset();
        assert(wp.expired());
    }


    {
        auto sp = makeShared<Accountant>();
        assert(Accountant::constructed == 1);

        WeakPtr<Accountant> wp = sp;
        auto ssp = sp;
        sp.reset();
        assert(Accountant::constructed == 1);
        assert(Accountant::destructed == 0);
        
        assert(!wp.expired());
        ssp.reset();
        assert(Accountant::destructed == 1);

        Accountant::constructed = 0;
        Accountant::destructed = 0;
    }

    assert(new_called == 2);
    assert(delete_called == 2);

    new_called = 0;
    delete_called = 0;

    {
        MyAllocator<NeitherDefaultNorCopyConstructible> alloc;
        auto sp = allocateShared<NeitherDefaultNorCopyConstructible>(
                alloc, NeitherDefaultNorCopyConstructible(0));
        int count = allocated;
        assert(allocated > 0);
        assert(allocate_called == 1);

        WeakPtr<NeitherDefaultNorCopyConstructible> wp = sp;
        auto ssp = sp;
        sp.reset();
        assert(count == allocated);
        assert(deallocated == 0);
        
        assert(!wp.expired());
        ssp.reset();
        assert(count == allocated);
    }
    
    assert(allocated == deallocated);

    assert(allocate_called == 1);
    assert(deallocate_called == 1);
    assert(construct_called == 1);
    assert(destroy_called == 1);
     
    allocated = 0;
    deallocated = 0;
    allocate_called = 0;
    deallocate_called = 0;
    construct_called = 0;
    destroy_called = 0;
 
    {
        MyAllocator<Accountant> alloc;
        auto sp = allocateShared<Accountant>(alloc);
        int count = allocated;
        assert(allocated > 0);
        assert(allocate_called == 1);
        assert(Accountant::constructed == 1);

        WeakPtr<Accountant> wp = sp;
        auto ssp = sp;
        sp.reset();
        assert(count == allocated);
        assert(deallocated == 0);
        assert(Accountant::constructed == 1);
        assert(Accountant::destructed == 0);
        
        assert(!wp.expired());
        ssp.reset();
        assert(count == allocated);
    }
    
    assert(allocated == deallocated);

    assert(Accountant::constructed == 1);
    assert(Accountant::destructed == 1);
    
    assert(allocate_called == 1);
    assert(deallocate_called == 1);
    assert(construct_called == 1);
    assert(destroy_called == 1);
 
    assert(new_called == 0);
    assert(delete_called == 0);

    allocated = 0;
    deallocated = 0;
    allocate_called = 0;
    deallocate_called = 0;
    construct_called = 0;
    destroy_called = 0;
 }

/*struct Enabled: public EnableSharedFromThis<Enabled> {
    SharedPtr<Enabled> get_shared() {
        return shared_from_this();
    }
};

void test_enable_shared_from_this() {
    {
        Enabled e;
        bool caught = false;
        try {
            e.get_shared();
        } catch (...) {
            caught = true;
        }
        assert(caught);
    }

    auto esp = makeShared<Enabled>();

    auto& e = *esp;
    auto sp = e.get_shared();

    assert(sp.use_count() == 2);

    esp.reset();
    assert(sp.use_count() == 1);

    sp.reset();
}*/

int mother_created = 0;
int mother_destroyed = 0;
int son_created = 0;
int son_destroyed = 0;

struct Mother {
    Mother() {
        ++mother_created;
    }
    virtual ~Mother() {
        ++mother_destroyed;
    }
};

struct Son: public Mother {
    Son() {
        ++son_created;
    }
    virtual ~Son() {
        ++son_destroyed;
    }
};

void test_inheritance_destroy() {
    {
        SharedPtr<Son> sp(new Son());

        SharedPtr<Mother> mp(new Mother());

        mp = sp;

        sp.reset(new Son());

    }
    assert(son_created == 2);
    assert(son_destroyed == 2);
    assert(mother_created == 3);
    assert(mother_destroyed == 3);
    
    mother_created = 0;
    mother_destroyed = 0;
    son_created = 0;
    son_destroyed = 0;

    {
        MyAllocator<Son> alloc;
        auto sp = allocateShared<Son>(alloc);

        SharedPtr<Mother> mp = sp;

        sp.reset(new Son());

    }
    assert(son_created == 2);
    assert(son_destroyed == 2);
    assert(mother_created == 2);
    assert(mother_destroyed == 2);

    assert(allocated == deallocated);
    assert(allocate_called == 1);
    assert(deallocate_called == 1);
    assert(construct_called == 1);
    assert(destroy_called == 1); 

    allocated = 0;
    deallocated = 0;
    allocate_called = 0;
    deallocate_called = 0;
    construct_called = 0;
    destroy_called = 0;
 }

int custom_deleter_called = 0;

struct MyDeleter {
    template <typename T>
    void operator()(T*) {
        ++custom_deleter_called;
    }
};

void test_custom_deleter() {
    MyDeleter deleter;
    int x = 0;
    
    new_called = 0;
    delete_called = 0;

    {
        SharedPtr<int> sp(&x, deleter);
        
        auto ssp = std::move(sp);

        auto sssp = ssp;

        ssp = makeShared<int>(5);

    }
    assert(custom_deleter_called == 1);
    
    // 1 for ControlBlock in sp and 1 for makeShared
    assert(new_called == 2);
    assert(delete_called == 2);

    new_called = 0;
    delete_called = 0;
    allocated = 0;
    deallocated = 0;
    allocate_called = 0;
    deallocate_called = 0;
    construct_called = 0;
    destroy_called = 0;
    custom_deleter_called = 0;

    Accountant::constructed = 0;
    Accountant::destructed = 0;

    Accountant acc;
    {
        MyAllocator<Accountant> alloc;
        MyDeleter deleter;

        SharedPtr<Accountant> sp(&acc, deleter, alloc);
        
        auto ssp = std::move(sp);

        auto sssp = ssp;

        ssp = makeShared<Accountant>();
    }

    assert(new_called == 1); // for makeShared
    assert(delete_called == 1);
    assert(allocate_called == 1);
    assert(deallocate_called == 1);
    assert(allocated == deallocated);

    assert(Accountant::constructed == 2);
    assert(Accountant::destructed == 1);

    assert(construct_called == 0);
    assert(destroy_called == 0);
    assert(custom_deleter_called == 1);
}

int main() {
    //static_assert(!std::is_base_of_v<std::shared_ptr<VerySpecialType>, SharedPtr<VerySpecialType>>,
    //        "don't try to use std smart pointers");
    
    //static_assert(!std::is_base_of_v<std::weak_ptr<VerySpecialType>, WeakPtr<VerySpecialType>>,
    //        "don't try to use std smart pointers");
    
    std::cerr << "Starting tests..." << std::endl;

    test_shared_ptr();
    std::cerr << "Test 1 (shared ptr) passed." << std::endl;

    test_weak_ptr();
    std::cerr << "Test 2 (weak ptr) passed." << std::endl;
    
    test_make_allocate_shared();
    std::cerr << "Test 3 (make/allocate shared) passed." << std::endl;
    
    //test_enable_shared_from_this();
    
    test_inheritance_destroy();    
    std::cerr << "Test 4 (inheritance) passed." << std::endl;

    test_custom_deleter();
    std::cerr << "Test 5 (custom deleter) passed." << std::endl;

    assert((!std::is_base_of_v<std::shared_ptr<VerySpecialType>, SharedPtr<VerySpecialType>>));
    
    assert((!std::is_base_of_v<std::weak_ptr<VerySpecialType>, WeakPtr<VerySpecialType>>));
 
    std::cout << 0;
}
