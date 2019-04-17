#include <stdio.h>
#include <stdlib.h>
int MAX_WORD_LENGTH = 20;

int word_count(char* file)
{
    FILE * f = fopen(file, "r");
	if (f == NULL) return 1;
    int count = 0;
	char c;
	while((c = fgetc(f)) != EOF)
	{
		if(c == ' ' || c == '\n')
		{
			count++;
		}
	}
    fclose(f);
    return count;
}

char** ftoa(char* file){
    int size = word_count(file);
    FILE * f = fopen(file, "r");
    char **data = (char **)malloc(size * sizeof(char *)); 
	int i;
    for ( i=0; i<size; i++) 
         data[i] = (char *)malloc(MAX_WORD_LENGTH * sizeof(int));
	if (f == NULL) return 1;
	char c;
	int count = 0;
    i = -1;
	while((c = fgetc(f)) != EOF)
	{
		if(c == ' ' || c == '\n')
		{
            data[count][++i] = '\0';
            count++;
            i = -1;
		}
		else
		{
            data[count][++i] = c;
		}
	}
	fclose(f);
    return data;
}

int main(int argc, char **argv){
    int size = word_count("db.txt");
    printf("%d", size);
    char **data = ftoa("db.txt");
	int i;
    for (i= 0; i < size; i++){
        printf("\n%s", data[i]);
    }
    printf("\n");
	return 0;
}
