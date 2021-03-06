//
// Created by jgoppert on 6/28/18.
//

#pragma once

using namespace std;

template <typename Derived, typename Base>
unique_ptr<Derived> static_unique_ptr_cast(unique_ptr<Base> &&p) {
  auto d = static_cast<Derived *>(p.release());
  assert(d);
  return unique_ptr<Derived>(d);
}

struct EnumClassHash {
  template <typename T> size_t operator()(T t) const {
    return static_cast<size_t>(t);
  }
};

// vim: set et fenc=utf-8 ff=unix sts=0 sw=2 ts=2 :
