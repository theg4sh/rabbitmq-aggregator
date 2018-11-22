## Description

The project aggregate messages from multiple consumers, merge them and publish into publish exchange.

### Dependencies

```
pthread
libconfig
libconfig-dev
```

## Configure

To configure you'll need to copy `config.cfg.example` to `config.cfg`
and set `consumeFrom` and `publishTo` groups.

See configuration syntax:
https://www.hyperrealm.com/libconfig/libconfig_manual.html#Configuration-Files

## How to build?

It's simple, you'll need to run:

```
make
```
