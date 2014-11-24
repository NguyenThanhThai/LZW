// trie.h
//
// Provides the trie table data structure that is required by lzw. Functions and
// variables are specified below

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct trie_node{
	int suffix; //the integer corresponding to suffix character
	int prefix; //the in dex of the prefix of whole character
	int code; //the overall index of the current node

	int appearances; // num of times this code appears
	
	int kid_count;  // number of kids, if it increases, children array realloced
	int *children; // array holding the code of children
} *trie;


//initialize this trie
void init(trie *table, int flags/*flags*/);

// find number of bits needed to represent a function
int findNumBits(int curr_code, int e);

// frees all kids in the table
void kill_children(trie *table, int total_codes);

// double the memory allocation as needed by the number of codes, and e
int reinit(trie *table, int total_codes, int maxbits, int e);

//inserts the index of the children of qtrie in the children matrix                                       
void insert(trie *table, int pre, int suffix, int code);

//returns 0 if the key isn't in the trie
int search(trie *table, int pre, int suffix, int start, int end);

// prints the information in the table for debugging
void print(trie *table, int total_codes);

// prunes the trie for the -p flag
int prune(trie *start, int total_codes, int e);
