# zvcr_utils
A collection of helpful utilities related to the [zvcr file format](https://github.com/2b2tplace/zvcr) and the
[Minecraft Anvil file format](https://minecraft.wiki/w/Anvil_file_format), in the form of a C++ library and a command 
line program.

# Command Usage
The `zvcr` CLI accepts parameters in sequence, denoted by a `-` prefix: \
`zvcr -flag -arg1=value1 -arg2=value2`

Basic parameter syntax: \
`-parameter=value` \
`-parameter='string value with spaces'` \
`-flag`

## Modes
You can choose from the following modes to do operations on zvcr / mca files, Minecraft worlds, etc. 

Some commands require the use of Minecraft registries (`-reg='/path/to/registries'`), which you can download 
[here](https://github.com/2b2tplace/mc-cpp/tree/main/registries). You may also optionally set the 
`ZVCR_CLI_MINECRAFT_REGISTRIES_PATH` environment variable to avoid passing in the `-reg` parameter every time.

In the help page,\
optional parameters are denoted with `[]`: `[-optional=value]`, \
variadic parameters are denoted with `[...]`: `-parameters=values[...]` (example: `zvcr -parameters=value1 value2 value3`).

- `-help`: Shows all commands and their usages. Use `zvcr -help <command>` to see the usage of that command.
- `-merge`: Merges distinct zvcr directories together, preserving their histories and combining their timelines
- `-import`: Imports Minecraft anvil (mca) files to zvcr
- `-export`: Exports zvcr files to Minecraft anvil (mca)
- `-level-dat`: Creates a level.dat file with custom parameters in a Minecraft world folder

### Merge
- `-merge`: Merges distinct zvcr directories together, preserving their histories and combining their timelines
    - `-dim={overworld/nether/end}`
    - `-in='/path/to/input-zvcr-dirs'[...]`
    - `-out='/path/to/output-zvcr-dir'`
    - `[-inputs=x.z or min_x.min_z,max_x.max_z[...] (defaults to all region files in the input directories)]`
    - `[-zstd-level=zstd compression level] (defaults to 8)`
    - `[-zstd-threads=zstd compression threads] (defaults to 16)`
    - `[-threads=number of threads (defaults to number of threads on this system)] (defaults to 32)`
    - `[-backup-existing]`

### Import
- `-import`: Imports Minecraft anvil (mca) files to zvcr
    - `-dim={overworld/nether/end}`
    - `-in='/path/to/minecraft-world-folder'[...]`
    - `-out='/path/to/zvcr-files'`
    - `[-inputs=x.z or min_x.min_z,max_x.max_z[...] (defaults to all region files in the input directories)]`
    - `[-override-epoch=a unix timestamp (in seconds, to override the timestamp of all snapshots; defaults to the recorded timestamp of each chunk)]`
    - `[-zstd-level=zstd compression level] (defaults to 8)`
    - `[-zstd-threads=zstd compression threads] (defaults to 16)`
    - `[-threads=number of threads (defaults to number of threads on this system)] (defaults to 32)`
    - `[-backup-existing]`
    - `-reg='/path/to/registries' (defaults to value of ZVCR_CLI_MINECRAFT_REGISTRIES_PATH environment variable)`

### Export
- `-export`: Exports zvcr files to Minecraft anvil (mca)
    - `-dim={overworld/nether/end}`
    - `-in='/path/to/zvcr-files'[...]`
    - `-out='/path/to/minecraft-world-folder'`
    - `[-inputs=x.z or min_x.min_z,max_x.max_z[...] (defaults to all region files in the input directories)]`
    - `[-epoch=a unix timestamp (in seconds; defaults to current time)]`
    - `[-threads=number of threads (defaults to number of threads on this system)] (defaults to 32)`
    - `-reg='/path/to/registries' (defaults to value of ZVCR_CLI_MINECRAFT_REGISTRIES_PATH environment variable)`

### Level-dat
- `-level-dat`: Creates a level.dat file with custom parameters in a Minecraft world folder
    - `-path='/path/to/minecraft-world-folder'`
    - `[-name='name of the world' (defaults to name of the world folder)]`
    - `[-spawn-dim={overworld/nether/end}]`
    - `[-spawn-pos=x,y,z] (defaults to 0, 0, 0)`
    - `-reg='/path/to/registries' (defaults to value of ZVCR_CLI_MINECRAFT_REGISTRIES_PATH environment variable)`

# Building the CLI from Source
> Note: Windows is currently not officially supported. It will probably compile, however no guarantees are made on 
  functionality.

This project is written in C++23 and uses CMake.

### Dependencies
- [Boost Iostreams](https://www.boost.org/library/latest/iostreams/)

### Clone & Build
```sh
git clone https://github.com/2b2tplace/zvcr_utils.git
cd zvcr_utils

# choosing clang in this example. you may choose a different compiler if you need
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

# executable should appear at ./build/zvcr
cmake --build build --target zvcr_cli --parallel
```

## Include it in your project
Add the following to your CMakeLists.txt:
```cmake
include(FetchContent)

FetchContent_Declare(zvcr_utils
        GIT_REPOSITORY https://github.com/2b2tplace/zvcr_utils.git
        GIT_TAG main
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(zvcr_utils)
target_link_libraries(my_project
        PRIVATE zvcr_utils
)
```