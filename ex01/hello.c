#include <stdio.h>
#include <string.h>

int main() {
    char input[100];
    printf("Hello World!\n");
    printf("请输入一些内容: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        printf("你输入的是: %s\n", input);
    } else {
        printf("读取输入时发生错误。\n");
    }
    
    return 0;
}
