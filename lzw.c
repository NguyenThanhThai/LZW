// lzw.c
//
// Implements the Lempel-Ziv-Welch algorithm for use in encoding and decoding a
// given file. Running encode will encode the stdin into a compressed version.
// Running decode will take stdin's stream of bits and decompress to stdin.
// Encode takes the -m MAXBITS flag (to specify maximum size of a character 
// compression), -e flag (to create a smaller initial table and use escape to 
// add characters), -p flag (to prune the table when it gets to the largest size


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "trie.h"
#include "code.h"

#define MAXBITS		12

// gets the next number. It will try to take two digits, but can take one.
int getNum(){
	char first = getc(stdin);
	int num = (int)first - 48;

	char second = getc(stdin);
	int num2 = (int)second - 48;
	if (num2 >= 0 && num2 < 10)
		num = num * 10 + num2;
	else
		ungetc(second, stdin);
	return num;
}

// encodes the stdin, takes in the three flags
void encode(int maxbits, int e, int p)
{
	int max_codes = 1 << maxbits;					//declaring max num of codes
	printf("%02i:%i:%i:", maxbits, p, e);			//give the beginning info

	trie table; 
	init(&table, e);				//must initialize
	int pre = 0;

	int c;									//int for chars
	int total_codes = 259;					//total code in table
	int offset = 3; 							//offset for w/o -e
	int nBits = 9;								//number of bits needed for repr
	if (e == 1){
		total_codes = 3;
		offset = 0;
		nBits = 2;
	}

	while((c = getchar()) != EOF){
		
		int kids = 0;
		int curr_code;
		if (pre > 2){ 								//if normal code, assign
			kids = table[pre].kid_count;
			curr_code = table[pre].code;
		} else {										//otherwise, go ahead
			if (e == 1){									// if -e
				table[0].appearances += 1;
				int code = search(&table, 0, c, 0, table[pre].kid_count - 1);
				if (code >= 0){							// find the char c
					pre = code;
					continue;
				} else{
					putBits(nBits, 1);
					if(total_codes < max_codes){
						nBits = reinit(&table, total_codes, maxbits, e);
						insert(&table, 0, c, total_codes);
						total_codes++;
					}
					pre = 0; 
					putBits(findNumBits(c,0), c);
					continue;
				}
			}else{										// w/o -e, move on
				pre = c + 3;
				continue;
			}
		}

		table[pre].appearances += 1;				//add to appearances
		int code = search(&table, pre, c+offset, 0, kids - 1);
		if (code >= 0 && (((total_codes + 1) < max_codes) || p == 0)){	
			pre = code; 	//if code is in table, or if predicting prune
			continue;
		} else{								//if the code isn't in table
			putBits(nBits, curr_code);
			if(total_codes < max_codes){/// stage 2
				nBits = reinit(&table, total_codes, maxbits, e);
				insert(&table, pre, c+offset, total_codes);
				total_codes++;

			}
			pre = 0; 										
			ungetc(c, stdin);
		}
		if (max_codes == total_codes && p ==1){			//does the pruning
			putBits(nBits, 0);
			total_codes = prune(&table, total_codes, e);
			nBits = reinit(&table, total_codes, maxbits, e);
			pre = 0;
		}

	}
	if (pre > 2){							//outputs last part if not found
		table[pre].appearances += 1;					
		putBits(nBits, pre);
	}
	putBits(nBits, 2);
	flushBits();
	kill_children(&table, total_codes);				//free all allocated memory
	free(table);
	return;
}

// output recursively prints the chars that arise from entry
void output(trie *table, int entry){
	(*table)[entry].appearances += 1;
	if (entry == 0)
		return;
	output(table, (*table)[entry].prefix);
	putchar((char)(*table)[entry].suffix);
	return;
}

// decodes the stdin stream of bits
void decode()
{
	int maxbits = getNum();				// get maxbits, e, and p from stdin
	getc(stdin);
	int p = getNum();
	getc(stdin);
	int e = getNum();
	getc(stdin);

	int max_codes = 1 << maxbits;						//set max num of codes

	trie table; 
	init(&table, e);				//must initialize
	int old_pre = 0;
	int nBits = 9;
	int offset = 3;

	int c;									//int for chars
	int total_codes = 259;					//total code in table
	if (e == 1){
		offset = 0;
		nBits = 2;
		total_codes = 3;
	}

	while((c = getBits(nBits)) != 2){//check num bits
		if (c == 0 && p == 1){							//prunes when 0 received
			total_codes = prune(&table, total_codes, e);
			nBits = reinit(&table, total_codes, maxbits, e);
			c = getBits(nBits);
			if (c == 2)
				break;
			old_pre = 0;
		}

		if (c == 1 && e == 1){							//if -e and get a 1
			c = getBits(8);
			if(total_codes < max_codes){ 				//stores code, if we can
				nBits = reinit(&table, total_codes, maxbits, e);
				insert(&table, old_pre, c, total_codes);
				total_codes++;
				if (old_pre > 2){
					nBits = reinit(&table, total_codes, maxbits, e);
					insert(&table, 0, c, total_codes);
					total_codes++;
				}
				output(&table, total_codes-1);
				table[total_codes-1].appearances -= 1;
				old_pre = 0;	
			} else								//otherwise, just puts it out
				putchar(c);
			old_pre = 0;
		
		} else {								// normal
			int first_letter = c;
			if (c == total_codes)						// K-omega-K-omega-K
				first_letter = old_pre;
			if (c > total_codes){
				fprintf(stderr,"invalid input\n");
				return;
			}
			while(table[first_letter].prefix != 0)	//go back as far as possible
				first_letter = table[first_letter].prefix;
			
			if(total_codes < max_codes){ 				// store, if possible
				if (old_pre > 2){
					if(e == 0)
						insert(&table, old_pre, first_letter - offset, 
							total_codes);
					if(e == 1)
						insert(&table, old_pre, table[first_letter].suffix, 
							total_codes);
					total_codes++;
				}									//update num of bits
				nBits = reinit(&table, total_codes, maxbits, e);
			}
			output(&table, c);							// print out to stdin
			old_pre = c;
		}	
	}

	kill_children(&table, total_codes);					//free allocated mem
	free(table);
	return;
}

int main(int argc, char const *argv[])
{	
	

	//run the program
	if (strcmp(argv[0], "./encode") == 0)
	{
		// flag parsing
		int maxbits = 12;
		int p = 0;
		int e = 0;
		for(int i = 1; i < argc; i++){
			if (!strcmp(argv[i], "-m")){
				i++;
				if (i >= argc){
					fprintf(stderr,"No maxbits\n");
					return 1;
				}
				//printf("%s\n", argv[i]);
				maxbits = atoi(argv[i]);
				if (maxbits <= 0) {
					fprintf(stderr,"Invalid maxbits\n");
					return 1;
				}else if (maxbits < 8 || maxbits > 20)
			 		maxbits = 12;
			} else if (!strcmp(argv[i], "-e")){
				e = 1;
			} else if (!strcmp(argv[i], "-p")){
				p = 1;
			} else{
				fprintf(stderr,"Invalid flags\n");
				return 1;
			}

		}	
		encode(maxbits, e, p);

	} else if (strcmp(argv[0], "./decode") == 0){
		if (argc > 1){
			fprintf(stderr,"usage: decode\n");
		} else
			decode();
	} else{
		fprintf(stderr,"Invalid\n");
	}

	return 0;
}
