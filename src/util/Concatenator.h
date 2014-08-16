// TODO: include guards, license, comments

template <typename Value>
struct ValueToString {
  static void append(StringBuffer &buf, Value value) {
    buf.concat(value);
  }
};

template <typename Value>
struct ValueToQuotedString {
  static void append(StringBuffer &buf, Value value) {
    buf.concat('"');
    buf.concat(value);
    buf.concat('"');
  }
};

template <typename Value>
struct QuoteStringsOnly : ValueToString<Value> {};

template <>
struct QuoteStringsOnly<const char*> : ValueToQuotedString<const char*> {};

template <>
struct QuoteStringsOnly<char*> : ValueToQuotedString<char*> {};

template <>
struct QuoteStringsOnly<String&> : ValueToQuotedString<String&> {};

template <>
struct QuoteStringsOnly<char> : ValueToQuotedString<char> {};

template < template<typename Value> class AppendValue = ValueToString >
struct  Concatenator {
  template <typename Sep>
  static void concat(StringBuffer& buf, Sep sep) {
  }

  template <typename Sep, typename Arg>
  static void concat(StringBuffer& buf, Sep sep, Arg arg) {
    AppendValue<Arg>::append(buf, arg);
  }

  template <typename Sep, typename Arg, typename... Args>
  static void concat(StringBuffer& buf, Sep sep, Arg arg, Args... args...) {
    AppendValue<Arg>::append(buf, arg);
    buf.concat(sep);
    concat(buf, sep, args...);
  }
};

