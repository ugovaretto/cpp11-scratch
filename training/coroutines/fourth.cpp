
#include <cstring>
#include <iostream>
#include <utility>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>

// custom heap allocation, to simplify the code no
// sync mechanism is implemented, to create
// per-thread allocators simply replace 'static' with 'thread_local' or
// create <thread_id, MemPools> map

// As of GCC 11, CLang 12, clang still requires
//   -fcoroutines-ts
//   -stdlib=libc++
//   #include <experimental/...>
//   namespace std::experimental::... 

// NOTE: __GNUC__ might be defined even when using CLang
#if defined(__GNUC__) && !defined(__clang__)
#include <coroutine>
namespace CORO = std;
#elif defined(__clang__)
#include <experimental/coroutine>
namespace CORO = std::experimental;
#else
#error Unsupported compiler
#endif

/// Memory pool: sequence of fixed vectors holding fixed size chunks.
/// 
/// Vectors are never resized.
template <typename T = char, size_t S = 64>
struct MemPool {
    enum Size: size_t {
        ChunkSize = S
    };
    using Index = size_t;
    using Chunk = std::array<T, ChunkSize>;
    std::vector<Chunk> chunks_;
    std::unordered_map<void*, Index> alloc_;
    std::unordered_set<Index> free_;
    void* Alloc(size_t sz) {
        if(sz > ChunkSize || free_.empty()) return nullptr;
        else {
           auto nh = free_.extract(free_.begin());
           void* ptr = &chunks_[nh.value()];
           alloc_.insert({ptr, nh.value()});
           return ptr;
        }
    }
    template <typename F>
    bool Destroy(void* ptr, F&& destroyFun) {
        auto f = alloc_.find(ptr);
        if(f == alloc_.end()) return false;
        else {
            const size_t i = f->second;
            destroyFun(&chunks_[i]);
            free_.insert(std::move(alloc_.extract(f).mapped()));
            return true;
        }
    }
    MemPool(size_t numchunks = 8) : chunks_{numchunks} {
        for(size_t i = 0; i != chunks_.size(); ++i) free_.insert(i);
    }
};


/// Dynamic sequence of fixed size memory pools,
/// memory pools are added as needed but currently are never deleted
/// to avoid making the code too complex; could add a sentinel that
/// starts deleting pools after reaching a specific threshold. 
template < typename T = char, size_t S = 64, size_t N = 64>
struct MemPools {
    enum Size: size_t {
        ChunkSize = S,
        NumChunks = N
    };
    using Pool = MemPool<T,S>;
    using Pools = std::list<MemPool<T,S>>;
    using PoolsIter = typename std::list<MemPool<T,S>>::iterator;
    Pools pools_;
    std::unordered_map<void*, PoolsIter> ptrPool_;
    PoolsIter Free()  { // should return MemPool, like this we perform the 
        for(PoolsIter i = begin(pools_); i != end(pools_); ++i) {
            if(!i->free_.empty()) return i;
        }
        return end(pools_);
    }
    void* Alloc(size_t sz) {
        if(sz > ChunkSize) return nullptr;
        auto i = Free();
        if(i == end(pools_)) {
            pools_.push_front(Pool(NumChunks));
            i = begin(pools_);
        }
        void* p = i->Alloc(sz);
        ptrPool_.insert({p, i});
        return p;
    }
    template <typename F>
    bool Destroy(void* ptr, F&& destroyFun = [](void* ){}) {
        auto i = ptrPool_.find(ptr);
        if(i == ptrPool_.end()) return false;
        i->second->Destroy(ptr, destroyFun);
        ptrPool_.erase(i);
        return true;
    }
    MemPools(size_t numPools = 1) { pools_.push_front(Pool());}
};

static /*thread_local*/ MemPools memPoolsG;

/// coroutine management code
class Resumable {
    struct Promise {
        auto get_return_object() {
            return Resumable{
                CORO::coroutine_handle<Promise>::from_promise(*this)};
        }
        auto initial_suspend() { return CORO::suspend_always{}; }
        auto final_suspend() noexcept { return CORO::suspend_always{}; }
        //auto return_value(T);
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
        static void* operator new(std::size_t sz) {
            std::cout << "custom new for size " << sz << std::endl;
            void* p = memPoolsG.Alloc(sz);
            return p ? p : ::operator new(sz);
        }
        static void operator delete  (void* ptr) noexcept {
            std::cout << "custom delete called" << std::endl;
            if(!memPoolsG.Destroy(ptr, [](void* p){ 
                reinterpret_cast<Promise*>(p)->~Promise();
                })) ::operator delete(ptr);
        }
    };
    CORO::coroutine_handle<Promise> h_;
    explicit Resumable(CORO::coroutine_handle<Promise> h) : h_{h} {}

   public:
    using promise_type = Promise;
    //Resumable(Resumable&& r) : h_(std::exchange(r.h_, {})) {}
    ~Resumable() {
        if (h_) h_.destroy();
    }
    bool resume() {
        if (!h_.done()) {
            h_.resume();
        }
        return !h_.done();
    }
};

/// create coroutine
auto coroutine() -> Resumable {
    std::cout << "  3 " << " Suspend...";
    co_await CORO::suspend_always{};
    std::cout << "Resume " << "  5 ";
}

//
int main(int, char**) {
    std::cout << "1 ";
    auto resumable = coroutine();
    std::cout << "2 ";
    resumable.resume();
    std::cout << "4 ";
    resumable.resume();
    std::cout << std::endl;
    return 0;
}