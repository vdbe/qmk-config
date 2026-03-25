# QMK Configuration Repository

This repository contains my personal [QMK](https://docs.qmk.fm)
keyboard configurations and builds. It follows the [external
userspace](https://docs.qmk.fm/newbs_external_userspace) pattern.

## Cloning

This repository uses Git submodules. Clone with one of the following methods:

**Option 1: Clone with submodules (recommended)**
```bash
git clone --recurse-submodules https://codeberg.org/vdbe/qmk-config.git
```

**Option 2: Clone and initialize/update submodules afterward**
```bash
git clone https://codeberg.org/vdbe/qmk-config.git
```
See the Updating section for how to initialize/update the submodules

## Updating

After cloning, to get the latest QMK firmware version committed to this repo:

```bash
git pull
git submodule update --init --recursive
```

To update to a new QMK firmware version (for contributors):

```bash
cd qmk_firmware
git fetch --tags
git checkout <version-tag>
cd ..
git add qmk_firmware
git commit -m "chore(qmk_firmware): update to <version>"
```

Replace `<version-tag>` with the desired version tag (e.g., `0.32.5`). See [QMK
Releases](https://github.com/qmk/qmk_firmware/releases) for available versions.

## Repository Structure

This repository uses the [external
userspace](https://docs.qmk.fm/newbs_external_userspace) pattern, keeping
keymaps and custom code separate from the main QMK firmware tree.


- **qmk_firmware/**: Git submodule pointing to the main [QMK
  firmware](https://github.com/qmk/qmk_firmware) repository (required for
  building)
- **scripts/**: Python scripts for keymap generation and other utilities

## Building

See the [QMK Docs](https://docs.qmk.fm) for detailed build instructions.

```bash
# Using Make - see https://docs.qmk.fm/getting_started_make_guide
make
```
