#ifndef FLOW_BACKPORTS_H
#define FLOW_BACKPORTS_H

#if __cplusplus < 201402L && !defined(_MSC_VER)

#include <cstddef> // for std::size_t
#include <memory>
namespace std {

template< std::size_t ... i >
struct index_sequence {
  typedef std::size_t value_type;
  typedef index_sequence<i...> type;
  // gcc-4.4.7 doesn't support `constexpr` and `noexcept`.
  static /*constexpr*/ std::size_t size() /*noexcept*/ {
    return sizeof ... (i);
  }
};

// this structure doubles index_sequence elements.
// s- is number of template arguments in IS.
template< std::size_t s, typename IS >
struct doubled_index_sequence;

template< std::size_t s, std::size_t ... i >
struct doubled_index_sequence<s, index_sequence<i... >> {
  typedef index_sequence<i..., (s + i)... > type;
};

// this structure incremented by one index_sequence, iff NEED-is true,
// otherwise returns IS
template< bool NEED, typename IS >
struct inc_index_sequence;

template< typename IS >
struct inc_index_sequence<false,IS>{ typedef IS type; };

template< std::size_t ... i >
struct inc_index_sequence< true, index_sequence<i...> > {
  typedef index_sequence<i..., sizeof...(i)> type;
};

// helper structure for make_index_sequence.
template< std::size_t N >
struct make_index_sequence_impl :
    inc_index_sequence< (N % 2 != 0),
    typename doubled_index_sequence< N / 2,
    typename make_index_sequence_impl< N / 2> ::type>::type> {};

// helper structure needs specialization only with 0 element.
template<>struct make_index_sequence_impl<0>{ typedef index_sequence<> type; };

// OUR make_index_sequence,  gcc-4.4.7 doesn't support `using`,
// so we use struct instead of it.
template< std::size_t N >
struct make_index_sequence : make_index_sequence_impl<N>::type {};

//index_sequence_for  any variadic templates
template< typename ... T >
struct index_sequence_for : make_index_sequence< sizeof...(T) >{};

//make_unique
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

#endif

#endif