LZW
===

This is an implementation of the Lempel-Ziv Welch Algorithm, as described in the paper, http://www.cs.duke.edu/courses/spring03/cps296.5/papers/welch_1984_technique_for.pdf. This enables compression of large streams of input (which can be redirected in and out of files via linux) through the **encode** executable, and decompression of these compressed streams through the **decode** executable. There are two flags readily available on the encode function (which decode will be able to decipher): the -e flag (provides an empty trie, devoid of 1-character strings, which will be added to the data structure using an escape code), the -p flag (provides the pruning function, which will remove some of the lesser used codes from the data structure). Both of these flags will make lzw an even better compression algorithm.

Lzw.c provides the code for the encode and decode functions/executables. Trie.c (and Trie.h) provides the array/trie-hybrid data structure that the lzw algorithm implements. code.c (and code.h), which have been provided, handle the output of bits from encode and decode. Attached is the makefile as well.
