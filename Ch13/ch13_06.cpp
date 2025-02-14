#include "ch13_06.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

void Strvec::push_back(const string &s) {
  chk_n_alloc();
  // 在first_free指向的元素中构造s的副本
  alloc.construct(first_free++, s);
}

pair<string *, string *> Strvec::alloc_n_copy(const string *b,
                                              const string *e) {
  // 分配空间保存给定范围中的元素
  auto data = alloc.allocate(e - b);
  // 初始化并返回一个pair，该pair由data和uninitialized_copy的返回值构成
  return {data, uninitialized_copy(b, e, data)};
}

void Strvec::free() {
  // 不能传递给deallocate一个空指针，如果elements为0，函数什么也不做
  if (elements) {
    // 逆序销毁旧元素
    // for (auto p = first_free; p != elements;) {
    //   alloc.destroy(--p);
    // }
    // 使用lambda表达式
    for_each(elements, first_free,
             [this](string &rhs) { alloc.destroy(&rhs); });

    alloc.deallocate(elements, cap - elements);
  }
}

Strvec::Strvec(const Strvec &s) {
  // 调用alloc_n_copy分配空间以容纳与s中一样多的元素
  auto newdata = alloc_n_copy(s.begin(), s.end());
  elements = newdata.first;
  first_free = cap = newdata.second;
}

Strvec::~Strvec() { free(); }

Strvec &Strvec::operator=(const Strvec &rhs) {
  // 调用alloc_n_copy分配内存，大小与rhs中元素占用的空间一样大
  auto data = alloc_n_copy(rhs.begin(), rhs.end());
  free();
  elements = data.first;
  first_free = cap = data.second;
  return *this;
}

Strvec &Strvec::operator=(Strvec &&rhs) noexcept {
  // 直接检测自赋值
  if (this != &rhs) {
    free();
    elements = rhs.elements;
    first_free = rhs.first_free;
    cap = rhs.cap;
    rhs.elements = rhs.first_free = rhs.cap = nullptr;
  }
  return *this;
}

void Strvec::reallocate() {
  // 我们将分配当前大小两倍的内存空间
  auto newcapacity = size() ? 2 * size() : 1;
  // 分配新内存
  auto newdata = alloc.allocate(newcapacity);
  // 将数据从旧内存移动到新内存
  auto dest = newdata;   // 指向新数组中下一个空闲位置
  auto elem = elements;  // 指向旧数组中下一个元素
  for (size_t i = 0; i != size(); ++i) {
    alloc.construct(dest++, std::move(*elem++));
  }
  free();  // 一旦我们移动完元素就释放旧内存
  // 更新我们的数据结构，执行新元素
  elements = newdata;
  first_free = dest;
  cap = elements + newcapacity;
}

Strvec::Strvec(initializer_list<string> il) {
  auto newdata = alloc_n_copy(il.begin(), il.end());
  elements = newdata.first;
  first_free = cap = newdata.second;
}

Strvec::Strvec(Strvec &&s) noexcept
    : elements(s.elements), first_free(s.first_free), cap(s.cap) {
  // 将s置于可析构的状态
  s.elements = s.first_free = s.cap = nullptr;
}

void Strvec::reserve(size_t n) {
  if (n <= capacity()) {
    return;
  }
  auto newdata = alloc.allocate(n);
  auto dest = newdata;
  auto elem = elements;
  for (size_t i = 0; i != size(); ++i) {
    alloc.construct(dest++, std::move(*elem++));
  }
  free();
  elements = newdata;
  first_free = dest;
  cap = elements + n;
}

void Strvec::resize(size_t n) {
  if (n > size()) {
    if (n > capacity()) {
      reserve(n * 2);
    }
    for (size_t i = size(); i != n; ++i) {
      alloc.construct(first_free++, "");
    }
  } else if (n < size()) {
    while (first_free != elements + n) {
      alloc.destroy(--first_free);
    }
  }
}

// 定义静态成员
allocator<string> Strvec::alloc = allocator<string>();

int main() {
  // 右值引用 只能绑定到将要销毁的对象
  int i = 42;
  const int &rl = i * 42;
  int &&ri = i * 10;
  // 获得一个左值的右值引用
  int &&rr = std::move(ri);

  return 0;
}
