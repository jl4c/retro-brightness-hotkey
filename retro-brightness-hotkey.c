/*
 * retro-brightness-hotkey
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * BTN_TRIGGER_HAPPY5 + BTN_DPAD_UP/DOWN ajustan el brillo del backlight
 * leyendo eventos crudos de evdev.
 *
 * Uso: retro-brightness-hotkey [/dev/input/eventX] [porcentaje]
 *      (por defecto /dev/input/event2 y 10%)
 */

#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <unistd.h>

#define EVENT_DEV   "/dev/input/event2"
#define MAX_PATH    "/sys/class/backlight/backlight/max_brightness"
#define BRI_PATH    "/sys/class/backlight/backlight/brightness"
#define DEFAULT_PCT 10

static int g_max = -1; /* max_brightness no cambia, se lee una sola vez */

static int read_int(const char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;

	char buf[16];
	ssize_t n = read(fd, buf, sizeof(buf) - 1);
	close(fd);

	if (n <= 0)
		return -1;

	buf[n] = '\0';
	return atoi(buf);
}

static void write_int(const char *path, int value)
{
	int fd = open(path, O_WRONLY);
	if (fd < 0)
		return;

	char buf[12];
	char *p = buf + sizeof(buf);

	do {
		*--p = (char)('0' + (value % 10));
		value /= 10;
	} while (value > 0);

	ssize_t w = write(fd, p, (size_t)(buf + sizeof(buf) - p));
	(void)w;
	close(fd);
}

static void adjust_brightness(int sign, int pct)
{
	if (g_max < 0)
		return;

	int cur = read_int(BRI_PATH);
	if (cur < 0)
		return;

	int step = g_max * pct / 100;
	if (step < 1)
		step = 1;

	int next = cur + sign * step;
	if (next < 0)
		next = 0;
	if (next > g_max)
		next = g_max;

	write_int(BRI_PATH, next);
}

int main(int argc, char *argv[])
{
	const char *dev = (argc > 1) ? argv[1] : EVENT_DEV;
	int pct = DEFAULT_PCT;

	if (argc > 2) {
		int p = atoi(argv[2]);
		if (p > 0 && p <= 100)
			pct = p;
	}

	g_max = read_int(MAX_PATH);

	int fd;
	while ((fd = open(dev, O_RDONLY)) < 0)
		sleep(1);

	struct input_event ev;
	int hotkey_held = 0;

	while (read(fd, &ev, sizeof(ev)) == (ssize_t)sizeof(ev)) {
		if (ev.type != EV_KEY)
			continue;

		switch (ev.code) {
		case BTN_TRIGGER_HAPPY5:
			hotkey_held = (ev.value != 0);
			break;
		case BTN_DPAD_UP:
			if (hotkey_held && ev.value == 1)
				adjust_brightness(+1, pct);
			break;
		case BTN_DPAD_DOWN:
			if (hotkey_held && ev.value == 1)
				adjust_brightness(-1, pct);
			break;
		}
	}

	close(fd);
	return 0;
}
