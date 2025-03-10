/*******************************************************************************
 *   (c) 2018 - 2023 Zondax AG
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/
#ifndef IDL_VALUE_H
#define IDL_VALUE_H


#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__ 1234  // Use 1234 for Little Endian
#endif

#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__ 4321
#endif

#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 1234
#endif



#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "func.h"
#include "idl_value_utils.h"
#include "service.h"
#include "zondax_ic.h"

extern "C" {
#include "zondax_ic.h"
}

namespace std {
template <>
struct default_delete<IDLValue> {
  void operator()(IDLValue *ptr) const {
    if (ptr != nullptr) idl_value_destroy(ptr);
  }
};
}  // namespace std

namespace zondax {

/**
 * Enum class definition that is used to describe the type of the inner IDLValue
 * instance.
 */
enum class IdlValueType : int {
  Bool,
  Null,
  Text,
  Number,
  Float64,
  Opt,
  Vec,
  Record,
  Variant,
  Principal,
  Service,
  Func,
  None,
  Int,
  Nat,
  Nat8,
  Nat16,
  Nat32,
  Nat64,
  Int8,
  Int16,
  Int32,
  Int64,
  Float32,
  Reserved,
  Invalid  // place holder for invalid types
};

#define ENUM_TO_STRING_CASE(x) \
  case IdlValueType::x:        \
    return #x;

// Function to convert IdlValueType to string
constexpr std::string_view to_string(IdlValueType value) {
  switch (value) {
    ENUM_TO_STRING_CASE(Bool)
    ENUM_TO_STRING_CASE(Null)
    ENUM_TO_STRING_CASE(Text)
    ENUM_TO_STRING_CASE(Number)
    ENUM_TO_STRING_CASE(Float64)
    ENUM_TO_STRING_CASE(Opt)
    ENUM_TO_STRING_CASE(Vec)
    ENUM_TO_STRING_CASE(Record)
    ENUM_TO_STRING_CASE(Variant)
    ENUM_TO_STRING_CASE(Principal)
    ENUM_TO_STRING_CASE(Service)
    ENUM_TO_STRING_CASE(Func)
    ENUM_TO_STRING_CASE(None)
    ENUM_TO_STRING_CASE(Int)
    ENUM_TO_STRING_CASE(Nat)
    ENUM_TO_STRING_CASE(Nat8)
    ENUM_TO_STRING_CASE(Nat16)
    ENUM_TO_STRING_CASE(Nat32)
    ENUM_TO_STRING_CASE(Nat64)
    ENUM_TO_STRING_CASE(Int8)
    ENUM_TO_STRING_CASE(Int16)
    ENUM_TO_STRING_CASE(Int32)
    ENUM_TO_STRING_CASE(Int64)
    ENUM_TO_STRING_CASE(Float32)
    ENUM_TO_STRING_CASE(Reserved)
    default:
      return "Unknown type";
  }
}

inline std::ostream &operator<<(std::ostream &os, IdlValueType value) {
  os << to_string(value);
  return os;
}
}  // namespace zondax

namespace zondax {

#define IDL_VALUE_PRIMITIVES(name, type)     \
  template <>                                \
  inline IdlValue::IdlValue(type value) {    \
    ptr.reset(idl_value_with_##name(value)); \
  }

#define PRIMITIVE_TYPES_GETTER(name, type)                                 \
  template <>                                                              \
  inline std::optional<type> IdlValue::getImpl(helper::tag_type<type> t) { \
    if (ptr == nullptr) return std::nullopt;                               \
    type value;                                                            \
    bool ret = name##_from_idl_value(ptr.get(), &value);                   \
    if (!ret) return std::nullopt;                                         \
    return std::make_optional<type>(value);                                \
  }

namespace helper {
template <typename T>
struct is_candid_variant {
  template <typename C>
  static constexpr auto test(int)
      -> decltype(std::declval<C>().__CANDID_VARIANT_NAME,
                  std::declval<C>().__CANDID_VARIANT_CODE, std::true_type{});

  template <typename>
  static constexpr std::false_type test(...);

  using type = decltype(test<T>(0));
  static constexpr bool value = type::value;
};

template <typename T>
inline constexpr bool is_candid_variant_v = is_candid_variant<T>::value;

// Helper to specialize IdlValue::get() method
template <typename... Args>
struct tag_type {};

// Helper struct to check whether a type is a specialization of std::variant
template <typename T>
struct is_variant : std::false_type {};

template <typename... T>
struct is_variant<std::variant<T...>> : std::true_type {};

// Helper struct to check whether a type is a specialization of std::tuple
template <typename T>
struct is_tuple : std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename T>
struct is_vector : std::false_type {};

template <typename T>
struct is_vector<std::vector<T>> : std::true_type {};

// A helper to convert a tuple to a variant
template <typename T>
struct tuple_to_variant;
template <typename... Ts>
struct tuple_to_variant<std::tuple<Ts...>> {
  using type = std::variant<Ts...>;
};

// Get the variant's types as a tuple
template <typename V>
struct variant_to_tuple;
template <typename... Ts>
struct variant_to_tuple<std::variant<Ts...>> {
  using type = std::tuple<Ts...>;
};

template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type for_each(
    std::tuple<Tp...> &, FuncT)  // Unused arguments are given no names.
{}

template <std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if <
    I<sizeof...(Tp), void>::type for_each(std::tuple<Tp...> &t, FuncT f) {
  f(std::get<I>(t), std::integral_constant<size_t, I>());
  for_each<I + 1, FuncT, Tp...>(t, f);
}

template <typename T>
struct tuple_to_types;  // forward declare the template

template <typename... Ts>
struct tuple_to_types<std::tuple<Ts...>> {
  template <template <typename...> typename U>
  using rebind = U<Ts...>;
};

template <typename Tuple, template <typename...> typename U>
using tuple_to_types_t = typename tuple_to_types<Tuple>::template rebind<U>;

// ************** helper for vector<tuple<args...>>

template <typename T>
struct is_vector_of_tuples : std::false_type {};

template <typename... Args>
struct is_vector_of_tuples<std::vector<std::tuple<Args...>>> : std::true_type {
};

// Helper Trait definitions to handle std::optiona<T>
// specializations.
template <typename T>
inline constexpr bool is_vector_of_tuples_v = is_vector_of_tuples<T>::value;

template <typename T>
struct is_optional : std::false_type {};

template <typename U>
struct is_optional<std::optional<U>> : std::true_type {};

template <typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

template <typename T>
struct inner_type {};

template <typename T>
struct inner_type<std::optional<T>> {
  typedef T type;
};

template <typename T>
struct inner_type<std::vector<T>> {
  typedef T type;
};

}  // namespace helper

/**
 * Struct use to represent and IdlValue of type Number, which is used
 * to represent a Big number.
 */
struct Number {
  std::string value;
};

class IdlValue {
  friend class IdlArgs;

 private:
  template <typename T>
  struct callGetVariant;
  template <typename... Ts>
  struct callGetVariant<std::variant<Ts...>> {
    static std::optional<std::variant<Ts...>> call(IdlValue *val) {
      constexpr auto indices = std::index_sequence_for<Ts...>{};
      return val->getVariant<Ts...>(indices);
    }
  };

  std::unique_ptr<IDLValue> ptr;

  // Helper to initialize .ptr from an std::tuple-like set of items
  // used by IdlValue(std::tuple<Args...>) constructor
  template <typename Tuple, size_t... Indices>
  void initializeFromTuple(Tuple &tuple, std::index_sequence<Indices...>);

  /**
   * Generic method meant to be specialized by users that wants to support their
   * custom types, and equivalent specialization must exist for constructors.
   *
   * @treturns The value contained in this instance.
   *
   */
  template <typename T,
            std::enable_if_t<!helper::is_vector<T>::value, bool> = true>
  std::optional<T> getImpl(helper::tag_type<T> t) {
    return std::nullopt;
  }

  // specialization for vector of tuples
  template <typename... Args>
  std::optional<std::vector<std::tuple<Args...>>> getImpl(
      helper::tag_type<std::vector<std::tuple<Args...>>>) {
    auto values_vec = vec_from_idl_value(ptr.get());

    if (values_vec == nullptr) return std::nullopt;

    auto vec_len = cidlval_vec_len(values_vec);
    std::vector<std::tuple<Args...>> ret;

    for (int i = 0; i < vec_len; ++i) {
      auto p = cidlval_vec_value_take(values_vec, i);

      if (p == nullptr) return std::nullopt;

      IdlValue value(p);
      auto val = value.get<std::tuple<Args...>>();

      if (!val.has_value()) return std::nullopt;

      ret.emplace_back(std::move(val.value()));
    }

    cidlval_vec_destroy(values_vec);

    return ret;
  }

  template <typename Vec,
            std::enable_if_t<helper::is_vector<Vec>::value, bool> = true,
            std::enable_if_t<!helper::is_tuple<
                                 typename helper::inner_type<Vec>::type>::value,
                             bool> = true>
  std::optional<Vec> getImpl(helper::tag_type<Vec>) {
    using T = typename helper::inner_type<Vec>::type;

    auto values_vec = vec_from_idl_value(ptr.get());
    if (values_vec == nullptr) return std::nullopt;
    auto vec_len = cidlval_vec_len(values_vec);
    std::vector<T> ret;
    for (int i = 0; i < vec_len; ++i) {
      auto p = cidlval_vec_value_take(values_vec, i);
      if (p == nullptr) return std::nullopt;
      IdlValue value(p);
      std::optional<T> val = value.get<T>();
      if (!val.has_value()) return std::nullopt;
      ret.emplace_back(std::move(val.value()));
    }
    /*we took ownership of inner fields but still needs to destroy vector */
    cidlval_vec_destroy(values_vec);
    return std::make_optional<std::vector<T>>(std::move(ret));
  }

  // Fallback function for non-variant, non-tuples, non-map-like
  // types
  template <typename T>
  std::optional<T> getHelper(std::false_type, std::false_type) {
    return getImpl<T>(helper::tag_type<T>{});
  }

  // Specialization for variant
  template <typename T>
  std::optional<T> getHelper(std::true_type, std::false_type) {
    return callGetVariant<T>::call(this);
  }

  // Specialization for tuple
  template <typename T, typename Tuple, std::size_t... I>
  std::optional<T> getTupleHelper(Tuple &&, std::index_sequence<I...>) {
    return getTuple<std::tuple_element_t<I, T>...>();
  }

  // Specialization for tuple
  template <typename T>
  std::optional<T> getHelper(std::false_type, std::true_type) {
    auto optTuple = getTupleHelper<T>(
        T{}, std::make_index_sequence<std::tuple_size_v<T>>{});
    if (!optTuple.has_value()) return std::nullopt;
    return optTuple;
  }

  template <typename... Ts, std::size_t... Is>
  static std::optional<std::tuple<Ts...>> vectorToTupleImpl(
      std::vector<IdlValue> &vector, std::index_sequence<Is...>) {
    using variant_t = std::tuple<std::optional<Ts>...>;
    variant_t variantVector{vector[Is].get<Ts>()...};

    // all this because std::move(std::nullopt) is undefined behavior in C++
    if (!std::apply([](auto &&...opt) { return ((opt.has_value()) && ...); },
                    variantVector)) {
      return std::nullopt;
    }
    std::tuple<Ts...> tuple{std::move(*(std::get<Is>(variantVector)))...};

    return std::optional<std::tuple<Ts...>>{std::move(tuple)};
  }

  template <typename... Ts>
  std::optional<std::tuple<Ts...>> vectorToTuple(
      std::vector<IdlValue> &vector) {
    return std::move(
        vectorToTupleImpl<Ts...>(vector, std::index_sequence_for<Ts...>{}));
  }

  template <typename... Ts>
  std::optional<std::tuple<Ts...>> getTuple() {
    // tuples are in reality a list or records, we have an specialization for
    // it, so lets use that here;
    auto records = get<std::unordered_map<std::string, IdlValue>>();

    if (!records.has_value()) return std::nullopt;

    auto values = std::move(records.value());

    std::vector<int> keys;
    for (auto &pair : values) {
      keys.push_back(std::stoi(pair.first));
    }

    // Sort the keys, yes why? because std::sort does not support sorting
    // move-only types
    std::sort(keys.begin(), keys.end());

    // Create a vector of IdlValue objects in the order defined by keys
    std::vector<IdlValue> vecValues;

    for (int key : keys) {
      vecValues.push_back(std::move(values[std::to_string(key)]));
    }
    return std::move(vectorToTuple<Ts...>(vecValues));
  }

  // Attempt to extract the Indexth variant type from the CVariant
  template <typename Variant, std::size_t Index>
  bool tryVariant(std::optional<Variant> &loc, std::string_view key,
                  std::size_t code) {
    using T = std::variant_alternative_t<Index, Variant>;

    if (loc.has_value()) return false;

    // check code, key & Index match
    if (T::__CANDID_VARIANT_NAME != key || T::__CANDID_VARIANT_CODE != code)
      return false;

    // get to obtain the inner type
    auto v = get<T>();
    if (!v.has_value()) return false;

    // initialize loc with the proper type
    loc.emplace(std::in_place_index<Index>, std::move(v.value()));
    return true;
  }

  template <typename... Ts, std::size_t... Indices>
  std::optional<std::variant<Ts...>> getVariant(
      std::index_sequence<Indices...>) {
    using Variant = std::variant<Ts...>;

    auto cvariant_data = asCVariant();
    if (!cvariant_data.has_value()) return std::nullopt;

    auto [key, code, variant] = std::move(cvariant_data.value());

    std::optional<Variant> result;

    (variant.tryVariant<Variant, Indices>(result, key, code), ...);

    return result;
  }

 public:
  // Disable copies, just move semantics
  IdlValue(const IdlValue &args) = delete;
  void operator=(const IdlValue &) = delete;

  // declare move constructor & assignment
  IdlValue(IdlValue &&o) noexcept;
  IdlValue &operator=(IdlValue &&o) noexcept;

  /******************** Constructors ***********************/

  /**
   * Default constructor for IdlValues of type Opt or None
   *
   * @tparam A generic type T wrapped into a std::optional
   *
   */
  template <typename T>
  explicit IdlValue(T);
  template <typename T,
            typename = std::enable_if_t<std::is_constructible_v<IdlValue, T>>>
  explicit IdlValue(std::optional<T>);

  /**
   * Default constructor for a collection of values of type T. Any type T is
   * supported as long as a constructor specialization exist for it.
   *
   * @tparam A move only vector of types T
   *
   */
  template <typename T,
            typename = std::enable_if_t<std::is_constructible_v<IdlValue, T>>>
  explicit IdlValue(std::vector<T> &&);

  /**
   * Default constructor for a collection of tuples. Any tuple is
   * supported as long as a constructor specialization exist for each element
   * it contains.
   *
   * @tparam A move only vector of tuple.
   *
   */
  template <typename... Args,
            typename = std::enable_if_t<
                (std::is_constructible_v<IdlValue, Args> && ...)>>
  explicit IdlValue(std::vector<std::tuple<Args...>> &&elems);

  /**
   * Default constructor for tuples. Any tuple is
   * supported as long as a constructor specialization exist for each element
   * it contains.
   *
   * @tparam A move only tuple.
   *
   */
  template <typename... Args,
            typename = std::enable_if_t<
                (std::is_constructible_v<IdlValue, Args> && ...)>>
  explicit IdlValue(std::tuple<Args...> &&);

  /**
   * Default constructor for std::variant. Any variant alternative type is
   * supported as long as a constructor specialization exist for each.
   *
   * @tparam std::variant<Args...>
   *
   */
  template <
      typename... Args,
      typename = std::enable_if_t<(helper::is_candid_variant_v<Args> && ...)>,
      typename =
          std::enable_if_t<(std::is_constructible_v<IdlValue, Args> && ...)>>
  explicit IdlValue(std::variant<Args...>);

  /**
   * Default constructor. Constructs an Null idlValue instance
   *
   */
  explicit IdlValue() : ptr(nullptr) {}

  /**
   * Static method to create an NULL idlValue instance
   */
  static IdlValue null();
  /**
   * Static method to create an IdlValue::Reserve instance
   */
  static IdlValue reserved();

  /**
   * static constructor for to create an idlValue with a list of records.
   *
   * @tparam An unordered_map containing a list of records.
   *
   */
  static IdlValue FromRecord(std::unordered_map<std::string, IdlValue> &fields);

  /**
   * static constructor for to create an idlValue of type Variant.
   *
   * @tparam key
   * @tparam IdlValue pointer containing the inner value.
   * @tparam code is the index at which this values is in a collection
   *
   */
  static IdlValue FromVariant(std::string key, IdlValue *val, uint64_t code);

  /******************** Getters ***********************/

  /**
   * Generic Getter to obtain the inner values this IdlValue instance
   * holds. This method is overloaded to support common C++ types.
   *
   * @treturns The value contained in this instance.
   *
   */
  template <typename T,
            typename = std::enable_if_t<!helper::is_vector_of_tuples_v<T> &&
                                        !helper::is_optional_v<T>>>
  std::optional<T> get() {
    return getHelper<T>(helper::is_variant<T>{}, helper::is_tuple<T>{});
  }

  /**
   * Generic Getter specialized to get a std::optional<T>, meant to be used by
   * IdlValue instances of type IDLValue::Opt or  IDLValue::None
   * holds. This method is overloaded to support common C++ types.
   *
   * @treturns The value contained in this instance.
   *
   */
  template <typename T, std::enable_if_t<helper::is_optional_v<T>, bool> = true>
  std::optional<T> get() {
    // get inner type:
    using U = typename helper::inner_type<T>::type;

    // use the type as inner value can be either Opt or None
    switch (type()) {
      case IdlValueType::Opt: {
        auto opt = getOpt();
        if (!opt.has_value()) return std::nullopt;

        auto value = std::move(opt.value());

        return value.get<U>();
      }
      case IdlValueType::None:
        return T{std::nullopt};

      default:
        return std::nullopt;
    }
  }

  /**
   * Generic Getter specialized to get values of type
   * std::map/std::unordered_map. This method relays on the corresponding getter
   * specialization for types U and V.
   *
   * @treturns The value contained in this instance.
   *
   */
  template <
      template <typename, typename> class T, typename U, typename V,
      std::enable_if_t<
          std::is_invocable_v<decltype(&IdlValue::get<U>), IdlValue &>, U>,
      std::enable_if_t<
          std::is_invocable_v<decltype(&IdlValue::get<V>), IdlValue &>, V>,
      std::enable_if_t<!std::is_same_v<T<U, V>, std::variant<U, V>>>>
  T<U, V> get() {
    return getImpl(helper::tag_type<T<U, V>>{});
  }

  template <template <typename, typename> class T, typename U, typename V>
  std::optional<T<U, V>> get(helper::tag_type<T<U, V>>) {
    return std::nullopt;
  }

  /**
   * Generic Getter specialized to get a vector of tuples.
   * This method relays on the corresponding getter
   * specialization for types contained in the std::tuple<Args...>
   *
   * @treturns The value contained in this instance.
   *
   */
  template <typename T,
            std::enable_if_t<helper::is_vector_of_tuples_v<T>, bool> = true>
  std::optional<T> get() {
    return getImpl(helper::tag_type<T>{});
  }

  /**
   * Generic Getter specialized to get an unordered_map or records.
   *
   * @treturns The value contained in this instance.
   *
   */
  std::unordered_map<std::string, IdlValue> getRecord();

  /**
   * Getter to get an IdlValue of type Opt or None.
   *
   * @treturns The value contained in this instance.
   *
   */
  std::optional<IdlValue> getOpt();

  /**
   * Getter to get the inner pointer to a raw IDLValue instance.
   * this methods transfer ownership to the caller.
   *
   * @treturns std::unique_ptr<IDLValue> instance.
   *
   */
  std::unique_ptr<IDLValue> getPtr();

  std::optional<std::tuple<std::string, std::size_t, IdlValue>> asCVariant();

  /**
   * Method to get the inner type of this IdlValue instance.
   *
   * @treturns the IdlValueType instance indicating the IDLValue type.
   *
   */
  IdlValueType type();
};

/******************** Private ***********************/

template <typename Tuple, size_t... Indices>
void IdlValue::initializeFromTuple(Tuple &tuple,
                                   std::index_sequence<Indices...>) {
  std::vector<IDLValue *> values;
  std::vector<std::array<uint8_t, 4>> indices;

  auto func = [&](auto &&tp, auto index) {
    IdlValue val(std::move(tp));
    values.push_back(val.ptr.release());

    // Array to store the resulting bytes
    std::array<uint8_t, 4> bytes;
    auto value = static_cast<uint32_t>(index);

    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;

// If the platform is big endian, swap the byte order
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    std::swap(bytes[0], bytes[3]);
    std::swap(bytes[1], bytes[2]);
#endif
    indices.push_back(bytes);
  };
  helper::for_each(tuple, func);

  std::vector<const char *> cstr_indices(indices.size());
  for (size_t i = 0; i < indices.size(); ++i) {
    cstr_indices[i] = reinterpret_cast<const char *>(indices[i].data());
  }

  ptr.reset(idl_value_with_record(cstr_indices.data(), indices.size(),
                                  values.data(), values.size(), true));
}

IDL_VALUE_PRIMITIVES(nat8, uint8_t)
IDL_VALUE_PRIMITIVES(nat16, uint16_t)
IDL_VALUE_PRIMITIVES(nat32, uint32_t)
IDL_VALUE_PRIMITIVES(nat64, uint64_t)
IDL_VALUE_PRIMITIVES(int8, int8_t)
IDL_VALUE_PRIMITIVES(int16, int16_t)
IDL_VALUE_PRIMITIVES(int32, int32_t)
IDL_VALUE_PRIMITIVES(int64, int64_t)
IDL_VALUE_PRIMITIVES(float32, float)
IDL_VALUE_PRIMITIVES(float64, double)
IDL_VALUE_PRIMITIVES(bool, bool)

template <>
inline IdlValue::IdlValue(std::string text) {
  // TODO: Use RetError
  ptr.reset(idl_value_with_text(text.c_str(), nullptr));
}

template <>
inline IdlValue::IdlValue(Number number) {
  // TODO: Use RetError
  // RetPtr_u8 error;

  ptr.reset(idl_value_with_number(number.value.c_str(), nullptr));
}

template <typename T, typename>
inline IdlValue::IdlValue(std::optional<T> val) {
  if (val.has_value()) {
    // use move-constructor
    IdlValue value(std::move(val.value()));
    ptr.reset(idl_value_with_opt(value.ptr.release()));
  } else {
    ptr.reset(idl_value_with_none());
  }
}

template <typename T, typename>
inline IdlValue::IdlValue(std::vector<T> &&elems) {
  std::vector<const IDLValue *> cElems;
  cElems.reserve(elems.size());

  for (auto &&e : elems) {
    IdlValue val(std::move(e));
    cElems.push_back(val.ptr.release());
  }

  ptr.reset(idl_value_with_vec(cElems.data(), cElems.size()));
}

template <typename... Args, typename>
inline IdlValue::IdlValue(std::vector<std::tuple<Args...>> &&elems) {
  std::vector<const IDLValue *> cElems;
  cElems.reserve(elems.size());

  for (auto &&e : elems) {
    IdlValue val(std::move(e));
    cElems.push_back(val.ptr.release());
  }

  ptr.reset(idl_value_with_vec(cElems.data(), cElems.size()));
}

template <typename... Args, typename>
inline IdlValue::IdlValue(std::tuple<Args...> &&tuple) {
  initializeFromTuple(tuple, std::index_sequence_for<Args...>());
}

template <typename... Args, typename, typename>
inline IdlValue::IdlValue(std::variant<Args...> variant) {
  std::visit(
      [this](auto &value) {
        IdlValue val(std::move(value));
        *this = IdlValue::FromVariant(std::string(value.__CANDID_VARIANT_NAME),
                                      &val, value.__CANDID_VARIANT_CODE);
      },
      variant);
}

template <>
inline IdlValue::IdlValue(IDLValue *ptr) : ptr(ptr){};

template <>
inline IdlValue::IdlValue(const IDLValue *ptr) : ptr((IDLValue *)ptr){};

template <>
inline IdlValue::IdlValue(zondax::Principal principal) {
  // TODO: Use RetError
  // RetPtr_u8 error;

  ptr.reset(idl_value_with_principal(principal.getBytes().data(),
                                     principal.getBytes().size(), nullptr));
}

template <>
inline IdlValue::IdlValue(zondax::Service service) {
  // TODO: Use RetError
  // RetPtr_u8 error;

  auto principal = service.principal_take();
  ptr.reset(idl_value_with_service(principal.getBytes().data(),
                                   principal.getBytes().size(), nullptr));
}

template <>
inline IdlValue::IdlValue(zondax::Func func) {
  auto princ = func.principal().getBytes();

  ptr.reset(idl_value_with_func(princ.data(), princ.size(),
                                func.method_name().c_str()));
}

template <>
inline IdlValue::IdlValue(std::monostate) {
  ptr.reset(idl_value_with_null());
}

inline IdlValue IdlValue::null(void) {
  IdlValue val;
  val.ptr.reset(idl_value_with_null());
  return val;
}

inline IdlValue IdlValue::reserved(void) {
  IdlValue val;
  val.ptr.reset(idl_value_with_reserved());
  return val;
}

PRIMITIVE_TYPES_GETTER(int8, int8_t)
PRIMITIVE_TYPES_GETTER(int16, int16_t)
PRIMITIVE_TYPES_GETTER(int32, int32_t)
PRIMITIVE_TYPES_GETTER(int64, int64_t)
PRIMITIVE_TYPES_GETTER(nat8, uint8_t)
PRIMITIVE_TYPES_GETTER(nat16, uint16_t)
PRIMITIVE_TYPES_GETTER(nat32, uint32_t)
PRIMITIVE_TYPES_GETTER(nat64, uint64_t)
PRIMITIVE_TYPES_GETTER(float32, float)
PRIMITIVE_TYPES_GETTER(float64, double)
PRIMITIVE_TYPES_GETTER(bool, bool)

template <>
inline std::optional<std::string> IdlValue::getImpl(
    helper::tag_type<std::string> type) {
  if (ptr == nullptr) return std::nullopt;

  CText *ctext = text_from_idl_value(ptr.get());
  if (ctext == nullptr) return std::nullopt;

  const char *str = ctext_str(ctext);
  if (str == nullptr) return std::nullopt;

  // use c null-terminated string constructor
  std::string s(str);

  ctext_destroy(ctext);

  return std::make_optional<std::string>(s);
}

template <>
inline std::optional<zondax::Principal> IdlValue::getImpl(
    helper::tag_type<zondax::Principal> type) {
  if (ptr == nullptr) return std::nullopt;

  CPrincipal *p = principal_from_idl_value(ptr.get());
  if (p == nullptr) return std::nullopt;

  std::vector<unsigned char> bytes(p->ptr, p->ptr + p->len);
  zondax::Principal principal(bytes);

  principal_destroy(p);

  return std::make_optional(std::move(principal));
}

template <>
inline std::optional<Number> IdlValue::getImpl(helper::tag_type<Number> type) {
  if (ptr == nullptr) return std::nullopt;

  CText *ctext = number_from_idl_value(ptr.get());
  if (ctext == nullptr) return std::nullopt;

  const char *str = ctext_str(ctext);

  std::string text(str);

  ctext_destroy(ctext);
  Number number;
  number.value = text;

  return std::make_optional(number);
}

template <>
inline std::optional<zondax::Func> IdlValue::getImpl(
    helper::tag_type<Func> type) {
  if (ptr == nullptr) return std::nullopt;

  struct CFunc *cFunc = func_from_idl_value(ptr.get());
  if (cFunc == nullptr) return std::nullopt;

  // Extract principal
  struct CPrincipal *cPrincipal = cfunc_principal(cFunc);
  std::vector<uint8_t> vec =
      std::vector<uint8_t>(cPrincipal->ptr, cPrincipal->ptr + cPrincipal->len);

  zondax::Func result(
      zondax::Principal(vec),
      std::string(cfunc_string(cFunc),
                  cfunc_string(cFunc) + cfunc_string_len(cFunc)));

  // Free the allocated CFunc
  cfunc_destroy(cFunc);

  return std::optional<zondax::Func>(std::move(result));
}

template <>
inline std::optional<std::vector<IdlValue>> IdlValue::getImpl(
    helper::tag_type<std::vector<IdlValue>> type) {
  if (ptr == nullptr) return std::nullopt;

  // call bellow creates a new IDLValue::Vec,
  // this means it "clones" from this.ptr
  struct CIDLValuesVec *cVec = vec_from_idl_value(ptr.get());
  if (cVec == nullptr) return std::nullopt;

  uintptr_t length = cidlval_vec_len(cVec);

  std::vector<IdlValue> result;
  result.reserve(length);

  // then it should take out each value from the vector
  // and give ownership to the caller.
  for (uintptr_t i = 0; i < length; ++i) {
    // the take variant leave the value in the vector in a null state
    // this is valid as inner value belongs now to result
    const IDLValue *valuePtr = cidlval_vec_value_take(cVec, i);
    result.emplace_back(valuePtr);
  }

  // Free the allocated CIDLValuesVec
  cidlval_vec_destroy(cVec);

  return std::make_optional(std::move(result));
}

template <>
inline std::optional<zondax::Service> IdlValue::getImpl(
    helper::tag_type<zondax::Service> type) {
  if (ptr == nullptr) return std::nullopt;

  CPrincipal *p = service_from_idl_value(ptr.get());

  if (p == nullptr) return std::nullopt;

  std::vector<unsigned char> bytes(p->ptr, p->ptr + p->len);
  zondax::Principal principal(bytes);

  // there is not issue in destroying raw principal
  // because bytes makes a deep copy
  principal_destroy(p);

  return std::make_optional(zondax::Service(std::move(principal)));
}

template <>
inline std::optional<std::monostate> IdlValue::getImpl(
    helper::tag_type<std::monostate> type) {
  if (ptr == nullptr) return std::nullopt;

  return idl_value_is_null(ptr.get()) ? std::make_optional<std::monostate>()
                                      : std::nullopt;
}

// Record specialization
// TODO: We need to add testing for this!!!!!
template <>
inline std::optional<std::unordered_map<std::string, IdlValue>>
IdlValue::getImpl<std::unordered_map<std::string, IdlValue>>(
    helper::tag_type<std::unordered_map<std::string, IdlValue>> t) {
  if (ptr.get() == nullptr) return std::nullopt;
  auto record = record_from_idl_value(ptr.get());
  if (record == nullptr) return std::nullopt;

  std::unordered_map<std::string, IdlValue> fields;

  auto len = crecord_keys_len(record);

  for (uintptr_t i = 0; i < len; ++i) {
    auto cKey = crecord_take_key(record, i);
    auto cVal = crecord_take_val(record, i);

    if (cKey == nullptr || cVal == nullptr) return std::nullopt;

    auto str = ctext_str(cKey);
    if (str == nullptr) return std::nullopt;

    auto key = std::string("");
    if (str != nullptr) {
      key = std::string(str);
    }

    IdlValue val(cVal);

    fields.emplace(std::make_pair(key, std::move(val)));
  }

  crecord_destroy(record);

  return std::make_optional(std::move(fields));
}

}  // namespace zondax

#endif
