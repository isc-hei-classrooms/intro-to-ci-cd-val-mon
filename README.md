# Dummy DB

This repository contains the sources of Dummy DB, a teeny-tiny toy database written in C++.

## Installation

Dummy DB is written in pure C++ and can be used as a single-header library.
Simply import `dummydb.hpp` to your project, and you're done!

## Usage

Here's is a minimal usage example:

```c++
#include <dummydb.hpp>
#include <iostream>

int main() {
  // Create a database capable of storing at most 4 tables.
  ddb::DummyDB db{4};

  // Create a table whose records are pairs of floating-point numbers and
  // assign the identity of that table to the local variable `t0`. The primary
  // key of the table is implicitly defined as an auto-incremented integer.
  auto t0 = db.create_table({ddb::Float, ddb::Float});

  // Create another table whose records are triples containing one
  // floating-point number and two integers.
  auto t1 = db.create_table({ddb::Float, ddb::Integer, ddb::Integer});

  // Inserts a couple of records into the first table and assign the identity
  // of the first (i.e., its primary key) to local variables.
  auto r0 = db.insert(t0, {3.14f, 9.81f});
  db.insert(t0, {1.66f, 2.17f});

  // Inserts a record into the second table.
  db.insert(t1, {3.14f, 42, 23});

  // Lookup the contents of the record identified by `r0`.
  auto data = db.record(t0, r0);
  for (auto i : data) {
    std::cout << std::get<1>(i) << std::endl;
  }

  return 0;
}
```

You can also compile this particular example and execute it on your machine with the following commands:

```bash
make
./build/main
```

The current version of Dummy DB supports 32-bit integers and 32-bit floating-point numbers.

## Testing

You can test this distribution of Dummy DB using the following command:

```bash
make test
```

## CI/CD

TO BE COMPLETED 
