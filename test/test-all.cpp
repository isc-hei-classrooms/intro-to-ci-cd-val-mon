#include <dummydb.hpp>
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  "create_database"_test = [] {
    ddb::DummyDB db{4};
    expect(db.max_table_count() == 4);
  };

  "create_table"_test = [] {
    ddb::DummyDB db{4};
    expect(db.table_count() == 0);
    db.create_table({ddb::Integer, ddb::Integer});
    expect(db.table_count() == 1);
  };

  "create_table_full"_test = [] {
    ddb::DummyDB db{0};
    expect(throws([&] {
      // Error: not enough space to create a new table.
      db.create_table({ddb::Integer, ddb::Integer});
    }));
  };

  "insert_string"_test = [] {
    ddb::DummyDB db{0};
    auto i = db.insert_string("Hello");
    auto j = db.insert_string("Hello");
    expect(i == j);

    auto k = db.insert_string("World");
    expect(i != k);
  };

  "find_string"_test = [] {
    ddb::DummyDB db{0};
    auto i = db.insert_string("Hello");
    auto j = db.insert_string("World");

    expect(db.find_string("Hello") == i);
    expect(db.find_string("World") == j);
    expect(db.find_string("Empty") == ddb::not_found);
  };

  "string"_test = [] {
    ddb::DummyDB db{0};
    auto i = db.insert_string("Hello");
    auto j = db.insert_string("World");

    expect(db.string(i) == "Hello");
    expect(db.string(j) == "World");
  };

  return 0;
}
