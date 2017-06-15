#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE* raw = NULL;

    printf("open file 2.\r\n");
    raw = fopen(argv[1],"r+");
    if(raw == NULL){
        printf("open file failed!\r\n");

        return -1;
    }

    fclose(raw);

    printf("open file 2.\r\n");
    raw = fopen(argv[1],"r+");
    if(raw == NULL){
        printf("open file failed twice!\r\n");

        return -1;
    }

    fclose(raw);

    return 0;
}
