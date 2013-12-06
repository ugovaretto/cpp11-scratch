


transaction_t {
    transaction_t(T ) : access_policy_t(data) {}
    operator()(F&& f) {
        return f(begin().data);
    }
};


transation_t read(cref(bufferIn));

const float m =
    read([](const float* array, int size){
        return max(array, array + size);
    });
