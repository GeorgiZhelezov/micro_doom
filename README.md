# Hello World

## Overview

A simple sample that can be used with any [supported board](#boards) and prints "Hello World" to the console.

## Building and Running

This application can be built and executed on QEMU as follows:

```markdown
zephyr-app-commands:
  :zephyr-app: samples/hello_world
  :host-os: unix
  :board: qemu_x86
  :goals: run
  :compact:
```

To build for another board, change "qemu_x86" above to that board's name.

## Sample Output

```console
Hello World! x86
```

Exit QEMU by pressing `CTRL+A` `x`.
