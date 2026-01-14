# Build stage
FROM debian:bookworm AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY Makefile .
COPY src/ src/
COPY include/ include/
COPY test/ test/

RUN make release
RUN make test

# Runtime stage
FROM debian:bookworm-slim

RUN useradd -m -u 1000 dummydb

WORKDIR /app

COPY --from=builder /app/build/main /app/dummydb

USER dummydb

CMD ["/app/dummydb"]
