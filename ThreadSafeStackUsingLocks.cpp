#include <exception>
#include <stack>
#include <__mutex_base>

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
        // 데이터를 복사/이동할때 예외를 던지거나
        // 아래 자료구조를 확장하기에 충분하지 않은 메모리가 할당될 수 있음
        // may throw an exception if either copying/moving the data value throws an exception
        // or not enough memory can be allocated to extend the underlying data structure.
    }

    threadsafe_stack& operator=(const threadsafe_stack) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
/* 2 */ if (data.empty()) throw empty_stack();
        // 첫번째로 오버로딩한 pop()
        // 스스로 empty_stack 예외로 던진다.
        // 비어있는 상태일때 안전하게 해준다.
/* 3 */ std::shared_ptr<T> const res(std::make_shared(std::move(data.top())));
        // 몇가지 이유로 인해 예외가 발생할 수 있음
        // std::make_shared 호출은 새 객체를 메모리에 할당하지 못하면 예외를 던지고
        // 내부 데이터는 레퍼런스 카운팅을 필요로 하거나 반환될 아이템의 복사 생성자 혹은 이동 생성자는 새로 할당된 메모리로 복사/이동할때 예외를 던진다.
        // 두 경우 모두 C++ 런타임과 표준 라이브러리에서 메모리 누수를 막고, 제대로 새 객체가 소멸되도록 보장한다.
/* 4 */ data.pop();
        // 결과 반환으로 예외를 던지지 못하게 보장한다
        // exception-safe 함.
        return res;
    }

    void pop(T& value) {
        // 두번째로 오버로딩한 pop() 이것도 비슷하다.
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
/* 5 */ value = std::move(data.top());
        // 새 객체와 std::shared_ptr 인스턴스를 생성하는것보다
        // 복사 할당 또는 이동 할당 연산자는 예외를 던질 수 있다.
/* 6 */ data.pop();
        // data.pop()이 호출되기 전까지 자료 구조를 수정하지 않는것은 예외 발생을 막는것이다.
        // 또한 이것도 exception-safe하다.
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
    // 마지막으로 empty()는 어떠한 데이터도 수정하지 않는다.
    // 그러므로 이것도 exception-safe 하다.
};

// 기본적인 쓰레드 세이프를 제공하는 방법은 각각의 멤버 함수를 뮤텍스(m)로 잠그는 것이다.
// 이는 한번에 오직 한 쓰레드가 데이터에 접근할 수 있도록 한다.
// 그러므로써 모든 멤버 함수들은 불변을 유지하고 어떤한 쓰레드도 불변이 깨지는것을 볼 수 없다.

// empty()와 pop() 함수들은 경쟁 상태가 발생한 가능성이있다.
// 이 코드들은 명시적으로 pop()에 잠금이 걸려있을때 스택이 비어있는지 명시적으로 확인하기 때문에, 문제가 되지 않는다.
// pop()을 호출하는 부분에서 꺼낸 데이터를 바로 반환해 줌으로써 std::stack의 top(), pop() 멤버 함수에서 발생할 수 있는 잠재적인 경쟁상태를 피한다.

// 여기에는 예외가 발생할 수 있는 몇몇 소스가 있다
// 뮤텍스를 잠그는 것은 예외를 던질 수 있지만, 극히 드물다. (std::mutex 소스 참고)
// 가각의 멤버 함수에서 첫번째로 하는것이 뮤텍스를 잠그는 것이기도 하다.
// 어느 데이터도 수정되지 않기때문에 안전하다
// 뮤텍스를 잠금 해제하는것은 실패하지 않고 항상 안전하다.
// 그리고 std::lock_guard<>는 뮤텍스가 잠긴채로 남아있지 않게 방지한다.

// 유저코 교착상태가 발생할만한 부분이 몇군데 있다.
//