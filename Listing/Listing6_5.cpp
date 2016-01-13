// 분리된 데이터로 동시성을 가능하게 하기

// Listing6_4에서 보았던 문제점을 값이 없는 더미 노드를 미리 할당함으로써 해결할 수 있다.
// 더미 노드를 미리 할당하면 큐에는 항상 적어도 1개 이상의 노드(head 이면서 tail)가 있다.
// 이제 빈 큐의 head 와 tail을 NULL이 아니라 더미 노드를 가리킨다.
// 큐가 비어있을때 try_pop()이 head->next에 접근하지 않기때문에 안전하다.
// 큐에 노드를 추가한다면, head와 tail은 각각 다른 노드들을 가리키게된다.
// 그래서 head->next와 tail->next사이에 경쟁이 없게된다.
// 단점은 더미 노드들을 위해 포인터로 데이터를 저장하는 추가의 인다이렉션이 들어간다는 것이다.
// 아래에서 어떻게 구현하는제 보게될것이다.


#include <memory>

template<typename T>
class queue {
private:
    struct node {
/* 1 */ std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node* tail;

public:
    /* 2 */
    queue() : head(new node), tail(head.get()) { }

    queue(const queue& other) = delete;

    queue& operator=(const queue& other) = delete;

    std::shared_ptr<T> try_pop() {
/* 3 */ if (head.get() == tail) {
            return std::shared_ptr<T>();
        }
/* 4 */ std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
/* 5 */ head = std::move(old_head->next);
/* 6 */ return res;
    }

    void push(T new_value) {
/* 7 */ std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
/* 8 */ std::unique_ptr<node> p(new node);
/* 9 */ tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};