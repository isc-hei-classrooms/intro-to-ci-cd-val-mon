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

This project implements automated Continuous Integration (CI) and Continuous Deployment (CD) pipelines using GitHub Actions and Docker.

### Overview

- **CI (Continuous Integration)**: Runs on every push and pull request to `main`. Builds and tests the code in a clean environment.
- **CD (Continuous Deployment)**: Runs only when a release is created. Produces versioned artifacts (binary + Docker image) and publishes them.

### Workflows

#### CI Workflow ([.github/workflows/test.yml](.github/workflows/test.yml))

**Trigger conditions:**
- Push to `main` branch
- Pull requests targeting `main`

**Jobs:**
1. `test`: Builds and tests natively
   - `make` - compiles the project
   - `make test` - runs unit tests

2. `docker-build`: Builds and smoke tests Docker image
   - `docker build` - builds the multi-stage Docker image (includes tests)
   - `docker run` - smoke test to verify the container starts successfully

The workflow fails if compilation, tests, or Docker build fails.

#### CD Workflow ([.github/workflows/deploy.yml](.github/workflows/deploy.yml))

**Trigger conditions:**
- Release creation (manual trigger on GitHub)

**Jobs:**
1. `ci`: Runs CI checks before deploying
   - Ensures the release is only created from passing code

2. `cd`: Builds and publishes release artifacts
   - Builds optimized release binary (`make release`)
   - Uploads artifacts to GitHub release:
     - `dummydb-<version>`
     - `LICENSE`
   - Builds and pushes Docker image to GitHub Container Registry (GHCR):
     - `ghcr.io/isc-hei-classrooms/intro-to-ci-cd-val-mon:<version>`
     - `ghcr.io/isc-hei-classrooms/intro-to-ci-cd-val-mon:latest`
   - Depends on: `ci` job (CD does not run if CI fails)

**Artifact naming:**
- Binary: `dummydb-<git-tag>` (e.g., `dummydb-v1.0.3`)
- Docker image: Tagged with both release version and `latest`

### Implementation Details

#### Task 1 - Basic CI/CD (commit `64cd44b`)

The initial implementation added two workflows:
- `test.yml`: Runs `make` and `make test` on every push/PR
- `deploy.yml`: Runs on release creation, builds release binary and uploads artifacts

**Issues encountered:** 
- Missing `permissions:` block (needed `contents: write` for release uploads, later `packages: write` for GHCR)
- Commented line that was causing the workflow to fail when uncommented
- The `deploy.yml` workflow stopped triggering on new releases. Multiple attempts were needed to diagnose and fix the issues, which explains the high number of "fix deploy" commits. The workflow required manual disabling and re-enabling in GitHub's Actions settings to properly reload.

#### Task 2 - Docker Support (commit `b5cfeca`)

Added Docker support with a multi-stage build:

**Dockerfile features:**
- **Builder stage**: Debian Bookworm with g++ and make
  - Compiles with `make release`
  - Runs tests with `make test` (build fails if tests fail)
- **Runtime stage**: Debian Bookworm Slim
  - Minimal image (no build tools)
  - Runs as non-root user (`dummydb`)
  - Only contains the compiled binary

**Docker constraints satisfied:**
- Multi-stage build ✓
- Tests run during build ✓
- Runtime image smaller than builder ✓
- Non-root user ✓

### Pulling and Running the Docker Image

**Authentication issue:** When I tried to pull the image from GHCR, I encounter this problem
```
Error response from daemon: unauthorized
```

**Solution:** Create a GitHub Personal Access Token (PAT):
1. GitHub → Settings → Developer settings → Personal access tokens → Tokens (classic)
2. Generate new token with `read:packages` permission
3. Login to GHCR:
```bash
echo YOUR_TOKEN | docker login ghcr.io -u your-username --password-stdin
```

**Pull and run the image:**
```bash
docker pull ghcr.io/isc-hei-classrooms/intro-to-ci-cd-val-mon:latest
docker run ghcr.io/isc-hei-classrooms/intro-to-ci-cd-val-mon:latest
```

Alternatively, if the package is set to public visibility, no authentication is required.
For our repo, the organistation disable this possibility.

### Reproducing the Build Locally
```bash
docker build -t dummydb .
docker run dummydb
```
