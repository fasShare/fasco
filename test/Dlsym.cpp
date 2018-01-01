#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

int main() {
    printf("uid = %d\n", getuid());
    printf("-------------------------\n");
    accept(0, NULL, NULL);
}

