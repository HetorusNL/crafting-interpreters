# Crafting Interpreters

Repository with the code/documentation of the crafting interpreters book/website.

The book/website is found here:  
https://craftinginterpreters.com/

## Information

Two interpreters will be created in Java and in C (maybe the Java one is reimplemented in Python later).
Challenges will be implemented in a copy of the interpreters as created according to the crafting interpreters book.

## Installation

Perform the following to install the dependencies of this repository (assuming an Arch Linux installation):

```bash
sudo pacman -S caddy jdk-openjdk less xdg-utils
```

## Profiling

Run callgrind (tool of valgrind) on the executable like:

```bash
valgrind --tool=callgrind src/c/craftinginterpreters/build/crafting-interpreters-c benchmark.lox
```

Then run kcachegrind on the resulting callgrind output (callgrind adds the PID value of the command to the output file name, so substituting '\*' with that):

```bash
kcachegrind callgrind.out.*
```

## FAQ

## License

MIT License, Copyright (c) 2024 Tim Klein Nijenhuis <tim@hetorus.nl>
