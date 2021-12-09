# ROPfuscator [![Build](https://github.com/ropfuscator/ropfuscator/actions/workflows/main.yaml/badge.svg)](https://github.com/ropfuscator/ropfuscator/actions/workflows/main.yaml)
![logo](./docs/logo.png)

ROPfuscator is a fine-grained code obfuscation framework for C/C++ programs using ROP (return-oriented programming).
ROPfuscator obfuscates a program at the assembly code level by transforming regular instructions into ROP chains, thwarting our natural conception of normal control flow.
It is implemented as an extension to LLVM x86 backend.

For build, usage and implementation, see individual documents:

- Building ROPfuscator: [build.md](./docs/build.md)
- Using ROPfuscator to obfuscate programs: [usage.md](./docs/usage.md)
- Obfuscation algorithm details: [algorithm.md](./docs/algorithm.md)
- Implementation details: [implementation.md](./docs/implementation.md)

## ROPfuscator Overview

![architecture](./docs/architecture.svg)

We combine the following obfuscation layers to achieve robust obfuscation against several attacks.

- ROP Transformation
  - Convert each instruction into one or more ROP gadgets, and translate the entire code to ROP chains.
- Opaque Predicate Insertion
  - Translate ROP gadget address(es) and stack pushed values into opaque constants, which are composition of multiple opaque predicates.
- Instruction Hiding
  - Instead of applying ROP transformation to all instructions, pick up some original instructions before ROP transformation and interleave them with the opaque predicate instructions.

## Supported platform and limitations

- Linux 32-bit x86 binary is the only supported target (as of now)
- Tested on the following platform, though it is expected to work on other Linux x86 distribution.
  - Host (compiler): Ubuntu 18.04 (64bit)
  - Target (runtime): Ubuntu 18.04 (64bit OS with 32bit support / 32bit OS)
- Compatible LLVM versions: 9
- For detailed limitations, see [limitation.md](./docs/limitation.md).

# Nix instructions

## Using `ccache`

Make sure to have created a folder that `ccache` can use as buffer and that `nix-daemon` has that path added to the sandbox whitelisted directories.

To do so, make sure to have this in your `configuration.nix`:

```nix
extra-sandbox-paths = /nix/var/cache/ccache
```

You can then use `ccache` by setting the right ownership to the buffer directory:

```sh
# mkdir -m0770 -p /nix/var/cache/ccache
# chown --reference=/nix/store /nix/var/cache/ccache
```
