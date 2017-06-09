#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE* raw = NULL;

    raw = fopen("0220170607150830","r+");
    if(raw == NULL){
        printf("open file failed!\r\n");

        return -1;
    }

    return 0;
}
