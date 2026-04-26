# UniTool

Configuration utility for the Framethrower video scaler, targeting Amiga systems equipped with PiStorm/Emu68.

## Features

- Adjust crop, dimensions, offsets, aspect ratio, and phase
- Configure scaling kernel parameters (B/C) and smoothing
- Scanline settings (normal and interlaced)
- Save/load presets in IFF format
- Copy settings directly to clipboard as `config.txt` format
- GUI and command-line modes
- Localized (English, Deutsch, Français)

## Requirements

- PiStorm/Emu68 version 1.1+
- Amiga OS with MUI (Magic User Interface)
- Framethrower hardware with `unicam.resource` version 1.2+

## Usage

Launch `UniTool` for the GUI, or use CLI arguments:

```
UniTool [LOAD <preset>] [SAVE <preset>] [WIDTH <n>] [HEIGHT <n>]
        [X <n>] [Y <n>] [B <n>] [C <n>] [ASPECT <n>] [PHASE <n>]
        [SCALER <n>] [SMOOTH] [INTEGER] [GUI] [QUIET]
```

| Option | Description |
|---|---|
| `LOAD <path>` | Load settings from a preset file and apply them |
| `SAVE <path>` | Save current settings to a preset file |
| `WIDTH <n>` | Crop width in pixels |
| `HEIGHT <n>` | Crop height in pixels |
| `X <n>` | Horizontal crop offset (must satisfy `X + WIDTH ≤ max_width`) |
| `Y <n>` | Vertical crop offset (must satisfy `Y + HEIGHT ≤ max_height`) |
| `B <n>` | Scaling kernel B parameter (0–1000) |
| `C <n>` | Scaling kernel C parameter (0–1000) |
| `ASPECT <n>` | Aspect ratio multiplied by 1000 (valid range: 333–3000, e.g. 1333 = 4:3) |
| `PHASE <n>` | Pixel phase adjustment (0–255) |
| `SCALER <n>` | Scaler mode selection (0–3) |
| `SMOOTH` | Enable smoothing |
| `INTEGER` | Restrict scaling to integer factors only |
| `GUI` | Force GUI mode even when other arguments are provided |
| `QUIET` | Suppress all console output |

Out-of-range values are silently ignored and the current value is kept.

`LOAD` takes precedence over individual crop/kernel options — when a preset is loaded, `WIDTH`, `HEIGHT`, `X`, `Y`, `B`, `C` are ignored.
