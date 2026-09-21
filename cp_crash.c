/* cp_crash.c — trigger a CP crash or soft re-init on Exynos cpif modem
 * ioctls pulled from drivers/soc/google/cpif/modem_prj.h (magic 'o')
 *
 * usage:
 *   cp_crash crash            # full crash -> cbd auto reload -> reinit
 *   cp_crash reinit           # soft re-init only (no crash)
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_MAGIC 'o'
#define IOCTL_TRIGGER_CP_CRASH _IO(IOCTL_MAGIC, 0x34)
#define IOCTL_POWER_RESET      _IOW(IOCTL_MAGIC, 0x21, struct boot_mode)

enum cp_boot_mode {
	CP_BOOT_MODE_NORMAL,
	CP_BOOT_MODE_DUMP,
	CP_BOOT_RE_INIT,
};

struct boot_mode {
	enum cp_boot_mode idx;
};

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s crash|reinit\n", argv[0]);
		return 1;
	}

	if (!strcmp(argv[1], "crash")) {
		int fd = open("/dev/umts_ipc0", O_RDWR);
		if (fd < 0) { perror("open umts_ipc0"); return 1; }
		if (ioctl(fd, IOCTL_TRIGGER_CP_CRASH, 0) < 0) {
			perror("ioctl IOCTL_TRIGGER_CP_CRASH");
			close(fd);
			return 1;
		}
		printf("crash triggered; cbd will auto reload/reinit\n");
		close(fd);
	} else if (!strcmp(argv[1], "reinit")) {
		int fd = open("/dev/umts_boot0", O_RDWR);
		if (fd < 0) { perror("open umts_boot0"); return 1; }
		struct boot_mode bm = { .idx = CP_BOOT_RE_INIT };
		if (ioctl(fd, IOCTL_POWER_RESET, &bm) < 0) {
			perror("ioctl IOCTL_POWER_RESET(CP_BOOT_RE_INIT)");
			close(fd);
			return 1;
		}
		printf("soft re-init issued\n");
		close(fd);
	} else {
		fprintf(stderr, "usage: %s crash|reinit\n", argv[0]);
		return 1;
	}

	return 0;
}
