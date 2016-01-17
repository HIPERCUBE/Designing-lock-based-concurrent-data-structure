#include <exception>
#include <stack>
#include <mutex>

struct empty_stack : std::exception {
    const char* what() const throw;
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() { }

    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
/* 1 */ data = other.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
/* 2 */ if (data.empty()) throw empty_stack();
/* 3 */ std::shared_ptr<T> const res(std::make_shared(std::move(data.top())));
/* 4 */ data.pop();
        return res;
    }

    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
/* 5 */ value = std::move(data.top());
/* 6 */ data.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};