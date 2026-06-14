#ifndef ARDOUR_GLIBMM_LISTHANDLE_H
#define ARDOUR_GLIBMM_LISTHANDLE_H

#include <glibmm/vectorutils.h>

#include <memory>
#include <utility>
#include <vector>

namespace Glib
{

template <typename T, typename Tr = Glib::Container_Helpers::TypeTraits<T>>
class ListHandle
{
public:
  using CType = typename Tr::CType;
  using CppType = typename Tr::CppType;
  using VectorType = std::vector<CppType>;
  using GListKeeperType = typename Glib::Container_Helpers::GListKeeper<Tr>;

  ListHandle() = default;

  explicit ListHandle(GList* list) : ListHandle(list, Glib::OWNERSHIP_DEEP) {}

  explicit ListHandle(GList* list, Glib::OwnershipType ownership)
  {
    for (GList* node = list; node; node = node->next)
    {
      values_.push_back(Tr::to_cpp_type(static_cast<CType>(node->data)));
    }
    if (ownership == Glib::OWNERSHIP_SHALLOW || ownership == Glib::OWNERSHIP_DEEP)
    {
      if (ownership == Glib::OWNERSHIP_DEEP)
      {
        for (GList* node = list; node; node = node->next)
        {
          Tr::release_c_type(static_cast<CType>(node->data));
        }
      }
      g_list_free(list);
    }
  }

  template <typename InputIt>
  ListHandle(InputIt first, InputIt last) : values_(first, last)
  {
  }

  ListHandle(const VectorType& values) : values_(values) {}
  ListHandle(VectorType&& values) : values_(std::move(values)) {}

  ListHandle(const ListHandle& other) : values_(other.values_) {}
  ListHandle(ListHandle&&) noexcept = default;

  ListHandle& operator=(const ListHandle& other)
  {
    if (this != &other)
    {
      values_ = other.values_;
      keeper_.reset();
    }
    return *this;
  }

  ListHandle& operator=(ListHandle&&) noexcept = default;

  GList* data() const
  {
    if (!keeper_)
    {
      auto list = Glib::Container_Helpers::create_glist<Tr>(values_.begin(), values_.end());
      keeper_ = std::make_unique<GListKeeperType>(list, Glib::OWNERSHIP_SHALLOW);
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
  mutable std::unique_ptr<GListKeeperType> keeper_;
};

} // namespace Glib

#endif
