#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// cac bien nay se la ten ham giao tiep voi kernel thong qua IOCTL
#define MOUSE_IOCTL_BASE      'M'
#define IOCTL_GET_STATUS      _IOR(MOUSE_IOCTL_BASE, 1, struct mouse_data)

// bien nay luu cac gia tri lay duoc tu Kernel
struct mouse_data {
	unsigned char left;
	unsigned char right;
	unsigned char wheel_click;
	char x;
	char y;
	char wheel;
};

int main() {
	// mo file driver
    int fd = open("/dev/usbmouse", O_RDONLY);
    if (fd < 0) {
        perror("loi mo file ko duoc");
        return 1;
    }
	// khai bao bien giao tiep voi kernel
    struct mouse_data data;
    while (1) {
	// lay gia tri tu kernel
	// Neu thanh cong se in ra man hinh
        if (ioctl(fd, IOCTL_GET_STATUS, &data) == 0) {
            printf("L: %d R: %d W: %d X: %d Y: %d Wheel: %d\n",
                data.left, data.right,data.wheel_click,(signed char)data.x, (signed char)data.y, (signed char)data.wheel);
        }
	// ham se lap lai moi 100 ms
        usleep(100000); 
    }
	//dong file driver
    close(fd);
    return 0;
}
