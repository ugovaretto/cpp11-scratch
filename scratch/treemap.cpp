#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>



template < typename K,
           typename T >
class treemap_t {
    std::vector< K > key_;
    std::vector< T > data_;
    std::unique_ptr< treemap_t > less_;
    std::unique_ptr< treemap_t > geq_;
public:
    treemap_t(const treemap_t& n) : key_(n.key_), data_(n.data_) {
        if(n.less_) less_ = new treemap_t(n.less_);
        if(n.geq_)  geq_  = new treemap_t(n.geq_);
    }
    treemap_t(treemap_t&& n) = default;
    treemap_t& operator=(treemap_t n) {
        key_ = std::move(n.key_);
        data_ = std::move(n.data_);
        less_ = n.less_;
        geq_ = n.geq_;
    }
    void add(const std::vector< K >& k, const T& v) {
        if(key_.empty()) key_ = k;
        if(k == key_) {
            data_.push_back(v);
            return; 
        }
        if(std::lexicographical_compare(k.begin(), k.end(), key_.begin())) {
            if(!less_) less_ = new treemap_t;
            less_->add(k, v); 
        } else {
            if(!geq_) geq_ = new treemap_t;
            geq_->add(k, v);
        }
    }
    void findall(const std::vector< K >& key,
                 std::vector< std::reference_wrapper< T > >& res) {    
        if(std::equal(key.begin(), key.end(), key_.begin())) {
            std::back_inserter< T > b(res); 
            std::copy(data_.begin(), data_.end(), b);
            if(less_) less_->findall(key, res);
            if(geq_) geq_->findall(key, res); 
        }
    }
    void findall(const std::vector< K >& key,
                 std::vector< std::reference_wrapper< T > >& res) const {    
        if(std::equal(key.begin(), key.end(), key_.begin())) {
            std::back_inserter< T > b(res); 
            std::copy(data_.begin(), data_.end(), b);
            if(less_) less_->findall(key, res);
            if(geq_) geq_->findall(key, res); 
        }
    }
    const T& find(const std::vector< K >& key) const {        
    }
    T& find(const std::vector< K >& key) {

    }
    bool empty() const {
        return key_.empty();
    }
};


int main(int, char**) {
    treemap_t< char, int > n;
    return 0;
}
