#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace drift::json {

class Value {
public:
    using array_type = std::vector<Value>;
    using object_type = std::map<std::string, Value>;
    using storage_type = std::variant<std::nullptr_t, bool, std::int64_t, std::string, array_type, object_type>;

    Value();
    Value(std::nullptr_t);
    Value(bool value);
    Value(std::int64_t value);
    Value(int value);
    Value(std::string value);
    Value(const char* value);
    Value(array_type value);
    Value(object_type value);

    [[nodiscard]] bool is_null() const noexcept;
    [[nodiscard]] bool is_bool() const noexcept;
    [[nodiscard]] bool is_number() const noexcept;
    [[nodiscard]] bool is_string() const noexcept;
    [[nodiscard]] bool is_array() const noexcept;
    [[nodiscard]] bool is_object() const noexcept;

    [[nodiscard]] bool as_bool() const;
    [[nodiscard]] std::int64_t as_i64() const;
    [[nodiscard]] const std::string& as_string() const;
    [[nodiscard]] const array_type& as_array() const;
    [[nodiscard]] const object_type& as_object() const;
    [[nodiscard]] array_type& as_array();
    [[nodiscard]] object_type& as_object();

    [[nodiscard]] bool contains(std::string_view key) const;
    [[nodiscard]] const Value& at(std::string_view key) const;
    [[nodiscard]] const Value* find(std::string_view key) const noexcept;

    void push(Value value);
    void set(std::string key, Value value);

private:
    storage_type data_;
};

Value parse(std::string_view source);
Value parse_file(const std::string& path);
std::string stringify(const Value& value, bool pretty = false);
std::string escape(std::string_view input);

std::string require_string(const Value& object, std::string_view key);
std::string optional_string(const Value& object, std::string_view key, std::string fallback);
std::int64_t require_i64(const Value& object, std::string_view key);
std::int64_t optional_i64(const Value& object, std::string_view key, std::int64_t fallback);
bool optional_bool(const Value& object, std::string_view key, bool fallback);
const Value::array_type& require_array(const Value& object, std::string_view key);
const Value::object_type& require_object(const Value& object, std::string_view key);

} // namespace drift::json

