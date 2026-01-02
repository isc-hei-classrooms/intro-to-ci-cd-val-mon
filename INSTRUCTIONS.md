# 203.3 - CI/CD and Docker for reproducible builds

During this week and the next, you will set up Continuous Integration (CI) and Continuous Deployment (CD) pipelines for the DummyDB project.

The emphasis of this course is on automation, reproducibility, and discipline: we will not deploy anything to the cloud.

## What CI/CD means in the context of this course

To avoid ambiguity, we use the following definitions:

- **Continuous Integration (CI)**: Every change to the codebase is automatically built and tested in a clean, isolated environment. CI answers the question: “Can this change be safely integrated?”

- **Continuous Deployment (CD)**: A versioned, immutable artifact is automatically produced and published when a [release is created](https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository). In this course, publishing binaries and container images counts as deployment.

We do **not** deploy servers or services.

## Objectives

By the end of these two weeks, you should be able to:

- Explain the difference between CI and CD using concrete examples.
- Set up CI pipelines using GitHub Actions.
- Produce release artifacts automatically and reproducibly.
- Use Docker to make builds deterministic across machines.
- Identify and debug CI/CD failures using logs.

## Format and Resources

This is a self-guided, hands-on assignment.

You are expected to:

- Consult online documentation and tutorials.
- Experiment, break things, and fix them.
- Document what you did and why, not just the final result.

Some useful resources:

- [A 6-minute introduction to CI/CD](https://www.youtube.com/watch?v=YLtlz88zrLg)
- [GitHub Actions Documentation](https://docs.github.com/en/actions), especially the [Write workflow](https://docs.github.com/en/actions/how-tos/write-workflows) section.
- [Docker Documentation](https://docs.docker.com/get-started/overview/), especially the [Dockerfile reference](https://docs.docker.com/engine/reference/builder/) and [Best practices for writing Dockerfiles](https://docs.docker.com/develop/develop-images/dockerfile_best-practices/).

## Definition of Done

Your repository must satisfy all of the following:

- CI runs automatically on every push and PR.
- CI fails when compilation or tests fail.
- CD runs only when a release is created.
- CD does not run if CI fails.
- Release artifacts include:
  - a compiled DummyDB binary,
  - a Docker image of DummyDB pushed to `ghcr.io`,
  - the LICENSE file.

A third party can reproduce the build using Docker and your documentation.

## Task 1 – Basic CI/CD Using GitHub Runners

GitHub provides hosted runners that can execute CI/CD pipelines free of charge.

### T1.1 Set up a CI workflow

Create a workflow file in `.github/workflows/` that:

- Runs on:
  - `push` to `main`
  - `pull_request` targeting `main`

- Executes:

    ```bash
    make
    make test
    ```

- Fails if compilation or tests fail.

This workflow implements **Continuous Integration**.

### T1.2 Separate CI from CD

Add logic so that:

- CI runs on pushes and pull requests.
- CD runs only on release creation (or version tag push).
- CD depends on CI (i.e. CD must not run if CI fails).

You may use:

- separate jobs, or
- separate workflows.

Document your choice.

### T1.3 Release artifacts

When a release is created:

- Build a release binary of DummyDB.
- Upload the following as release artifacts:
  - the compiled binary,
  - the LICENSE file.

Artifacts must be downloadable from the GitHub release page.

## Task 2 – Reproducible Builds Using Docker

GitHub runners evolve over time. To make builds reproducible, you will encode the build environment explicitly using Docker.

### T2.1 Create a Dockerfile

Write a `Dockerfile` that:

- Uses a [multi-stage build](https://docs.docker.com/build/building/multi-stage/).
- Builds and tests DummyDB in the build stage.
- Produces a minimal runtime image.

Skeleton:

```Dockerfile
# Build stage
FROM debian:bookworm AS builder
# build + test

# Runtime stage
FROM debian:bookworm-slim
# runtime only
```

**Dockerfile constraints (mandatory)**:

- Compilers and build tools must not be present in the runtime image.
- The image build must fail if tests fail.
- The runtime image must be smaller than the build image.
- The container must run as a non-root user.

### T2.2 Verify build and test in Docker

When running:

```bash
docker build .
```

- The build must compile DummyDB.
- Tests must run inside the container.
- Any failure must stop the build.

Document at least one Docker build failure you encountered and how you fixed it.

### T2.3 Integrate Docker into CI

Add a GitHub workflow that:

- Builds the Docker image in CI.
- Runs the container as a [smoke test](https://en.wikipedia.org/wiki/Smoke_testing_(software)).
- Fails CI if the container fails to run.

This workflow is still CI, not CD.

### T2.4 Publish artifacts on release

On release creation:

- Push the Docker image to GitHub Container Registry (GHCR).
- Tag the image with the release version.
- Also upload the native binary and LICENSE as release artifacts.

The image name must be valid and lowercase.

### T2.5 Pull the image on your local machine

Ensure that your Docker image can run on your local machine by logging into `ghcr.io` and pull the image. Ask other students with different architectures to do the same (e.g., `amd64` vs `arm64`).

Think about these aspects:

- The target architecture of your Docker image (`amd64`, `arm64`, or multi-arch).
- Any platform mismatch you encountered and how you handled it.

## Check that you are done

Experiment with your CI/CD pipelines by producing the following scenarios:

- A commit that breaks CI and causes it to fail.
- A fix that restores CI.
- A Docker build failure due to a bug in the codebase or Dockerfile.
- The change that fixes the Docker build.
- A successful release producing all artifacts.

You must update the `README.md` file with a clear description of:

- CI trigger conditions.
- CD trigger conditions.
- Artifact naming and versioning.

How to reproduce the build locally:

```sh
docker build -t <image-name> .
docker run <image-name>
```
