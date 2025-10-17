#ifndef __DLIST_H
#define __DLIST_H

/*
 * 本文件源自 Linux 内核代码 (include/linux/list.h)
 * 经过修改，仅移除了硬件预取(hardware prefetching)相关的代码。
 * 在此声明版权，所有功劳归于其原作者。
 * Kulesh Shanmugasundaram (kulesh [squiggly] isis.poly.edu)
 */

/*
 * 一个简单的双向链表实现。
 *
 * 当我们需要操作整个链表而不是单个节点时，一些内部函数（以 "__"
 * 开头）会非常有用。 因为在这种情况下，我们可能已经知道了前一个和后一个节点，
 * 直接使用这些内部函数可以生成比通用单节点操作函数更高效的代码。
 */

/**
 * container_of - 从结构体成员的指针获取其容器结构体的指针
 *
 * @ptr: 指向成员的指针。
 * @type: 成员所在的容器结构体的类型。
 * @member: 成员在结构体中的名称。
 *
 */
// 计算一个成员在结构体中的偏移量
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE*)0)->MEMBER)
#endif

// container_of 宏：根据成员指针ptr，计算出整个结构体的起始地址
#define container_of(ptr, type, member)                            \
  ({                                                               \
    /* 检查 ptr 和 member 的类型是否匹配，__mptr 是一个临时指针 */ \
    const typeof(((type*)0)->member)* __mptr = (ptr);              \
    /* 将成员指针减去它在结构体中的偏移量，得到结构体的起始地址 */ \
    (type*)((char*)__mptr - offsetof(type, member));               \
  })

/*
 * 下面是两个非 NULL 的指针常量，在正常情况下访问它们会引发页错误（Page
 * Fault）。 这两个“毒药”指针用于验证用户没有使用未初始化的链表节点。
 * 如果一个被删除的节点的指针被设置为它们，再次使用时就会立即崩溃，便于调试。
 */
#define LIST_POISON1 ((void*)0x00100100)
#define LIST_POISON2 ((void*)0x00200200)  // 原文件是0x00200，为了区分，稍作修改

// 标准链表节点（又称“小结构体”）
struct list_head {
  struct list_head* prev;  // 指向前一个节点
  struct list_head* next;  // 指向后一个节点
};

// 宏：静态初始化一个链表头
#define LIST_HEAD_INIT(name) {&(name), &(name)}

// 宏：定义并初始化一个链表头变量
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

// 宏：动态初始化一个链表头（使其指向自身）
// 注意：宏定义中如果有多条语句，必须用 do{}while(0)
// 包裹，使其在语法上成为一条单独的语句。
#define INIT_LIST_HEAD(ptr) \
  do {                      \
    (ptr)->next = (ptr);    \
    (ptr)->prev = (ptr);    \
  } while (0)

/*
 * 在两个已知的、连续的节点之间插入一个新节点。
 *
 * 这个函数仅用于内部的链表操作，因为它假定我们已经知道了 prev 和 next 节点！
 */
static inline void __list_add(struct list_head* new, struct list_head* prev,
                              struct list_head* next) {
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

/**
 * list_add – 添加一个新节点（头插法）
 * @new:  要添加的新节点
 * @head: 链表头，新节点将插入到此头节点之后
 *
 * 在指定的头节点后插入一个新节点。
 * 这种方式很适合用来实现“栈”（后进先出）。
 */
static inline void list_add(struct list_head* new, struct list_head* head) {
  __list_add(new, head, head->next);
}

/**
 * list_add_tail – 添加一个新节点（尾插法）
 * @new:  要添加的新节点
 * @head: 链表头，新节点将插入到此头节点之前
 *
 * 在指定的头节点前插入一个新节点。
 * 这种方式很适合用来实现“队列”（先进先出）。
 */
static inline void list_add_tail(struct list_head* new,
                                 struct list_head* head) {
  __list_add(new, head->prev, head);
}

/*
 * 通过让前后两个节点互相指向来删除一个链表节点。
 *
 * 这个函数仅用于内部的链表操作，因为它假定我们已经知道了 prev 和 next 节点！
 */
static inline void __list_del(struct list_head* prev, struct list_head* next) {
  next->prev = prev;
  prev->next = next;
}

/**
 * list_del – 从链表中删除一个节点
 * @entry: 要从链表中删除的节点。
 * 注意：此操作后，对该节点调用 list_empty() 的结果是未定义的。
 *       该节点处于一个不确定的状态。
 */
static inline void list_del(struct list_head* entry) {
  __list_del(entry->prev, entry->next);
  // 将被删除节点的指针设置为“毒药”，防止误用（这里简单设为0）
  entry->next = (void*)0;
  entry->prev = (void*)0;
}

/**
 * list_del_init – 从链表中删除一个节点并重新初始化它
 * @entry: 要从链表中删除的节点。
 */
static inline void list_del_init(struct list_head* entry) {
  __list_del(entry->prev, entry->next);
  INIT_LIST_HEAD(entry);  // 重新初始化，使其成为一个空的独立链表
}

/**
 * list_move – 从一个链表中删除节点，并将其作为另一个链表的头节点添加
 * @list: 要移动的节点
 * @head: 目标链表的头，移动的节点将成为它的新头节点（紧随其后）
 */
static inline void list_move(struct list_head* list, struct list_head* head) {
  __list_del(list->prev, list->next);
  list_add(list, head);
}

/**
 * list_move_tail – 从一个链表中删除节点，并将其作为另一个链表的尾节点添加
 * @list: 要移动的节点
 * @head: 目标链表的头，移动的节点将成为它的新尾节点（插在它之前）
 */
static inline void list_move_tail(struct list_head* list,
                                  struct list_head* head) {
  __list_del(list->prev, list->next);
  list_add_tail(list, head);
}

/**
 * list_empty – 测试一个链表是否为空
 * @head: 需要测试的链表头。
 */
static inline int list_empty(struct list_head* head) {
  return head->next == head;
}

/*
 * 内部函数，用于拼接两个链表的核心逻辑。
 */
static inline void __list_splice(struct list_head* list,
                                 struct list_head* head) {
  struct list_head* first = list->next;
  struct list_head* last = list->prev;
  struct list_head* at = head->next;

  first->prev = head;
  head->next = first;

  last->next = at;
  at->prev = last;
}

/**
 * list_splice – 连接两个链表
 * @list: 要被并入的新链表的头。
 * @head: 目标链表的头，新链表将被插入到它的后面。
 */
static inline void list_splice(struct list_head* list, struct list_head* head) {
  if (!list_empty(list)) __list_splice(list, head);
}

/**
 * list_splice_init – 连接两个链表，并重新初始化被清空的那个链表
 * @list: 要被并入的新链表的头。
 * @head: 目标链表的头，新链表将被插入到它的后面。
 *
 * 操作完成后，@list 所指向的链表头会被重新初始化。
 */
static inline void list_splice_init(struct list_head* list,
                                    struct list_head* head) {
  if (!list_empty(list)) {
    __list_splice(list, head);
    INIT_LIST_HEAD(list);
  }
}

/**
 * list_entry – 根据链表节点指针获取其宿主结构体的指针
 * @ptr:    指向 `struct list_head` 成员的指针。
 * @type:   宿主结构体的类型。
 * @member: `list_head` 成员在宿主结构体中的名字。
 */
#define list_entry(ptr, type, member) \
  container_of(ptr, type,             \
               member)  // list_entry 本质上就是 container_of 的一个别名

/**
 * list_for_each - 遍历链表
 * @pos:  一个 `struct list_head` 指针，用作循环计数器（游标）。
 * @head: 你要遍历的链表的头。
 */
#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev - 反向遍历链表
 * @pos:  一个 `struct list_head` 指针，用作循环计数器（游标）。
 * @head: 你要遍历的链表的头。
 */
#define list_for_each_prev(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - 安全地遍历链表（防止在遍历过程中删除节点导致循环中断）
 * @pos:  一个 `struct list_head` 指针，用作循环计数器（游标）。
 * @n:    另一个 `struct list_head` 指针，用作临时存储，保存下一个节点的位置。
 * @head: 你要遍历的链表的头。
 */
#define list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * list_for_each_entry - 遍历一个包含特定类型数据的链表
 * @pos:    一个指向你的数据结构类型的指针，用作循环计数器。
 * @head:   你要遍历的链表的头。
 * @member: `list_head` 成员在你的数据结构中的名字。
 */
#define list_for_each_entry(pos, head, member)               \
  for (pos = list_entry((head)->next, typeof(*pos), member); \
       &pos->member != (head);                               \
       pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - 安全地遍历一个包含特定类型数据的链表
 * @pos:    一个指向你的数据结构类型的指针，用作循环计数器。
 * @n:      另一个同类型的指针，用作临时存储。
 * @head:   你要遍历的链表的头。
 * @member: `list_head` 成员在你的数据结构中的名字。
 */
#define list_for_each_entry_safe(pos, n, head, member)        \
  for (pos = list_entry((head)->next, typeof(*pos), member),  \
      n = list_entry(pos->member.next, typeof(*pos), member); \
       &pos->member != (head);                                \
       pos = n, n = list_entry(n->member.next, typeof(*n), member))

#endif /* __DLIST_H */