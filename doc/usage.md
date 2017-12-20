# HowTo Use xmr-stak

## Content Overview
* [Configuration](#configuration)
* [Usage on Windows](#usage-on-windows)
* [Usage on Linux](#usage-on-linux)
* [Command Line Options](#command-line-options)
* [HTML and JSON API report configuraton](#xx)

## Configurations

Before you started the miner the first time there are no config files available.
Config files will be created at the first start.
The number of files depends on the available backends.
`config.txt` contains the common miner settings.
`amd.txt`, `cpu.txt` and `nvidia.txt` contains miner backend specific settings and can be used for further tuning ([Tuning Guide](tuning.md)).


## Usage on Linux & MacOS
1) Open a terminal within the folder with the binary
2) Start the miner with `./xmr-stak`

## Command Line Options

The miner allow to overwrite some of the settings via command line options.
Run `xmr-stak --help` to show all available command line options.

## Docker image usage

You can run the Docker image the following way:

```
docker run --rm -it -u $(id -u):$(id -g) --name fireice-uk/xmr-stak -v "$PWD":/mnt xmr-stak
docker stop xmr-stak
docker run --rm -it -u $(id -u):$(id -g) --name fireice-uk/xmr-stak -v "$PWD":/mnt xmr-stak --config config.txt
```

Debug the docker image by getting inside:

```
docker run --entrypoint=/bin/bash --rm -it -u $(id -u):$(id -g) --name fireice-uk/xmr-stak -v "$PWD":/mnt xmr-stak
```
