#include <stdio.h>
#include <stdlib.h>

int main() {

	char * str = "This,is,a,string,yay,go,irish,yeet";
	char *token;
	while ((token = strsep(&str, ","))) printf("%s", token);


}
