#include <__mutex_base>
#include <queue>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() { }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
/* 1 */ value = std::move(*data_queue.front());
        data_queue.pop();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
/* 2 */ value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
/* 3 */ std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
/* 4 */ std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    void push(T new_value) {
/* 5 */ std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

// std::shared_ptr<>로 데이터를 홀딩한 결과는 다음과 같다.
//  L pop() 함수들은 새 데이터를 받기 위해 래펀런스 변수를 받아온다 