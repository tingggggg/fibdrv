#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

int main()
{
    char write_buf[] = "testing writing";
    int offset = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        long long t31, t16, t6, t_clz;

        lseek(fd, i, SEEK_SET);
        t31 = write(fd, write_buf, 1);
        t_clz = write(fd, write_buf, 2);
        t16 = write(fd, write_buf, 3);
        t6 = write(fd, write_buf, 4);

        printf("%d %lld %lld %lld %lld\n", i, t31, t16, t6, t_clz);
    }

    close(fd);
    return 0;
}
