# cachalot

## Docker

```bash
docker build <options> -t cachalot:dev .
```

```bash
// create container
docker run --name cachalot-dev -p 5555:5555 cachalot:dev

// stop container
docker stop cachalot-dev

// run existing container (after create container)
docker start cachalot-dev
```

Debug and development container
```bash
docker run --rm -it --name cachalot-test -p 5555:5555 --entrypoint /bin/bash cachalot:dev
docker run  --rm -it --name cachalot-test
```

**Options:**

- `--no-cache` - Каждый раз запускать сборку с нуля.
- `--rm` - Удалить после завершения контейнера.
- `-it` - Запускать в режиме терминала.
- `--name` - Имя контейнера.
- `--entrypoint` - Команда, которая будет запускаться при создании контейнера (по умолчанию *ch_grid*).
- `-p [host_port]:[container_port]` - Порт, к которому будет слушать контейнер.

