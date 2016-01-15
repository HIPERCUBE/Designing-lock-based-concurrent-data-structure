// 세밀한 잠금이 적용된 쓰레드세이프 큐

#include <memory>
#include <__mutex_base>

template<typename T>
class threadsafe_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;

    node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        std::lock_guard<std::mutex> head_lock(head_mutex);

        if (head.get() == get_tail()) {
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

public:
    threadsafe_queue() : head(new node), tail(head.get()) { }

    threadsafe_queue(const threadsafe_queue& other) = delete;

    threadsafe_queue& operator=(const threadsafe_queue& other) = delete;

    std::shared_ptr<T> try_pop() {
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head : std::shared_ptr<T>();
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        node* const new_tail = p.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(p);
        tail = new_value;
    }
};

// 6.1.1에서 보았던 가이드라인을 생각하며 이 코드를 공격적으로 봐보자.
// 파괴된 invariant를 보기전에, 이것들을 확실하게 알아야한다.
//  L tail->next == nullptr
//  L tail->data == nullptr
//  L head == tail 은 리스드가 비었음을 의미한다.
//  L 1개의 원소가 있을때는 head->next == tail 이다.
//  L 리스트의 각 노드 x에 대하여, 'x != tail', 'x->data'는 T 인스턴스를 가리키고, x->next는 리스트의 다음 노드를 가리킨다.
//  L head의 next 노드를 따가가다보면 결국 tail이 나올것이다.

// push() 자체는 똑바르다.
// 자료구조의 수정은 tail_mutex에의해 보호받고, 불변을 유지한다.
// (새 tail 노드는 비어있고, data와 next는 현재 리스트에서 가장 마지막에 있는 tail노드가 적용된다.)

// 흥미로운 부분은 try_pop()이다.
// tail 읽기를 보호하기위해, tail_mutex의 잠금만이 필요한것이 아니다.
// head의 데이터를 읽어서 경쟁상태를 만들면 안된다.
// 만약 그 mutex를 가지고 있지 않다면, try_pop()을 호출하는 쓰레드와 push()를 동시에 호출하는 쓰레드는 서로 정의된 작업순서가 없다.
// 게다가 각 멤버 함수가 mutex로 잠그고있으면, 다른 뮤텍스들로 잠거야한다.
// 같은 데이터에 접근할 가능성이 있다.
// 큐의 모든 데이터들은 push()로 들어갔다.
// 모든 쓰레드가 작업순서 정의 없이 같은 데이터에 접근하기 때문에, 5장에서 봤듯이 경쟁상태와 undefined behavior가 발생한다.
// 감사하게도 get_tail()의 tail_mutex 잠금은 모든것을 해결해준다.
// get_tail()의 호출은 push()의 호출과 같이 같은 뮤텍스를 잠그기때문에, 두 호출사이에 순서가 정의되어있다.
// tail의 전 값을 볼경우에는 get_tail()의 호출이 push()전에 일어나고,
// tail의 새 값을 보고 전 tail의 값을 새 데이터에 넣을때는, push()이후에 일어난다.