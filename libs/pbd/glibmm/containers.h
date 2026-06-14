#ifndef ARDOUR_GLIBMM_CONTAINERS_H
#define ARDOUR_GLIBMM_CONTAINERS_H

#include <optional>

#include <glibmm/arrayhandle.h>
#include <glibmm/listhandle.h>
#include <glibmm/slisthandle.h>

namespace Glib
{

template <typename T>
class List_Iterator
{
public:
  using value_type = T;

  List_Iterator(GList* node = nullptr, GList* /*end*/ = nullptr) : node_(node) {}

  bool operator==(const List_Iterator& other) const { return node_ == other.node_; }
  bool operator!=(const List_Iterator& other) const { return node_ != other.node_; }

  T& operator*() const
  {
    if (!cache_)
    {
      cache_.emplace(static_cast<typename T::BaseObjectType*>(node_ ? node_->data : nullptr));
    }
    return *cache_;
  }

  T* operator->() const { return &(**this); }

  List_Iterator& operator++()
  {
    if (node_)
    {
      node_ = node_->next;
    }
    cache_.reset();
    return *this;
  }

  List_Iterator operator++(int)
  {
    List_Iterator tmp(*this);
    ++*this;
    return tmp;
  }

  List_Iterator& operator--()
  {
    if (node_)
    {
      node_ = node_->prev;
    }
    cache_.reset();
    return *this;
  }

  List_Iterator operator--(int)
  {
    List_Iterator tmp(*this);
    --*this;
    return tmp;
  }

private:
  GList* node_ = nullptr;
  mutable std::optional<T> cache_;
};

template <typename Iter>
class List_ConstIterator
{
public:
  using value_type = typename Iter::value_type;

  List_ConstIterator() = default;
  explicit List_ConstIterator(const Iter& iter) : iter_(iter) {}

  bool operator==(const List_ConstIterator& other) const { return iter_ == other.iter_; }
  bool operator!=(const List_ConstIterator& other) const { return iter_ != other.iter_; }

  const value_type& operator*() const { return *iter_; }
  const value_type* operator->() const { return &(*iter_); }

  List_ConstIterator& operator++()
  {
    ++iter_;
    return *this;
  }

  List_ConstIterator operator++(int)
  {
    List_ConstIterator tmp(*this);
    ++*this;
    return tmp;
  }

private:
  Iter iter_;
};

template <typename Iter>
class List_ReverseIterator
{
public:
  using value_type = typename Iter::value_type;

  List_ReverseIterator() = default;
  explicit List_ReverseIterator(const Iter& iter) : iter_(iter) {}

  bool operator==(const List_ReverseIterator& other) const { return iter_ == other.iter_; }
  bool operator!=(const List_ReverseIterator& other) const { return iter_ != other.iter_; }

  value_type& operator*() const
  {
    auto tmp = iter_;
    --tmp;
    return *tmp;
  }

  value_type* operator->() const { return &(**this); }

  List_ReverseIterator& operator++()
  {
    --iter_;
    return *this;
  }

  List_ReverseIterator operator++(int)
  {
    List_ReverseIterator tmp(*this);
    ++*this;
    return tmp;
  }

private:
  Iter iter_;
};

#endif
