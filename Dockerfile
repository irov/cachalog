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
RUN bash ./build_dependencies_debug.bash 
RUN bash ./build_solution_debug.bash


FROM debian:10

LABEL org.opencontainers.image.title = "Cachalot"

RUN apt-get update && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/bin/cachalot_unix/ /app/tmp
RUN ["cp", "/app/tmp/Unix Makefiles/Debug/ch_grid", "/app"]

# RUN echo '$CONFIG' >> /app/docker_config.json
RUN echo '{ "max_thread": 8, "max_record": 50, "max_time": 30, "grid_uri": "0.0.0.0", "grid_port": 5555, "token": "adgnp9283hrn;agd", "name": "docker_ch_grid" }' >> /app/docker_config.json
EXPOSE 5555
RUN chmod +X ./ch_grid
ENTRYPOINT ["/app/ch_grid", "--config", "/app/docker_config.json"]
# CMD ["--config", "/app/docker_config.json"]


