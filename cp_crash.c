
/* hardcrash.c — trigger a CP crash on Exynos cpif modem, no args
 * ioctl pulled from drivers/soc/google/cpif/modem_prj.h (magic 'o')
 * cbd auto-reloads/reinits the CP after this; umts_router comes back
 * once phone_state returns to ONLINE.
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
 
#define IOCTL_TRIGGER_CP_CRASH _IO('o', 0x34)
 
int main(void)
{
	int fd = open("/dev/umts_ipc0", O_RDWR);
	if (fd < 0) { perror("open umts_ipc0"); return 1; }
 
	if (ioctl(fd, IOCTL_TRIGGER_CP_CRASH, 0) < 0) {
		perror("ioctl IOCTL_TRIGGER_CP_CRASH");
		close(fd);
		return 1;
	}
 
	printf("crash triggered; cbd will auto reload/reinit\n");
	close(fd);
	return 0;
}
 
