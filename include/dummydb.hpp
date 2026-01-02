#include <algorithm>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <limits>

namespace ddb {

/// The size of a table.
constexpr std::size_t table_size = 4096;

/// The size of a string table.
constexpr std::size_t string_table_size = 4096;

/// A value indicating that a record or string was not found.
constexpr std::size_t not_found = std::numeric_limits<std::size_t>::max();

/// The type of a field in a table.
enum FieldType : std::uint8_t {
  Integer, Float, String
};

/// The value of a field.
using Value = std::variant<std::int32_t, double, std::string>;

/// Returns `address` advanced by `byte_offset` bytes.
void* advanced(void* address, std::size_t byte_offset) {
  return static_cast<void*>(static_cast<std::byte*>(address) + byte_offset);
}

/// Returns `x` rounded up to the nearest multiple of `n`, which is a power of two.
template<typename N>
N rounded_up_to_nearest_multiple(N x, N n) {
  auto r = x & (n - 1);
  return (r == 0) ? x : x + (n - r);
}

/// A collection of tables.
class DummyDB final {
private:

  /// The header of the storage of a database.
  struct Header {

    /// The offset of the header relative to the start of the storage's allocation.
    const std::size_t offset;

    /// The maximum number of tables that the database can hold (excluding the string table).
    const std::size_t max_table_count;

    /// The number of tables in the database (excluding the string table).
    std::size_t table_count;

  };

  /// The raw data in the database.
  ///
  /// This pointer refers to an instance of `Header` and a tail-allocated byte buffer storing the
  /// contents of the database. Note that this pointer may be offset w.r.t. the dynamic allocation
  /// performed in the constructor to satisfy the alignment requirements, hence calling `delete[]`
  /// on this pointer may cause undefined behavior.
  ///
  /// Tables are allocated right after the header and have the same alignment. Each table stores a
  /// header that describes its schema and the number of records that it contains, followed by the
  /// records themselves.
  void* data;

  /// Accesses the header of this database.
  Header& header() const {
    return *static_cast<Header*>(data);
  }

    /// Accesses the header of this database.
  Header& header() {
    return *static_cast<Header*>(data);
  }

  /// Accesses the table with the given identity.
  void* table(std::size_t identity) {
    return advanced(data, sizeof(Header) + string_table_size + (identity * table_size));
  }

  /// Accesses the string table of this database.
  char* string_table() const {
    return static_cast<char*>(data) + sizeof(Header);
  }

  /// Returns the total capacity of the database.
  std::size_t capacity() const {
    auto n = header().max_table_count;
    auto a = std::max(alignof(Header), alignof(Value)) - 1;
    auto s = a + sizeof(Header) + string_table_size + (n * table_size);
    return s;
  }

  /// Returns the offset of `s` in the string table or the position immediately after the last
  /// string in the table.
  std::size_t string_offset(std::string const& s) const {
    auto* ss = string_table();
    std::size_t o = 0;

    while ((o < string_table_size) && (ss[o] != 0)) {
      auto n = static_cast<std::size_t>(reinterpret_cast<unsigned char*>(ss)[o]);
      auto t = ss + o + 1;
      if (std::equal(std::begin(s), std::end(s), t, t + n)) {
        return o;
      } else {
        o += (n + 1);
      }
    }

    return o;
  }

public:

  /// Creates an instance capable of containing up to `max_table_count` tables.
  DummyDB(std::size_t max_table_count) : data(nullptr) {
    // Allocate enough memory to store the header and the tables.
    auto a = std::max(alignof(Header), alignof(Value)) - 1;
    auto s = a + sizeof(Header) + string_table_size + (max_table_count * table_size);

    // Compute the offset of the header to satisfy alignment requirements.
    auto d = new std::byte[s];
    std::fill(d, d + s, std::byte{0});

    // Assign `data` to the start of the header.
    auto header_offset = -(reinterpret_cast<std::uintptr_t>(d) & a) & a;
    data = d + header_offset;
    new(data) Header{header_offset, max_table_count, 0};
  }

  ~DummyDB() {
    auto h = static_cast<Header*>(data);
    auto d = static_cast<std::byte*>(data) - h->offset;
    delete[] d;
  }

  /// Returns the maximum number of tables that the database can hold.
  std::size_t max_table_count() const {
    return header().max_table_count;
  }

  /// Returns the number of tables in the database.
  std::size_t table_count() const {
    return header().table_count;
  }

  /// Creates a new table with the given scheme and returns its identity.
  std::size_t create_table(std::vector<FieldType> const& schema) {
    Header& h = header();
    if (h.table_count == h.max_table_count) {
      throw std::overflow_error("not enough space to create a new table");
    }

    auto t = table(h.table_count);

    // Store the scheme of the table.
    auto s = static_cast<std::uint8_t*>(t);
    *(s++) = static_cast<std::uint8_t>(schema.size());
    for (auto f : schema) {
      *(s++) = static_cast<FieldType>(f);
    }

    // Update the table count, expecting that `h` be a mutable reference on the header.
    return h.table_count++;
  }

  /// Inserts `record` in the table identified by `table_identity` and returns its identity.
  std::size_t insert(std::size_t table_identity, std::vector<Value> const& record) {
    auto t = table(table_identity);
    auto record_width = static_cast<std::size_t>(*static_cast<uint8_t*>(t));
    auto record_size = record_width * sizeof(std::uint32_t);
    auto table_header = rounded_up_to_nearest_multiple(record_width + 1, alignof(std::size_t));

    auto n = static_cast<std::size_t*>(advanced(t, table_header));
    auto b = table_header + sizeof(std::size_t) + ((*n) * record_size);
    if ((b + record_size) > table_size) {
      throw std::overflow_error("table is full");
    }

    // Copy the contents of the record.
    auto p = static_cast<std::uint32_t*>(advanced(t, b));
    for (std::size_t i = 0; i < record_width; ++i) {
      switch (*static_cast<FieldType*>(advanced(t, i + 1))) {
        case Integer:
          *static_cast<std::int32_t*>(static_cast<void*>(p++)) = std::get<0>(record[i]);
          continue;

        case Float:
          *static_cast<double*>(static_cast<void*>(p++)) = std::get<1>(record[i]);
          continue;

        case String:
          auto s = insert_string(std::get<2>(record[i]));
          *(p++) = static_cast<std::uint32_t>(s);
          continue;
      }
    }

    return (*n)++;
  }

  /// Returns the contents of the record identified by `record_identity`, which is stored in the
  /// table identified by `table_identity`.
  std::vector<Value> record(std::size_t table_identity, std::size_t record_identity) {
    auto t = table(table_identity);
    auto record_width = static_cast<std::size_t>(*static_cast<uint8_t*>(t));
    auto record_size = record_width * sizeof(std::uint32_t);
    auto table_header = rounded_up_to_nearest_multiple(record_width + 1, alignof(std::size_t));

    auto b = table_header + sizeof(std::size_t) + (record_identity * record_size);
    auto p = static_cast<std::uint32_t*>(advanced(t, b));

    std::vector<Value> result;
    result.reserve(record_width);
    for (std::size_t i = 0; i < record_width; ++i) {
      switch (*static_cast<FieldType*>(advanced(t, i + 1))) {
        case Integer:
          result.emplace_back(*static_cast<std::int32_t*>(static_cast<void*>(p++)));
          continue;

        case Float:
          result.emplace_back(*static_cast<double*>(static_cast<void*>(p++)));
          continue;

        case String:
          auto s = *static_cast<std::uint32_t*>(static_cast<void*>(p++));
          result.emplace_back(string(s));
          continue;
      }
    }

    return result;
  }

  /* Returns the identity of the string `s` if it is in this database or the maximum representable
  value of `std::size_t` otherwise. */
  std::size_t find_string(std::string const& s) const {
    auto o = string_offset(s);
    if ((o < string_table_size) && (string_table()[o] != 0)) {
      return o;
    } else {
      return not_found;
    }
  }

  /// Inserts `s` in this database if it wasn't already and returns its identity.
  std::size_t insert_string(std::string const& s) {
    auto o = string_offset(s);
    if ((o < string_table_size) && (string_table()[o] != 0)) {
      return o;
    } else if ((o + 1 + s.size()) > capacity()) {
      throw std::overflow_error("table is full");
    } else {
      auto* ss = string_table();
      reinterpret_cast<unsigned char*>(ss)[o] = static_cast<unsigned char>(s.size() & 0xff);
      std::copy(std::begin(s), std::end(s), ss + o + 1);
      return o;
    }
  }

  /// Returns the string identified by `id`:
  std::string string(std::size_t id) {
    auto* ss = string_table();
    auto n = static_cast<std::size_t>(reinterpret_cast<unsigned char*>(ss)[id]);
    return std::string{ss + id + 1, n};
  }

};

}