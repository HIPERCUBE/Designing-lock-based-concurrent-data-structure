// Listing6_2, Listing6_3에서 보호받는 아이템(data_queue)과 뮤텍스를 사용했다.
// 세밀한 잠금을 사용하기 위해서 큐의 내부(구성부분)를 보고, 각 분리된 데이터 아이템 마다 뮤텍스를 할당해야한다.

// 가장 간단한 큐 자료구조는 링크드 리스트이다.
// 큐는 head 포인터를 포함하고 있다.
// head 포인터는 리스트의 첫번째 아이템을 가리키고, 각 아이템은 다음 다이템을 가리긴다.
// 데이터 아이템은 다음 아이템의 포인터로 헤드 포인터를 교체함으로써 큐에서 삭제되고, 이전 헤드가 반환된다. (데이터 꺼낼때 헤드 이동하고 꺼낸거임)

// 아이템들은 큐의 다른쪽 끝에 추가된다.(tail)
// 이렇게 하기 위해서는, 큐는 리스트의 마지막 아이템을 참조하는 tail 포인터도 가지고 있어야한다.
// 마지막 아이템의 next 포인터를 새 아이템으로 업데이트하고 tail 포인터가 새 아이템을 참조하게 하면서, 새 아이템이 추가된다.
// 리스트가 비어있을때는 head 와 tail 포인터 둘다 NULL이다.

// 아리에서 보게될 예제는 Listing6_2 큐 인터페이스의 cut-down 버전을 기반으로한 큐의 간단한 구현이다
// 이 큐는 오직 싱글 쓰레드만 지원하기 때문에, try_pop() 함수만 있고 wait_and_pop() 함수는 없다.


#include <memory>

template<typename T>
class queue {
private:
    struct node {
        T data;
        std::unique_ptr<node> next;

        node(T data_) : data(std::move(data_)) { }
    };

/* 1 */
    std::unique_ptr<node> head;
/* 2 */
    node* tail;

public:
    queue() { }

    queue(const queue& other) = delete;

    queue& operator=(const queue& other) = delete;

    std::shared_ptr<T> try_pop() {
        if (!head)
            return std::shared_ptr<T>();
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(head->data)));
        std::unique_ptr<node> const old_head = std::move(head);
/* 3 */ head = std::move(old_head->next);
        return res;
    }

    void push(T new_value) {
        std::unique_ptr<node> p(new node(std::move(new_value)));
        node* const new_tail = p.get();
        if (tail) {
/* 4 */     tail->next = std::move(p);
        } else {
/* 5 */     head = std::move(p);
        }
/* 6 */ tail = new_tail;
    }
};