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

