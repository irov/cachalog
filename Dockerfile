FROM debian:10 as builder

ENV DEBUG=True

RUN apt-get update && apt-get install -y \
  build-essential \
  libsnappy-dev \
  libssl-dev \
  libz-dev \
  openssl \
  python3 \
  cmake \
  git

WORKDIR /app

COPY . .

WORKDIR /app/build/downloads/
RUN bash ./downloads_unix.bash

WORKDIR /app/build/unix/
RUN bash ./build_dependencies_debug.bash & bash ./build_solution_debug.bash


# FROM debian:10

# LABEL org.opencontainers.image.title = "Cachalot"

# RUN apt-get update && rm -rf /var/lib/apt/lists/*

# WORKDIR /app
# COPY ["--from=builder", "/app/bin/cachalot_unix/Unix Makefiles/Debug/ch_grid", "/app"]
# ARG bin_file="/app/bin/cachalot_unix/Unix\\ Makefiles/Debug/ch_grid"
# COPY --from=builder ["/app/bin/cachalot_unix/", "/app"]
# EXPOSE 5555
# RUN apt-get install tree
# RUN tree /app/bin/cachalot_unix
# RUN chmod +X ./ch_grid
# CMD ["./ch_grid"]


