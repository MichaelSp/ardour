#ifndef ARDOUR_GLIBMM_SLISTHANDLE_H
#define ARDOUR_GLIBMM_SLISTHANDLE_H

#include <glibmm/vectorutils.h>

#include <memory>
#include <utility>
#include <vector>

namespace Glib
{

template <typename T, typename Tr = Glib::Container_Helpers::TypeTraits<T>>
class SListHandle
{
public:
  using CType = typename Tr::CType;
  using CppType = typename Tr::CppType;
  using VectorType = std::vector<CppType>;
  using GSListKeeperType = typename Glib::Container_Helpers::GSListKeeper<Tr>;

  SListHandle() = default;

  explicit SListHandle(GSList* list) : SListHandle(list, Glib::OWNERSHIP_DEEP) {}

  explicit SListHandle(GSList* list, Glib::OwnershipType ownership)
  {
    for (GSList* node = list; node; node = node->next)
    {
      values_.push_back(Tr::to_cpp_type(static_cast<CType>(node->data)));
    }
    if (ownership == Glib::OWNERSHIP_SHALLOW || ownership == Glib::OWNERSHIP_DEEP)
    {
      if (ownership == Glib::OWNERSHIP_DEEP)
      {
        for (GSList* node = list; node; node = node->next)
        {
          Tr::release_c_type(static_cast<CType>(node->data));
        }
      }
      g_slist_free(list);
    }
  }

  template <typename InputIt>
  SListHandle(InputIt first, InputIt last) : values_(first, last)
  {
  }

  SListHandle(const VectorType& values) : values_(values) {}
  SListHandle(VectorType&& values) : values_(std::move(values)) {}

  SListHandle(const SListHandle& other) : values_(other.values_) {}
  SListHandle(SListHandle&&) noexcept = default;

  SListHandle& operator=(const SListHandle& other)
  {
    if (this != &other)
    {
      values_ = other.values_;
      keeper_.reset();
    }
    return *this;
  }

  SListHandle& operator=(SListHandle&&) noexcept = default;

  GSList* data() const
  {
    if (!keeper_)
    {
      auto list = Glib::Container_Helpers::create_gslist<Tr>(values_.begin(), values_.end());
      keeper_ = std::make_unique<GSListKeeperType>(list, Glib::OWNERSHIP_SHALLOW);
    }

    return keeper_->data();
  }

  std::size_t size() const noexcept { return values_.size(); }
  bool empty() const noexcept { return values_.empty(); }

  const CppType& operator[](std::size_t index) const { return values_[index]; }
  operator const VectorType&() const noexcept { return values_; }

  typename VectorType::const_iterator begin() const noexcept { return values_.begin(); }
  typename VectorType::const_iterator end() const noexcept { return values_.end(); }

private:
  VectorType values_;
  mutable std::unique_ptr<GSListKeeperType> keeper_;
};

} // namespace Glib

#endif
