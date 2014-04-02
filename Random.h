//author: Ugo Varetto
//minimal wrapper over <random> types, main reason to use this calss is to have a place where
//to store an instance of the random engine and distribution types
#pragma once
#include <random>

template < typename T >
struct SelectUniformDistribution;

template <>
struct SelectUniformDistribution< int > {
    using type = std::uniform_int_distribution< int >;
};

template <>
struct SelectUniformDistribution< float > {
    using type = std::uniform_real_distribution< float >;
};

template <>
struct SelectUniformDistribution< double > {
    using type = std::uniform_real_distribution< double >;
};

template < typename T, 
           typename RandomEngineT = std::default_random_engine,
           typename DistributionTypeT = typename SelectUniformDistribution< T >::type >
class Random {
public:
    using RandomEngineType = RandomEngineT;
    using DistributionType = DistributionTypeT;
public:
    Random(T low, T high) : low_(low),
                            high_(high),
                            rng_(std::random_device{}()),
                            dist_(low, high) {}
    T operator()() const {
        return dist_(rng_);               
    }
    void reset() {
        dist_.reset();
    }
    T min() const { return dist_.min(); }
    T max() const { return dist_.max(); }
    void seed(T s) { rng_.seed(s); }
    template < typename SSeqT >
    void seed(const SSeqT& seq) {
        rng_.seed(seq);
    }
private:
    T low_;
    T high_;
    mutable RandomEngineType rng_;
    mutable DistributionType dist_;
};
