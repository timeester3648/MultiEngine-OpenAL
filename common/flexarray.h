#ifndef AL_FLEXARRAY_H
#define AL_FLEXARRAY_H

#include <cstddef>
#include <type_traits>

#include "almalloc.h"
#include "alspan.h"

namespace al {

/* Storage for flexible array data. This is trivially destructible if type T is
 * trivially destructible.
 */
template<typename T, size_t alignment, bool = std::is_trivially_destructible<T>::value>
struct FlexArrayStorage {
    alignas(alignment) const ::al::span<T> mData;

    static constexpr size_t Sizeof(size_t count, size_t base=0u) noexcept
    { return sizeof(FlexArrayStorage) + sizeof(T)*count + base; }

    FlexArrayStorage(size_t size) : mData{::new(static_cast<void*>(this+1)) T[size], size} { }
    ~FlexArrayStorage() = default;

    FlexArrayStorage(const FlexArrayStorage&) = delete;
    FlexArrayStorage& operator=(const FlexArrayStorage&) = delete;
};

template<typename T, size_t alignment>
struct FlexArrayStorage<T,alignment,false> {
    alignas(alignment) const ::al::span<T> mData;

    static constexpr size_t Sizeof(size_t count, size_t base=0u) noexcept
    { return sizeof(FlexArrayStorage) + sizeof(T)*count + base; }

    FlexArrayStorage(size_t size) : mData{::new(static_cast<void*>(this+1)) T[size], size} { }
    ~FlexArrayStorage() { std::destroy(mData.begin(), mData.end()); }

    FlexArrayStorage(const FlexArrayStorage&) = delete;
    FlexArrayStorage& operator=(const FlexArrayStorage&) = delete;
};

/* A flexible array type. Used either standalone or at the end of a parent
 * struct, with placement new, to have a run-time-sized array that's embedded
 * with its size.
 */
template<typename T, size_t alignment=alignof(T)>
struct FlexArray {
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using index_type = size_t;
    using difference_type = ptrdiff_t;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using Storage_t_ = FlexArrayStorage<element_type,alignment>;

    Storage_t_ mStore;

    static constexpr index_type Sizeof(index_type count, index_type base=0u) noexcept
    { return Storage_t_::Sizeof(count, base); }
    static std::unique_ptr<FlexArray> Create(index_type count)
    {
        void *ptr{al_calloc(alignof(FlexArray), Sizeof(count))};
        return std::unique_ptr<FlexArray>{al::construct_at(static_cast<FlexArray*>(ptr), count)};
    }

    FlexArray(index_type size) : mStore{size} { }
    ~FlexArray() = default;

    [[nodiscard]] auto size() const noexcept -> index_type { return mStore.mData.size(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return mStore.mData.empty(); }

    [[nodiscard]] auto data() noexcept -> pointer { return mStore.mData.data(); }
    [[nodiscard]] auto data() const noexcept -> const_pointer { return mStore.mData.data(); }

    [[nodiscard]] auto operator[](index_type i) noexcept -> reference { return mStore.mData[i]; }
    [[nodiscard]] auto operator[](index_type i) const noexcept -> const_reference { return mStore.mData[i]; }

    [[nodiscard]] auto front() noexcept -> reference { return mStore.mData.front(); }
    [[nodiscard]] auto front() const noexcept -> const_reference { return mStore.mData.front(); }

    [[nodiscard]] auto back() noexcept -> reference { return mStore.mData.back(); }
    [[nodiscard]] auto back() const noexcept -> const_reference { return mStore.mData.back(); }

    [[nodiscard]] auto begin() noexcept -> iterator { return mStore.mData.begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator { return mStore.mData.begin(); }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator { return mStore.mData.cbegin(); }
    [[nodiscard]] auto end() noexcept -> iterator { return mStore.mData.end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator { return mStore.mData.end(); }
    [[nodiscard]] auto cend() const noexcept -> const_iterator { return mStore.mData.cend(); }

    [[nodiscard]] auto rbegin() noexcept -> reverse_iterator { return end(); }
    [[nodiscard]] auto rbegin() const noexcept -> const_reverse_iterator { return end(); }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator { return cend(); }
    [[nodiscard]] auto rend() noexcept -> reverse_iterator { return begin(); }
    [[nodiscard]] auto rend() const noexcept -> const_reverse_iterator { return begin(); }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator { return cbegin(); }

    DEF_PLACE_NEWDEL()
};

} // namespace al

#endif /* AL_FLEXARRAY_H */
