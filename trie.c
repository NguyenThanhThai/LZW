// trie.c
//
// Provides the trie data structure for lzw, accompanied with trie.h, which 
// defines the trie struct

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "trie.h"

//initialize this trie
void init(trie *table, int e /*flags*/)
{
	(*table) = malloc(512*sizeof(struct trie_node));      //// maxbits - 9

	////////// NULL
	(*table)[0].code = 0;
	(*table)[0].suffix = 0;
	(*table)[0].prefix = 0;
	(*table)[0].appearances = 0;
	(*table)[0].kid_count = 0;
	(*table)[0].children = NULL;
	(*table)[0].children = malloc(2 * sizeof(int));
	
	////////// ESC
	(*table)[1].prefix = 0;
	(*table)[1].code = 1;
	(*table)[1].suffix = 0;
	(*table)[1].appearances = 0;
	(*table)[1].kid_count = 0;
	(*table)[1].children = malloc(2 * sizeof(int));
	
	////////// EOF
	(*table)[2].prefix = 0;
	(*table)[2].code = 2;
	(*table)[2].suffix = 0;
	(*table)[2].appearances = 0;
	(*table)[2].kid_count = 0;
	(*table)[2].children = malloc(2 * sizeof(int));
	
	//trie *table;
	if (e == 0){
		for(int i = 0; i < 256; i++){
			(*table)[i+3].suffix = i;
			(*table)[i+3].prefix = 0;
			(*table)[i+3].code = i+3;

			(*table)[i+3].appearances = 0;
			(*table)[i+3].kid_count = 0;
			(*table)[i+3].children = malloc(2 * sizeof(int));
		}
	} 
	//return table;
}

int findNumBits(int curr_code, int e){		//gives you num bits to repr an int
	int num_bits = 0;
	int code_bits = curr_code;
	while(code_bits > 0){
		code_bits = code_bits / 2;
		num_bits++;
	}
	if (num_bits < 8 && e == 0) //////should return 9?
		return 8;
	return num_bits;
}

// different version of findNumBits
// int findNumBits(int curr_code, int e){
// 	int bits = 0;
// 	int code = curr_code + 1;
// 	while (code !=0){
// 		bits++;
// 		code = curr_code >> bits;
// 	}
// 	if (bits < 8 && e == 0) //////should return 9?
// 		return 8;
// 	return bits;
// }


int reinit(trie *table, int total_codes, int maxbits, int e)
{										//expands the memory for the table by 2
	if (findNumBits(total_codes, e) > findNumBits(total_codes - 1, e)){
		if (findNumBits(total_codes, e) <= maxbits){
			(*table) = realloc(*table,2*(total_codes)*sizeof(struct trie_node));
			return findNumBits(total_codes, e);
		}
		return maxbits;
	}
	return findNumBits(total_codes, e);
	
}

void kill_children(trie *table, int total_codes){				//frees kids
	for (int i = 0; i < total_codes; i++){
		if ((*table)[i].children != NULL){
			free((*table)[i].children);
		}
	}
}

//inserts the index of the children of qtrie in the children matrix                                       
void insert(trie *table, int pre, int suffix, int code)
{
													//assign the kid w/ suffix
	(*table)[code].code = code;
	(*table)[code].suffix = suffix;
	(*table)[code].prefix = (*table)[pre].code;
 
	(*table)[code].appearances = 1;
	(*table)[code].kid_count = 0;
	(*table)[code].children = NULL;

	//put this suffix in pre's child
	(*table)[pre].children = realloc((*table)[pre].children, 
		((*table)[pre].kid_count + 1)* sizeof(int));
	(*table)[pre].children[(*table)[pre].kid_count] = code;
	(*table)[pre].kid_count++;

	int i;
	for(i = (*table)[pre].kid_count - 2; i >= 0; i--){
		if ((*table)[(*table)[pre].children[i]].suffix > suffix){
			int temp = (*table)[pre].children[i];
			(*table)[pre].children[i] = code;
			(*table)[pre].children[i+1] = temp;
		} else{
			break;
		}
	}

}

//returns the position in the trie table of suffix, returns 0 if not in table
int search(trie *table, int pre, int suffix, int start, int end)
{
	if (start > end)
		return -1;

	int middle = (start + end) / 2;
	int middle_code = (*table)[pre].children[middle];
	int middle_suffix = (*table)[middle_code].suffix;
	if (middle_suffix == suffix)
		return middle_code;
	else if (suffix > middle_suffix){
		return search(table, pre, suffix, middle + 1, end);
	} else{
		return search(table, pre, suffix, start, middle - 1);
	}

}


//print function
void print(trie *table, int total_codes){
	printf("Code\tPref\tSuff\tAppearances\n"); 
	for (int i = 0; i < total_codes; i++){
		printf("%i\t%i\t%i\t%i\n", (*table)[i].code, (*table)[i].prefix, (*table)[i].prefix, (*table)[i].appearances);
	}

}


// start is stack variable pointing to stack variable pointing to heap variable
// prunes the whole table and returns a new table, with the rest freed
int prune(trie *start, int total_codes, int e){
	trie end;		
	init(&end, e);
	int new_count = 259;
	if (e == 1)
		new_count = 3;

	int start_count = new_count;

	for (int i = 0; i < new_count; i++){
		end[i].appearances = (*start)[i].appearances/2;
	}
	while(start_count < total_codes){
		(*start)[start_count].appearances /= 2; 
		if ((*start)[start_count].appearances > 0){
			
			insert(&end, (*start)[start_count].prefix, 
				(*start)[start_count].suffix, new_count);

			for (int i = 0; i < (*start)[start_count].kid_count; i++){
				(*start)[(*start)[start_count].children[i]].prefix = new_count;
			}
			new_count++;
			reinit(&end, new_count, 20, e);
		}
		start_count++;
	}

	kill_children(start, total_codes);
	free(*start);
	*start = end;
	return new_count;
}
