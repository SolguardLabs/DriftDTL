#include "core/json.hpp"

#include "core/error.hpp"

#include <charconv>
#include <cctype>
#include <fstream>
#include <sstream>

namespace drift::json {
namespace {

class Parser {
public:
    explicit Parser(std::string_view source)
        : source_(source)
    {
    }

    Value parse_root()
    {
        skip_ws();
        Value value = parse_value();
        skip_ws();
        if (!eof()) {
            fail_at("unexpected trailing JSON input");
        }
        return value;
    }

private:
    [[nodiscard]] bool eof() const noexcept
    {
        return offset_ >= source_.size();
    }

    [[nodiscard]] char peek() const
    {
        if (eof()) {
            fail_at("unexpected end of JSON input");
        }
        return source_[offset_];
    }

    char take()
    {
        char ch = peek();
        ++offset_;
        return ch;
    }

    bool take_if(char expected)
    {
        if (!eof() && source_[offset_] == expected) {
            ++offset_;
            return true;
        }
        return false;
    }

    void skip_ws()
    {
        while (!eof() && std::isspace(static_cast<unsigned char>(source_[offset_]))) {
            ++offset_;
        }
    }

    Value parse_value()
    {
        skip_ws();
        char ch = peek();
        if (ch == '"') {
            return Value(parse_string());
        }
        if (ch == '{') {
            return Value(parse_object());
        }
        if (ch == '[') {
            return Value(parse_array());
        }
        if (ch == 't') {
            expect_literal("true");
            return Value(true);
        }
        if (ch == 'f') {
            expect_literal("false");
            return Value(false);
        }
        if (ch == 'n') {
            expect_literal("null");
            return Value(nullptr);
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) {
            return Value(parse_number());
        }
        fail_at("unexpected JSON token");
    }

    std::string parse_string()
    {
        if (!take_if('"')) {
            fail_at("expected string");
        }

        std::string out;
        while (!eof()) {
            char ch = take();
            if (ch == '"') {
                return out;
            }
            if (static_cast<unsigned char>(ch) < 0x20) {
                fail_at("unescaped control character in string");
            }
            if (ch != '\\') {
                out.push_back(ch);
                continue;
            }

            char escaped = take();
            switch (escaped) {
            case '"':
                out.push_back('"');
                break;
            case '\\':
                out.push_back('\\');
                break;
            case '/':
                out.push_back('/');
                break;
            case 'b':
                out.push_back('\b');
                break;
            case 'f':
                out.push_back('\f');
                break;
            case 'n':
                out.push_back('\n');
                break;
            case 'r':
                out.push_back('\r');
                break;
            case 't':
                out.push_back('\t');
                break;
            case 'u':
                append_unicode_escape(out);
                break;
            default:
                fail_at("invalid escape sequence");
            }
        }

        fail_at("unterminated string");
    }

    void append_unicode_escape(std::string& out)
    {
        if (offset_ + 4 > source_.size()) {
            fail_at("short unicode escape");
        }

        unsigned code = 0;
        for (int i = 0; i < 4; ++i) {
            char ch = source_[offset_++];
            code <<= 4;
            if (ch >= '0' && ch <= '9') {
                code += static_cast<unsigned>(ch - '0');
            } else if (ch >= 'a' && ch <= 'f') {
                code += static_cast<unsigned>(ch - 'a' + 10);
            } else if (ch >= 'A' && ch <= 'F') {
                code += static_cast<unsigned>(ch - 'A' + 10);
            } else {
                fail_at("invalid unicode escape");
            }
        }

        if (code <= 0x7F) {
            out.push_back(static_cast<char>(code));
        } else if (code <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | (code >> 6)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (code >> 12)));
            out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        }
    }

    std::int64_t parse_number()
    {
        std::size_t start = offset_;
        if (take_if('-') && eof()) {
            fail_at("invalid number");
        }

        if (take_if('0')) {
            if (!eof() && std::isdigit(static_cast<unsigned char>(source_[offset_]))) {
                fail_at("leading zero in number");
            }
        } else {
            if (eof() || !std::isdigit(static_cast<unsigned char>(source_[offset_]))) {
                fail_at("invalid number");
            }
            while (!eof() && std::isdigit(static_cast<unsigned char>(source_[offset_]))) {
                ++offset_;
            }
        }

        if (!eof() && (source_[offset_] == '.' || source_[offset_] == 'e' || source_[offset_] == 'E')) {
            fail_at("fractional numbers are not accepted");
        }

        std::int64_t value = 0;
        auto view = source_.substr(start, offset_ - start);
        auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value);
        if (ec != std::errc{} || ptr != view.data() + view.size()) {
            fail_at("integer outside supported range");
        }
        return value;
    }

    Value::array_type parse_array()
    {
        if (!take_if('[')) {
            fail_at("expected array");
        }

        Value::array_type out;
        skip_ws();
        if (take_if(']')) {
            return out;
        }

        while (true) {
            out.push_back(parse_value());
            skip_ws();
            if (take_if(']')) {
                return out;
            }
            if (!take_if(',')) {
                fail_at("expected comma or array end");
            }
        }
    }

    Value::object_type parse_object()
    {
        if (!take_if('{')) {
            fail_at("expected object");
        }

        Value::object_type out;
        skip_ws();
        if (take_if('}')) {
            return out;
        }

        while (true) {
            skip_ws();
            if (peek() != '"') {
                fail_at("expected object key");
            }
            std::string key = parse_string();
            skip_ws();
            if (!take_if(':')) {
                fail_at("expected colon after object key");
            }
            Value value = parse_value();
            auto insert_result = out.emplace(std::move(key), std::move(value));
            if (!insert_result.second) {
                fail_at("duplicate object key");
            }
            skip_ws();
            if (take_if('}')) {
                return out;
            }
            if (!take_if(',')) {
                fail_at("expected comma or object end");
            }
        }
    }

    void expect_literal(std::string_view literal)
    {
        if (source_.substr(offset_, literal.size()) != literal) {
            fail_at("invalid literal");
        }
        offset_ += literal.size();
    }

    [[noreturn]] void fail_at(std::string message) const
    {
        message += " at byte ";
        message += std::to_string(offset_);
        throw DriftError(ErrorCode::InvalidJson, std::move(message));
    }

    std::string_view source_;
    std::size_t offset_ = 0;
};

class Writer {
public:
    explicit Writer(bool pretty)
        : pretty_(pretty)
    {
    }

    std::string write(const Value& value)
    {
        out_.clear();
        write_value(value, 0);
        if (pretty_) {
            out_.push_back('\n');
        }
        return out_;
    }

private:
    void write_value(const Value& value, int depth)
    {
        if (value.is_null()) {
            out_ += "null";
        } else if (value.is_bool()) {
            out_ += value.as_bool() ? "true" : "false";
        } else if (value.is_number()) {
            out_ += std::to_string(value.as_i64());
        } else if (value.is_string()) {
            out_.push_back('"');
            out_ += escape(value.as_string());
            out_.push_back('"');
        } else if (value.is_array()) {
            write_array(value.as_array(), depth);
        } else {
            write_object(value.as_object(), depth);
        }
    }

    void write_array(const Value::array_type& values, int depth)
    {
        out_.push_back('[');
        if (!values.empty()) {
            newline(depth + 1);
            for (std::size_t i = 0; i < values.size(); ++i) {
                indent(depth + 1);
                write_value(values[i], depth + 1);
                if (i + 1 < values.size()) {
                    out_.push_back(',');
                }
                newline(depth + 1);
            }
            indent(depth);
        }
        out_.push_back(']');
    }

    void write_object(const Value::object_type& values, int depth)
    {
        out_.push_back('{');
        if (!values.empty()) {
            newline(depth + 1);
            std::size_t index = 0;
            for (const auto& [key, value] : values) {
                indent(depth + 1);
                out_.push_back('"');
                out_ += escape(key);
                out_.push_back('"');
                out_ += pretty_ ? ": " : ":";
                write_value(value, depth + 1);
                if (++index < values.size()) {
                    out_.push_back(',');
                }
                newline(depth + 1);
            }
            indent(depth);
        }
        out_.push_back('}');
    }

    void newline(int)
    {
        if (pretty_) {
            out_.push_back('\n');
        }
    }

    void indent(int depth)
    {
        if (!pretty_) {
            return;
        }
        for (int i = 0; i < depth; ++i) {
            out_ += "  ";
        }
    }

    bool pretty_;
    std::string out_;
};

[[noreturn]] void type_error(std::string_view expected)
{
    throw DriftError(ErrorCode::InvalidJson, "expected JSON " + std::string(expected));
}

} // namespace

Value::Value()
    : data_(nullptr)
{
}

Value::Value(std::nullptr_t)
    : data_(nullptr)
{
}

Value::Value(bool value)
    : data_(value)
{
}

Value::Value(std::int64_t value)
    : data_(value)
{
}

Value::Value(int value)
    : data_(static_cast<std::int64_t>(value))
{
}

Value::Value(std::string value)
    : data_(std::move(value))
{
}

Value::Value(const char* value)
    : data_(std::string(value))
{
}

Value::Value(array_type value)
    : data_(std::move(value))
{
}

Value::Value(object_type value)
    : data_(std::move(value))
{
}

bool Value::is_null() const noexcept
{
    return std::holds_alternative<std::nullptr_t>(data_);
}

bool Value::is_bool() const noexcept
{
    return std::holds_alternative<bool>(data_);
}

bool Value::is_number() const noexcept
{
    return std::holds_alternative<std::int64_t>(data_);
}

bool Value::is_string() const noexcept
{
    return std::holds_alternative<std::string>(data_);
}

bool Value::is_array() const noexcept
{
    return std::holds_alternative<array_type>(data_);
}

bool Value::is_object() const noexcept
{
    return std::holds_alternative<object_type>(data_);
}

bool Value::as_bool() const
{
    if (!is_bool()) {
        type_error("bool");
    }
    return std::get<bool>(data_);
}

std::int64_t Value::as_i64() const
{
    if (!is_number()) {
        type_error("integer");
    }
    return std::get<std::int64_t>(data_);
}

const std::string& Value::as_string() const
{
    if (!is_string()) {
        type_error("string");
    }
    return std::get<std::string>(data_);
}

const Value::array_type& Value::as_array() const
{
    if (!is_array()) {
        type_error("array");
    }
    return std::get<array_type>(data_);
}

const Value::object_type& Value::as_object() const
{
    if (!is_object()) {
        type_error("object");
    }
    return std::get<object_type>(data_);
}

Value::array_type& Value::as_array()
{
    if (!is_array()) {
        type_error("array");
    }
    return std::get<array_type>(data_);
}

Value::object_type& Value::as_object()
{
    if (!is_object()) {
        type_error("object");
    }
    return std::get<object_type>(data_);
}

bool Value::contains(std::string_view key) const
{
    return find(key) != nullptr;
}

const Value& Value::at(std::string_view key) const
{
    const Value* found = find(key);
    if (!found) {
        throw DriftError(ErrorCode::MissingField, "missing JSON field: " + std::string(key));
    }
    return *found;
}

const Value* Value::find(std::string_view key) const noexcept
{
    if (!is_object()) {
        return nullptr;
    }
    const auto& object = std::get<object_type>(data_);
    auto it = object.find(std::string(key));
    if (it == object.end()) {
        return nullptr;
    }
    return &it->second;
}

void Value::push(Value value)
{
    if (!is_array()) {
        data_ = array_type{};
    }
    as_array().push_back(std::move(value));
}

void Value::set(std::string key, Value value)
{
    if (!is_object()) {
        data_ = object_type{};
    }
    as_object()[std::move(key)] = std::move(value);
}

Value parse(std::string_view source)
{
    return Parser(source).parse_root();
}

Value parse_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw DriftError(ErrorCode::InvalidArgument, "unable to open fixture: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parse(buffer.str());
}

std::string stringify(const Value& value, bool pretty)
{
    return Writer(pretty).write(value);
}

std::string escape(std::string_view input)
{
    std::string out;
    out.reserve(input.size() + 8);
    for (char ch : input) {
        switch (ch) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                out += "\\u00";
                constexpr char hex[] = "0123456789abcdef";
                out.push_back(hex[(ch >> 4) & 0x0F]);
                out.push_back(hex[ch & 0x0F]);
            } else {
                out.push_back(ch);
            }
        }
    }
    return out;
}

std::string require_string(const Value& object, std::string_view key)
{
    return object.at(key).as_string();
}

std::string optional_string(const Value& object, std::string_view key, std::string fallback)
{
    const Value* found = object.find(key);
    if (!found || found->is_null()) {
        return fallback;
    }
    return found->as_string();
}

std::int64_t require_i64(const Value& object, std::string_view key)
{
    return object.at(key).as_i64();
}

std::int64_t optional_i64(const Value& object, std::string_view key, std::int64_t fallback)
{
    const Value* found = object.find(key);
    if (!found || found->is_null()) {
        return fallback;
    }
    return found->as_i64();
}

bool optional_bool(const Value& object, std::string_view key, bool fallback)
{
    const Value* found = object.find(key);
    if (!found || found->is_null()) {
        return fallback;
    }
    return found->as_bool();
}

const Value::array_type& require_array(const Value& object, std::string_view key)
{
    return object.at(key).as_array();
}

const Value::object_type& require_object(const Value& object, std::string_view key)
{
    return object.at(key).as_object();
}

} // namespace drift::json
