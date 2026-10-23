#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/* redirects stdout to out.txt, then runs ls -l */
int main(void)
{
    close(1);  /* close stdout */

    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    fprintf(stderr, "out.txt opened with file descriptor %d\n", fd);

    execlp("ls", "ls", "-l", (char *)NULL);
    perror("exec");
    return 127;
}
