# Running This Wallet Module Against logosctl

`logos-wallet-module` is a Logos `core` module that wraps the
[go-wallet-sdk](https://github.com/status-im/go-wallet-sdk) C library to provide
Ethereum transaction construction and (RPC-backed) chain access. This doc-test
exercises **this** wallet-module commit end-to-end through the headless
`logosctl` runtime:

1. Build the `logosctl` CLI from its published flake. `logosctl` is the
   headless frontend for `logos-liblogos`, so building it brings in the whole
   module-runtime stack (`logos_host`, `liblogos_core`, the IPC layer).
2. Build **this** wallet module as an installable `.lgx` package straight from
   its own flake's `#lgx` output, **pinned to the commit under test** — so the
   module you run is built from exactly what is checked out here, not the latest
   published release.
3. Start a `logosctl` daemon and install the `.lgx` into its session.
4. Load `wallet_module`, introspect it with `module show`, and call several of
   its **offline** transaction helpers — verifying the module actually runs and
   round-trips real values through the go-wallet-sdk. (We deliberately avoid the
   RPC methods so the run needs no network access and is reproducible in CI.)

Because the module is built from the commit under test and then loaded and called
through a real `logosctl` daemon, a green run is real evidence that this change
keeps the wallet module loadable and callable.

**What you'll build:** This `wallet_module`, packaged as `.lgx`, installed into a `logosctl` session, and called through its daemon.

**What you'll learn:**

- How to build the `logosctl` runtime from its flake
- How a module's flake exposes a ready-to-install `.lgx` via its `#lgx` output
- How to install an `.lgx` into a `logosctl` session
- How to start the `logosctl` daemon, load a module, introspect it, and call its methods
- How to pass a JSON argument to a method with logosctl's `@file` parameter syntax
- How to shut the daemon down and confirm it has exited

## Prerequisites

- **Nix** with flakes enabled. Install from [nixos.org](https://nixos.org/download.html), then enable flakes:

```bash
mkdir -p ~/.config/nix
echo 'experimental-features = nix-command flakes' >> ~/.config/nix/nix.conf
```

Verify: `nix flake --help >/dev/null 2>&1 && echo "Flakes enabled"`

- **A Linux or macOS machine.**

---

## Step 1: Build logosctl

Build the `logosctl` CLI from its published flake. The result is symlinked to
`./logos/`. `logosctl` is the headless frontend for `logos-liblogos`, so this
one build brings in the whole module-runtime stack the daemon needs.

### 1.1 Build the CLI

```bash
nix build 'github:logos-co/logos-logoscore-cli#ctl' --out-link ./logos
```

The build produces `logos/bin/logosctl` plus bundled runtime libraries
and a `logos/modules/` directory containing the built-in
`capability_module` (required for the auth handshake when loading
modules).

---

## Step 2: Build and install this wallet module

Build **this** wallet module's `.lgx` straight from its flake's `#lgx` output,
start a daemon, and install the package into its session. Every module
built with
[`logos-module-builder`](https://github.com/logos-co/logos-module-builder)
exposes a ready-to-install `#lgx`.

> The `` in the URL is what pins the build to a specific commit: the
> doc-test runner expands it to a concrete ref. Locally that is this
> checkout's `HEAD` (see `run.sh`); in CI it is the commit being tested. With
> no pin it falls back to the latest `master`.

### 2.1 Build the module's .lgx

Build the `#lgx` output and link it as `./wallet-lgx`. (This compiles the
module and its SDK dependencies through Nix, so the first build is slow.)

```bash
# From inside the clone this is simply: nix build '.#lgx'
nix build 'github:logos-co/logos-wallet-module#lgx' -o wallet-lgx
```

The `.lgx` package is now under `./wallet-lgx/`:

```bash
ls wallet-lgx/*.lgx
```

### 2.2 Start the daemon

Start a `logosctl` daemon. Its session directory, `./session`, holds the
daemon's config, its installed modules and their data. `--detach`
returns once the daemon accepts commands.

```bash
logosctl daemon start --detach
```

### 2.3 Install the .lgx

A path ending in `.lgx` installs from disk, with no catalog involved. A
local build is unsigned, which the session's default policy accepts with
a warning. `-y` confirms, since there is no terminal to ask on.

```bash
logosctl install wallet-lgx/*.lgx -y
```

### 2.4 Confirm the install

List the session's packages and confirm the module landed:

```bash
logosctl package ls
```

---

## Step 3: Call the module

Load `wallet_module`, introspect it, and call several of its offline
transaction helpers with the client subcommands. The daemon logs to
`session/logs/daemon.log`.

### 3.1 Write a sample transaction

The transaction helpers take a JSON-encoded transaction. Write a minimal
legacy (EIP-155, chainId 1) transaction to `tx.json`; we pass it to the
methods below with logosctl's `@file` argument syntax.

```json
{
  "type": "0x0",
  "nonce": "0x0",
  "gasPrice": "0x3b9aca00",
  "gas": "0x5208",
  "to": "0x70997970c51812dc3a010c7d01b50e0d17dc79c8",
  "value": "0x0",
  "input": "0x",
  "chainId": "0x1",
  "v": "0x26",
  "r": "0x1",
  "s": "0x1"
}
```

### 3.2 Inspect the startup log

Review the daemon's startup output:

```bash
cat session/logs/daemon.log
```

### 3.3 Check daemon status

Verify the daemon is running:

```bash
logosctl daemon status
```

### 3.4 List discovered modules

`wallet_module` should be visible in the scan directory:

```bash
logosctl module ls
```

### 3.5 Load the module

Load `wallet_module` into the running daemon:

```bash
logosctl module load wallet_module
```

### 3.6 Confirm the module is loaded

Re-run `daemon status`; the module that was `not_loaded` before now reports
`loaded`:

```bash
logosctl daemon status
```

### 3.7 Introspect the module with module show

`module show` lists the `Q_INVOKABLE` methods the module exposes — the
same methods you can `call`:

```bash
logosctl module show wallet_module
```

### 3.8 Hash a transaction

`transactionGetHash` computes the hash of the transaction in `tx.json`.
The `@tx.json` argument tells logosctl to load the file's contents as the
argument. The result is a real `0x…` keccak hash from the go-wallet-sdk:

```bash
logosctl call wallet_module transactionGetHash @tx.json
```

### 3.9 Encode the transaction to RLP

`transactionJsonToRlp` serialises the transaction to its RLP hex
encoding:

```bash
logosctl call wallet_module transactionJsonToRlp @tx.json
```

### 3.10 Build an ETH transfer transaction

`txGeneratorTransferETH` builds an unsigned ETH-transfer transaction from
a small JSON parameter object. Write the parameters to `transfer.json` and
pass them to the generator with `@transfer.json`:

```json
{
  "to": "0x70997970c51812dc3a010c7d01b50e0d17dc79c8",
  "value": "1000000000000000000",
  "nonce": 0,
  "gasLimit": 21000,
  "gasPrice": "1000000000",
  "chainID": "1"
}
```

```bash
logosctl call wallet_module txGeneratorTransferETH @transfer.json
```

### 3.11 Stop the daemon

Shut the daemon down cleanly:

```bash
logosctl daemon stop
```

The daemon removes its state file and exits.

### 3.12 Confirm the daemon has stopped

With no daemon running, the client reports `not_running` and exits
non-zero, so we add `|| true` to let the doc-test assert on the output:

```bash
logosctl daemon status
```
