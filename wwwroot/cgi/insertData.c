#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mysql.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main()
{
    printf("mysql client Version: %s\n", mysql_get_client_info());

    int fd = open("./test.jpeg", O_WRONLY|O_CREAT, 0666);
    if(fd < 0)
    {
        printf("push failed\n");
        return 0;
    }
    int content_length = atoi(getenv("CONTENT_LENGTH"));
    int i = 0; 
    char c;
    for(; i < content_length; i++)
    {
        read(0, &c, 1);
        write(fd, &c, 1);
    }
    return 0;
}
