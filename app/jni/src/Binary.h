#pragma once
#include <fstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <type_traits>
#include <cstring>

#pragma warning(push)
#pragma warning(disable : 4267)


// Define traits to replace concepts
template <typename T>
struct is_trivial : std::is_trivial<T> {};

template <typename T>
struct is_trivial_pair {
    static const bool value = is_trivial<typename T::first_type>::value &&
                              is_trivial<typename T::second_type>::value;
};

template <typename T>
struct is_binary_data {
    static const bool value = is_trivial<T>::value;
};

template <typename T>
struct is_binary_class {
private:
    template <typename U>
    static auto test(int) -> decltype(std::declval<U>().Write(std::declval<std::ostream&>()), std::declval<U>().Read(std::declval<std::istream&>()), std::true_type());

    template <typename>
    static std::false_type test(...);

public:
    static const bool value = decltype(test<T>(0))::value;
};

template <typename T>
using ContainerChild = typename std::remove_cv<typename std::remove_reference<decltype(*std::declval<T>().cbegin())>::type>::type;

template <typename T>
struct is_binary_vector {
private:
    template <typename U>
    static auto test(int) -> decltype(
    std::declval<U>().size(),
            std::declval<U>().reserve(std::declval<size_t>()),
            std::declval<U>().cbegin(),
            std::declval<U>().cend(),
            std::declval<U>().push_back(std::declval<typename U::value_type>()),
            std::true_type());

    template <typename>
    static std::false_type test(...);

public:
    static const bool value = decltype(test<T>(0))::value;
};

template <typename T>
struct is_binary_map {
private:
    template <typename U>
    static auto test(int) -> decltype(
    typename U::key_type(),
            typename U::mapped_type(),
            typename U::value_type(),
            std::declval<U>().cbegin(),
            std::declval<U>().cend(),
            std::true_type());

    template <typename>
    static std::false_type test(...);

public:
    static const bool value = decltype(test<T>(0))::value;
};

// #define _BIN_DBG

/// <summary>
/// 提供结构化的二进制与stl的转换
/// </summary>
class Binary {
public:
    template <typename T>
    static typename std::enable_if<is_binary_data<T>::value>::type Write(std::ostream& stm, const T& dat) {
        stm.write((char*)&dat, sizeof(dat));
#ifdef _BIN_DBG
        stm.flush();
#endif //  _BIN_DBG
    }

    template <typename T>
    static typename std::enable_if<is_binary_data<T>::value>::type Read(std::istream& stm, T& dat) {
        stm.read((char*)&dat, sizeof(dat));
    }

    template <typename T>
    static typename std::enable_if<is_binary_class<T>::value>::type Write(std::ostream& stm, const T& cls) {
        cls.Write(stm);
    }

    template <typename T>
    static typename std::enable_if<is_binary_class<T>::value>::type Read(std::istream& stm, T& cls) {
        cls.Read(stm);
    }

    template <typename T>
    static typename std::enable_if<is_binary_vector<T>::value>::type Read(std::istream& stm, T& vec) {
        typedef ContainerChild<T> ContainerChildType;
        unsigned long long size = 0;
        Read(stm, size);
        vec.reserve(size);
        for (size_t i = 0; i < size; i++) {
            if (stm.eof())
                return;
            ContainerChildType data{};
            Read(stm, data);
            vec.push_back(data);
        }
    }

    template <typename T>
    static typename std::enable_if<is_binary_vector<T>::value>::type Write(std::ostream& stm, const T& vec) {
        unsigned long long sz = vec.size();
        Write(stm, sz);
        for (const auto& data : vec) {
            Write(stm, data);
            sz--;
        }
    }

    template <typename T>
    static typename std::enable_if<is_binary_map<T>::value>::type Read(std::istream& stm, T& map) {
        typedef ContainerChild<T> ContainerChildType;
        unsigned long long size = 0;
        Read(stm, size);
        for (size_t i = 0; i < size; i++) {
            if (stm.eof())
                return;
            typename std::remove_cv<typename ContainerChildType::first_type>::type key{};
            Read(stm, key);
            typename std::remove_cv<typename ContainerChildType::second_type>::type val{};
            Read(stm, val);
            map[key] = val;
        }
    }

    template <typename T>
    static typename std::enable_if<is_binary_map<T>::value>::type Write(std::ostream& stm, const T& map) {
        unsigned long long sz = map.size();
        Write(stm, sz);
        for (const auto& kv : map) {
            Write(stm, kv.first);
            Write(stm, kv.second);
            sz--;
        }
    }
};

#pragma warning(pop)
