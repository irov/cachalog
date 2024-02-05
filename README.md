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

```bash
docker image save -o ./ch_grid_dev.tar cachalot-dev
rsync -avz ./ch_grid_dev.tar user@address.dest:builds/
// on server
sudo docker load builds/ch_grid_dev.tar
```
or
```bash
docker image save cachalot:dev | gzip | ssh -C username@address.dest sudo docker load
docker image save cachalot:dev | gzip | pv | ssh -C username@address.dest 'zcat | sudo docker load'
```
Docker image сохраняет в tar => можно сжать через gzip
pv (pipe viewer) - сторонняя утилита

**Options:**

- `--no-cache` - Каждый раз запускать сборку с нуля.
- `--rm` - Удалить после завершения контейнера.
- `-it` - Запускать в режиме терминала.
- `--name` - Имя контейнера.
- `--entrypoint` - Команда, которая будет запускаться при создании контейнера (по умолчанию *ch_grid*).
- `-p [host_port]:[container_port]` - Порт, к которому будет слушать контейнер.
- `-e [VAR=VALUE]` or `--env [VAR=VALUE]` - Переменные окружения для контейнера:
    * `CACHALOT__TOKEN` - токен для CACHALOT

rsync:
- a: Режим архива, который сохраняет разрешения файлов, времена изменения, рекурсивность и другие атрибуты файлов.
- v: Подробный вывод, который отображает информацию о передаваемых файлах.
- z: Сжатие данных во время передачи для ускорения процесса.

