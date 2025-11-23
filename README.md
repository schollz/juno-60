# juno-60

Roland Juno-60 synthesizer emulation.

## SuperCollider Plugin

This repository includes a C++ implementation of the Juno-60 synthesizer as a SuperCollider UGen plugin. The implementation is based on the JavaScript junox library.

### Quick Start

```bash
# Build and install the SuperCollider plugin
make install

# Or specify a custom SuperCollider path
make SC_PATH=/path/to/supercollider install
```

See [sc-plugin/README.md](sc-plugin/README.md) for detailed documentation.

## junox (JavaScript Web Audio)

The `junox` directory contains the original JavaScript implementation using Web Audio API.