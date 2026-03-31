#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

// function declarations
void printBits(unsigned char c);
int maxConsecutiveOnes(unsigned char c);
int compareByBits(const void* el1, const void* el2);
void* mybsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*));
void addToArray(void** arr, int* size, void* newElem, size_t elemSize);
void updateOddBits(char* c);

int main()
{
	char inputBuffer[100];
	FILE* file = NULL;
	LIST list1, list2;
	NODE* p1, * p2;
	char* dynamicArray = NULL;
	int arrSize = 0;
	char tempChar;
	int i, strLen, searchLen;
	char* foundChar;
	char keyChar;

	// get string from user
	printf("Please enter the string:\n");
	if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL)
	{
		// remove \n from the end
		char* newline = strchr(inputBuffer, '\n');
		if (newline) *newline = '\0';
	}

	// open file for both read and write
	file = fopen("dat.1", "wb+");
	if (!file)
	{
		printf("Error opening file!\n");
		return 1;
	}

	// write string and null terminator to file
	strLen = (int)strlen(inputBuffer);
	fwrite(inputBuffer, sizeof(char), strLen + 1, file);

	// go back to start of file
	rewind(file);

	// init lists
	if (!L_init(&list1) || !L_init(&list2))
	{
		fclose(file);
		return 1;
	}

	p1 = &list1.head;
	p2 = &list2.head;
	int listToggle = 0;

	// read chars and split to two lists
	while (fread(&tempChar, sizeof(char), 1, file) == 1)
	{
		if (listToggle == 0)
		{
			p1 = L_insert(p1, (DATA)tempChar);
			listToggle = 1;
		}
		else
		{
			p2 = L_insert(p2, (DATA)tempChar);
			listToggle = 0;
		}
	}

	// move data to dynamic array
	p1 = list1.head.next;
	p2 = list2.head.next;

	while (p1 != NULL || p2 != NULL)
	{
		if (p1 != NULL)
		{
			tempChar = (char)p1->key;
			addToArray((void**)&dynamicArray, &arrSize, &tempChar, sizeof(char));
			p1 = p1->next;
		}
		if (p2 != NULL)
		{
			tempChar = (char)p2->key;
			addToArray((void**)&dynamicArray, &arrSize, &tempChar, sizeof(char));
			p2 = p2->next;
		}
	}

	// free memory
	L_free(&list1);
	L_free(&list2);

	// print bits before sort
	printf("The bits in the array, BEFORE the sorting, are:\n");
	for (i = 0; i < arrSize; i++)
		printBits((unsigned char)dynamicArray[i]);

	// sort array (without the sentinel)
	if (arrSize > 1)
	{
		qsort(dynamicArray, arrSize - 1, sizeof(char), compareByBits);
	}

	// print bits after sort
	printf("The bits in the array, AFTER the sorting, are:\n");
	for (i = 0; i < arrSize; i++)
		printBits((unsigned char)dynamicArray[i]);

	printf("And the string AFTER the sort is:\n");
	printf("%s\n", dynamicArray);

	// search part
	printf("Please enter the sequence length to search for (unsigned number):\n");
	if (scanf("%d", &searchLen) != 1) searchLen = 0;

	// create dummy key for comparison
	keyChar = 0;
	for (i = 0; i < searchLen && i < 8; i++) {
		keyChar |= (1 << i);
	}

	// search using mybsearch
	foundChar = (char*)mybsearch(&keyChar, dynamicArray, arrSize - 1, sizeof(char), compareByBits);

	if (foundChar == NULL)
	{
		printf("Can not find such a sequence!\n");
	}
	else
	{
		printf("The character %c has been found!\n", *foundChar);

		updateOddBits(foundChar);

		printf("The bits in the array, AFTER THE BITS UPDATE, are:\n");
		for (i = 0; i < arrSize; i++)
			printBits((unsigned char)dynamicArray[i]);

		printf("And the string AFTER THE BITS UPDATE is:\n");
		printf("%s\n", dynamicArray);
	}

	free(dynamicArray);
	fclose(file);

	return 0;
}

// helper function to add to array
void addToArray(void** arr, int* size, void* newElem, size_t elemSize)
{
	void* temp;
	if (*arr == NULL)
		temp = malloc(elemSize);
	else
		temp = realloc(*arr, (*size + 1) * elemSize);

	if (temp != NULL)
	{
		*arr = temp;
		char* dest = (char*)(*arr) + (*size * elemSize);
		memcpy(dest, newElem, elemSize);
		(*size)++;
	}
}

// print 8 bits
void printBits(unsigned char c)
{
	int i;
	for (i = 0; i < 8; i++)
		printf("%d", (c << i & 0x80) ? 1 : 0);
	printf("\n");
}

// count max consecutive ones
int maxConsecutiveOnes(unsigned char c)
{
	int max = 0, current = 0, i;
	for (i = 0; i < 8; i++)
	{
		if ((c >> i) & 1)
			current++;
		else {
			if (current > max) max = current;
			current = 0;
		}
	}
	if (current > max) max = current;
	return max;
}

// compare function for qsort
int compareByBits(const void* el1, const void* el2)
{
	unsigned char c1 = *(unsigned char*)el1;
	unsigned char c2 = *(unsigned char*)el2;
	return maxConsecutiveOnes(c1) - maxConsecutiveOnes(c2);
}

// generic binary search implementation
void* mybsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*))
{
	size_t left = 0, right = num - 1, mid;
	int res;
	const char* ptr = (const char*)base;

	if (num == 0) return NULL;

	while (left <= right)
	{
		mid = left + (right - left) / 2;
		const void* midElem = ptr + (mid * size);

		res = compar(key, midElem);

		if (res == 0)
			return (void*)midElem;
		else if (res > 0)
			left = mid + 1;
		else
		{
			if (mid == 0) break;
			right = mid - 1;
		}
	}
	return NULL;
}

// turn on odd bits
void updateOddBits(char* c)
{
	*c = *c | 0x55;
}