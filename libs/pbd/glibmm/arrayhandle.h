#ifndef ARDOUR_GLIBMM_ARRAYHANDLE_H
#define ARDOUR_GLIBMM_ARRAYHANDLE_H

#include <glibmm/vectorutils.h>

#include <memory>
#include <utility>
#include <vector>

namespace Glib
{

template <typename T, typename Tr = Glib::Container_Helpers::TypeTraits<T>>
class ArrayHandle
{
public:
  using CType = typename Tr::CType;
  using CTypeNonConst = typename Tr::CTypeNonConst;
  using CppType = typename Tr::CppType;
  using VectorType = std::vector<CppType>;
  using ArrayKeeperType = typename Glib::Container_Helpers::ArrayKeeper<Tr>;

  ArrayHandle() = default;

  explicit ArrayHandle(CTypeNonConst* array)
    : ArrayHandle(array, Glib::OWNERSHIP_DEEP)
  {
  }

  explicit ArrayHandle(const CType* array)
    : ArrayHandle(array, Glib::OWNERSHIP_DEEP)
  {
  }

  ArrayHandle(CTypeNonConst* array, std::size_t array_size, Glib::OwnershipType ownership)
  {
    if (array)
    {
      values_.reserve(array_size);
      for (std::size_t i = 0; i < array_size; ++i)
      {
        values_.push_back(Tr::to_cpp_type(array[i]));
      }
      if (ownership == Glib::OWNERSHIP_SHALLOW || ownership == Glib::OWNERSHIP_DEEP)
      {
        if (ownership == Glib::OWNERSHIP_DEEP)
        {
          for (std::size_t i = 0; i < array_size; ++i)
          {
            Tr::release_c_type(array[i]);
          }
        }
        g_free(array);
      }
    }
  }

  ArrayHandle(CTypeNonConst* array, Glib::OwnershipType ownership)
  {
    if (array)
    {
      std::size_t size = 0;
      while (array[size])
      {
        ++size;
      }
      values_.reserve(size);
      for (std::size_t i = 0; i < size; ++i)
      {
        values_.push_back(Tr::to_cpp_type(array[i]));
      }
      if (ownership == Glib::OWNERSHIP_SHALLOW || ownership == Glib::OWNERSHIP_DEEP)
      {
        if (ownership == Glib::OWNERSHIP_DEEP)
        {
          for (std::size_t i = 0; i < size; ++i)
          {
            Tr::release_c_type(array[i]);
          }
        }
        g_free(array);
      }
    }
  }

  ArrayHandle(const CType* array, Glib::OwnershipType ownership)
    : ArrayHandle(const_cast<CTypeNonConst*>(array), ownership)
  {
  }

  template <typename InputIt>
  ArrayHandle(InputIt first, InputIt last) : values_(first, last)
  {
  }

  ArrayHandle(const VectorType& values) : values_(values) {}
  ArrayHandle(VectorType&& values) : values_(std::move(values)) {}

  ArrayHandle(const ArrayHandle& other) : values_(other.values_) {}
  ArrayHandle(ArrayHandle&&) noexcept = default;

  ArrayHandle& operator=(const ArrayHandle& other)
  {
    if (this != &other)
    {
      values_ = other.values_;
      keeper_.reset();
    }
    return *this;
  }

  ArrayHandle& operator=(ArrayHandle&&) noexcept = default;

  const CType* data() const
  {
    if (!keeper_)
    {
      auto array = Glib::Container_Helpers::create_array<Tr>(values_.begin(), values_.size());
      keeper_ = std::make_unique<ArrayKeeperType>(array, values_.size(), Glib::OWNERSHIP_SHALLOW);
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
  mutable std::unique_ptr<ArrayKeeperType> keeper_;
};

using StringArrayHandle = ArrayHandle<Glib::ustring>;

} // namespace Glib

#endif
