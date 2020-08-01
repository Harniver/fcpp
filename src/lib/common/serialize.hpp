// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file serialize.hpp
 * @brief Implementation of serialization operations for standard and FCPP classes.
 */

#ifndef FCPP_COMMON_SERIALIZE_H_
#define FCPP_COMMON_SERIALIZE_H_

#include <cstring>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <type_traits>

#include "lib/internal/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of common use.
namespace common {


//! @cond INTERNAL
namespace details {
    //! @brief Proxy function copying memory without unwanted warnings.
    void copy(const void* x, const void* y, size_t l) {
        memcpy((void*)x, y, l);
    }
}
//! @endcond


//! @brief Stream-like object for input or output serialization (depending on `io`).
template <bool io>
class sstream;


//! @brief Stream-like object for input serialization.
//! @{
template <>
class sstream<false> {
  public:
    //! @brief Constructor from raw data.
    sstream(std::vector<char> data) : m_data(data), m_idx(0) {}

    //! @brief Reads a trivial type from the stream.
    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    sstream& read(T& x, size_t l = sizeof(T)) {
        details::copy(&x, m_data.data() + m_idx, l);
        m_idx += l;
        return *this;
    }

  private:
    //! @brief The raw data.
    std::vector<char> m_data;
    //! @brief The read index.
    size_t m_idx;
};
using isstream = sstream<false>;
//! @}


//! @brief Stream-like object for output serialization.
//! @{
template <>
class sstream<true> {
  public:
    //! @brief Default constructor.
    sstream() = default;

    //! @brief Conversion to raw data.
    operator std::vector<char>() {
        return m_data;
    }

    //! @brief Writes a trivial type from the stream.
    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    sstream& write(T const& x, size_t l = sizeof(T)) {
        m_data.resize(m_data.size() + l);
        details::copy(m_data.data() + m_data.size() - l, &x, l);
        return *this;
    }

  private:
    //! @brief The raw data.
    std::vector<char> m_data;
};
using osstream = sstream<true>;
//! @}


//! @cond INTERNAL
namespace details {
    //! @brief Checks whether a class has a member serialize function.
    template<typename C>
    struct has_serialize {
      private:
        struct tag;

        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(std::declval<T>().serialize(std::declval<tag&>())),
            tag&
        >::type;

        template <typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = type::value;
    };
}


//! @brief Serialisation from trivial types.
template <typename T>
std::enable_if_t<std::is_trivially_copyable<T>::value, isstream&>
inline operator&(isstream& is, T& x) {
    return is.read(x);
}

//! @brief Serialisation to trivial types.
template <typename T>
std::enable_if_t<std::is_trivially_copyable<T>::value, osstream&>
inline operator&(osstream& os, T& x) {
    return os.write(x);
}

//! @brief Serialisation from/to user classes.
template <bool io, typename T>
std::enable_if_t<details::has_serialize<T>::value, sstream<io>&>
inline operator&(sstream<io>& is, T& x) {
    return x.serialize(is);
}

//! @brief Serialisation from/to standard containers.
template <bool io, typename T>
inline std::enable_if_t<(not details::has_serialize<T>::value) and (not std::is_trivially_copyable<T>::value), sstream<io>&>
operator&(sstream<io>&, T&);


//! @brief Serialisation from an input stream.
template <typename T>
inline isstream& operator>>(isstream& is, T& x) {
    return is & x;
}


//! @brief Serialisation to an output stream.
template <typename T>
inline osstream& operator<<(osstream& os, T const& x) {
    return os & ((T&)x);
}


//! @cond INTERNAL
namespace details {
    //! @brief Serialization of indexed classes.
    //! @{
    template <typename S, typename T>
    S& indexed_serialize(S& s, T&, std::index_sequence<>) {
        return s;
    }

    template <typename S, typename T, size_t i, size_t... is>
    S& indexed_serialize(S& s, T& x, std::index_sequence<i, is...>) {
        s & std::get<i>(x);
        return indexed_serialize(s, x, std::index_sequence<is...>{});
    }

    template <typename S, typename... Ts>
    S& serialize(S& s, std::tuple<Ts...>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template <typename S, typename T, size_t n>
    S& serialize(S& s, std::array<T, n>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<n>{});
    }

    template <typename S, typename T, typename U>
    S& serialize(S& s, std::pair<T, U>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<2>{});
    }
    //! @}

    //! @brief Bytes to be used for sizes.
    template <bool b>
    constexpr size_t size_sizeof = b ? FCPP_TRACE/8 : sizeof(size_t);

    //! @brief Serialization of iterable classes.
    //! @{
    template <typename T, bool b>
    isstream& iterable_serialize(isstream& s, T& x, std::integral_constant<bool, b>) {
        size_t size = 0;
        s.read(size, size_sizeof<b>);
        x.clear();
        for (size_t i = 0; i < size; ++i) {
            typename T::value_type v;
            s >> v;
            x.insert(x.end(), std::move(v));
        }
        return s;
    }

    template <typename T, bool b>
    osstream& iterable_serialize(osstream& s, T& x, std::integral_constant<bool, b>) {
        s.write(x.size(), size_sizeof<b>);
        for (auto& i : x) s << i;
        return s;
    }

    template <typename S, typename K>
    S& serialize(S& s, std::set<K>& x) {
        return iterable_serialize(s, x, std::is_same<K, trace_t>{});
    }

    template <typename S, typename K, typename V>
    S& serialize(S& s, std::map<K, V>& x) {
        return iterable_serialize(s, x, std::is_same<K, trace_t>{});
    }

    template <typename S, typename K>
    S& serialize(S& s, std::unordered_set<K>& x) {
        return iterable_serialize(s, x, std::is_same<K, trace_t>{});
    }

    template <typename S, typename K, typename V>
    S& serialize(S& s, std::unordered_map<K, V>& x) {
        return iterable_serialize(s, x, std::is_same<K, trace_t>{});
    }

    template <typename S, typename T>
    S& serialize(S& s, std::vector<T>& x) {
        return iterable_serialize(s, x, std::false_type{});
    }
    //! @}
}
//! @cond INTERNAL

//! @brief Serialisation from/to standard containers.
template <bool io, typename T>
inline std::enable_if_t<(not details::has_serialize<T>::value) and (not std::is_trivially_copyable<T>::value), sstream<io>&>
operator&(sstream<io>& is, T& x) {
    return details::serialize(is, x);
}


}


}

#endif // FCPP_COMMON_SERIALIZE_H_