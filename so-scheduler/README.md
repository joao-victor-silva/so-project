# SO Scheduler

## Compile

### Main program (escalona)

```shell
gcc src/escalona.c -o escalona
```

### Test programs (teste*)

```shell
gcc src/teste10.c -o teste10
gcc src/teste20.c -o teste20
gcc src/teste30.c -o teste30
```

### Everything, the fun way (requires xargs)

```shell
ls src | sed 's/\.c//' | xargs -I {} gcc src/{}.c -o {}
```

### Everything, the boring way (requires make)

```shell
make
```

## Run

As the specification required, to run you need to call `escalona <core_count> <quantum> <input_file_name>`. Below are a example of command to run:

```shell
escalona 1 5 input.txt
```

Obs: you can pass the `LOG_LEVEL` environment variable to activate and control the application logs, here are two examples of it.

```shell
LOG_LEVEL=INFO escalona 1 5 input.txt
```

and

```shell
LOG_LEVEL=DEBUG escalona 1 5 input.txt
```
