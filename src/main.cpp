#include "dummydb.hpp"

#include <iostream>

int main() {
  // Create a database capable of storing at most 4 tables.
  ddb::DummyDB db{4};

  // Create a table whose records are pairs of floating-point numbers and
  // assign the identity of that table to the local variable `t0`. The primary
  // key of the table is implicitly defined as an auto-incremented integer.
  auto t0 = db.create_table({ddb::Float, ddb::Float});

  // Create another table whose records are triples containing one
  // floating-point number, one integer, and one string.
  auto t1 = db.create_table({ddb::Float, ddb::Integer, ddb::String});

  // Inserts a couple of records into the first table and assign the identity
  // of the first (i.e., its primary key) to local variables.
  auto r0 = db.insert(t0, {3.14f, 9.81f});
  db.insert(t0, {1.66f, 2.17f});

  // Lookup the contents of the record identified by `r0`.
  auto d0 = db.record(t0, r0);
  for (auto i : d0) {
    std::cout << std::get<1>(i) << std::endl;
  }

  // Inserts a record into the second table.
  auto r1 = db.insert(t1, {3.14f, 42, "Hello, World!"});

  // Lookup the contents of the record identified by `r1`.
  auto d1 = db.record(t1, r1);
  std::cout << std::get<std::string>(d1[2]) << std::endl;

  return 0;
}
