# retro-brightness-hotkey

A minimal C daemon for adjusting screen brightness on Linux-based retro
handheld gaming devices (Anbernic R36S, RG353V, and similar) using a gamepad
hotkey combination — no on-screen menu required.

## How it works

The program reads raw `evdev` events from `/dev/input/eventX`. While a
hotkey button is held down, pressing D-Pad Up or Down increases or decreases
the backlight brightness by a percentage of its maximum value, written
directly to the backlight's `brightness` sysfs attribute.

- Hotkey: `BTN_TRIGGER_HAPPY5`
- Increase: `BTN_DPAD_UP`
- Decrease: `BTN_DPAD_DOWN`

The implementation avoids `stdio` and heap allocations: brightness is read
and written with raw `open`/`read`/`write` syscalls, and `max_brightness` is
read once at startup and cached.

## Usage

```
retro-brightness-hotkey [/dev/input/eventX] [percentage]
```

- `/dev/input/eventX` — input device to read (default: `/dev/input/event2`)
- `percentage` — brightness step as a percentage of max, 1-100 (default: `10`)

If the input device isn't available yet (e.g. at early boot), the program
waits and retries every second until it appears.

## Building

```
gmake
```

To cross-compile, e.g. for an aarch64 musl handheld from an x86_64 host:

```
gmake CC="clang -target aarch64-chimera-linux-musl"
```

`CFLAGS` defaults to `-Os -Wall -Wextra` and `LDFLAGS` to `-s` (stripped
binary).

## Installing

```
gmake install PREFIX=/usr
```

This installs:

- `$(PREFIX)/bin/retro-brightness-hotkey`
- `$(PREFIX)/lib/dinit.d/retro-brightness-hotkey` (service file, see below)

## Running as a dinit service

A service file is provided in `dinit.d/retro-brightness-hotkey`:

```
type = process
command = /usr/bin/retro-brightness-hotkey /dev/input/event2
restart = true
smooth-recovery = true
```

After installing, enable it with:

```
dinitctl enable retro-brightness-hotkey
```

The service runs as root by default, so it can write to the backlight's
`brightness` attribute without relying on udev group permissions.

## Per-device configuration

The following are compile-time constants in `retro-brightness-hotkey.c` and
may need to be adjusted for your specific device:

- `MAX_PATH` / `BRI_PATH` — paths to the backlight's `max_brightness` and
  `brightness` sysfs attributes. Check `/sys/class/backlight/` for the
  correct backlight name on your device.
- `BTN_TRIGGER_HAPPY5`, `BTN_DPAD_UP`, `BTN_DPAD_DOWN` — the hotkey and D-Pad
  button codes. Verify these with `evtest` against your device's
  `/dev/input/eventX`, since different handhelds may map their buttons
  differently.
- `EVENT_DEV` (`/dev/input/event2` by default) — the input device path also
  varies by device; pass the correct path as the first argument or set it in
  the dinit service file.

## Tested on

- R36S (Chimera Linux)
- RG353V (in progress)

## License

GPL-2.0-only — see [LICENSE](LICENSE)
