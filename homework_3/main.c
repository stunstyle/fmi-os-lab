#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_TOKEN_LENGTH 2048

void append(char* s, char c)
{
	int len = strlen(s);
	s[len] = c;
	s[len+1] = '\0';
}

void checkArgCount(int argc)
{	if (argc < 3)
	{
		errx(1, "usage: ./main <dictionary file> <word to search for>");
	}
}

void movePointerToAfterDelim(int fd, char delim)
{
	int readStatus = 0;
	char c;

	while((readStatus = (read(fd, &c, 1))) != -1)
	{
		if(c == delim)
		{
			return;
		}
	}
}

void readUntilDelim(int fd, char* word, char delim)
{
	int readStatus = 0;
	char c;
	char buffer[MAX_TOKEN_LENGTH] = "";

	while((readStatus = (read(fd, &c, 1))) != -1)
	{
		if(c == delim)
		{
			strcpy(word,buffer);
			return;
		}
		append(buffer, c);
	}
}

bool binarySearch(int dictionaryFd, char* word, char* result) 
{
	int dictionarySize = lseek(dictionaryFd, 0, SEEK_END);	
	int mid = dictionarySize / 2;
	int L = 0;
	int R = dictionarySize - 1;
	
	while(L < R)
	{
		char currWord[MAX_TOKEN_LENGTH] = "";
		mid = (L + R) / 2;
		lseek(dictionaryFd, mid, SEEK_SET);
		
		movePointerToAfterDelim(dictionaryFd, '\0');
		readUntilDelim(dictionaryFd, currWord, '\n');

		int status = strcmp(currWord, word);
		if (status == 0)
		{
			char definition[MAX_TOKEN_LENGTH] = "";
			readUntilDelim(dictionaryFd, definition, '\0');
			strcpy(result,definition);
			return 1;	
		}

		else if (status < 0)
		{
			L = mid + 1;
			continue;
		}

		else if (status > 0)
		{
			R = mid - 1;
			continue;
		}
	}	
	return 0;
}

int main(int argc, char* argv[])
{
	checkArgCount(argc);	
	int dictionaryFd; 
	if((dictionaryFd = open(argv[1], O_RDONLY)) == -1)
	{
		err(2, "could not open dictionary file");
	}

	char result[MAX_TOKEN_LENGTH] = "";
	bool wordFound = binarySearch(dictionaryFd, argv[2], result);
	close(dictionaryFd);

	if(wordFound)
	{
		write(1, result, strlen(result));
		exit(0);
	}
	else
	{
		errx(3, "word not found in dictionary");
	}
}
