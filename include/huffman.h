#pragma once
#include <math.h>
/* needs
#include "vector.h"
*/

// used before huffman to measure frequencies
typedef struct char_freq {
    int freq;
    uint8 value;
} char_freq;

// only used in tree construction
typedef struct huff_node {
    char flag;
    int freq;
    int parent;
} huff_node;

typedef struct huff_code {
    uint16 code;
    uint8 bit_count;
} huff_code;

typedef struct huffman {
    size_t total_chars;
    huff_code* codes;
} huffman;

int __char_frequency_sort_comp(const void* a, const void* b)
{
    return ((char_freq*)a)->freq - ((char_freq*)b)->freq;
}

int char_frequency(char* str, int strlen, char_freq* hf)
{
    for (int i=0; i < strlen; i++) {
        hf[(uint8)str[i]].value = str[i];
        hf[(uint8)str[i]].freq++;
    }

    // sort the frequency of bytes from low to high
    qsort(hf, 256, sizeof(char_freq), __char_frequency_sort_comp);

    // return the first non zero frequency byte index
    int i;
    for(i=0; i < 256 && hf[i].freq == 0; i++)
        ;
    return i;
}

int diff_frequency(char* str, int strlen, char_freq* hf)
{
    // count each byte, lineart find
    int j = 0;
    for (int i=0; i < strlen; i++) {

        int pos;
        for(pos=0; pos < j; pos++) {
            if (hf[pos].value == str[i]) {
                hf[pos].freq++;
                break;
            }
        }

        if (pos == j) {
            hf[j].value = str[i];
            hf[j].freq++;
            j++;
        }
    }

    // sort the frequency of bytes from low to high
    qsort(hf, j, sizeof(char_freq), __char_frequency_sort_comp);
    return j;
}


// flag = 0 for sum, 1 for actual char
#define BYTE 1
#define SUM 0

huffman build_huffman_tree(char_freq* hf, int len)
{
    vector tree; vector_init(&tree, sizeof(huff_node), len*2);
    huff_code* encoding = malloc(len*sizeof(huff_code)+1);

    // empty tree in a vector
    for(int i=0; i < len; i++) {
        huff_node hv = {BYTE, hf[i].freq};
        vector_push(&tree, &hv);
    }

    // build tree and insert sums in correct positions, -1 flag for sums
    for(int i=0; i < tree.length-1; i++) {

        huff_node* hn1 = vector_index(&tree, i++);
        huff_node* hn2 = vector_index(&tree, i);
        int sum = hn1->freq + hn2->freq;

        int pos = i+1;
        huff_node* hv = tree.begin;
        while(pos < tree.length && sum >= hv[pos].freq)
            pos++;

        hn1->parent = pos;
        hn2->parent = pos;

        huff_node hn = {SUM, sum};
        vector_insert(&tree, &hn, pos);
    }

    // get huffman code
    huff_node* hv = tree.begin;
    size_t total_chars = 0;
    int j = 0;
    int ROOT = tree.length-1;
    for(int i=0; i < tree.length; i++) {
        // printf("%d: {%d, %d} child of index %d\n", i, hv[i].flag, hv[i].freq, hv[i].parent);
        if(hv[i].flag) {
            int index = i;
            int shift = 0;
            int code = 0;
            total_chars += hv[index].freq;
            do
                code |= (((index & 1)) << shift++);
            while((index = hv[index].parent) != ROOT);
            encoding[j].code = code;
            // assert(shift <= 8 && "huffman cannot be used on this file!");
            encoding[j].bit_count = shift;
            ++j;
        }
    }

    vector_dest(&tree);
    return (huffman){total_chars, encoding};
}

// len=# of unique charcter
void huffman_fset(char* fname, huffman h, char_freq* cf, int cflen, char* str, int strlen)
{
    // main dictionary used to transorm char in str to code in h.codes

    huff_code dict[256];
    for(int i=0; i < cflen; i++)
        dict[cf[i].value] = h.codes[i];

    size_t malloc_size = cflen * 4 + strlen + 1000;
    char* fstr = malloc(malloc_size);

    int i = 0;

    //table size
    fstr[i++] = cflen;
    printf("table size: %d\n", cflen);

    // table
    for(int j=0; j < cflen; j++) {

        for(int k=0; k < 2; k++)
            fstr[i++] = (h.codes[j].code >> (8*k)) & 0x0FF;

        fstr[i++] = h.codes[j].bit_count;
        fstr[i++] = cf[j].value;
    }

    // fewest_bits
    fstr[i++] = h.codes[cflen-1].bit_count;

    // bit count, little endian, 8 bytes
    for(int j=0; j < 8; j++)
        fstr[i++] = (h.total_chars >> (8*j)) & 0x0FF;

    // convertion
    int byte_count = 0;
    int bit_pos = 8, b = 0;

    for(int j=0; j < strlen; j++) {

        int c = *str++;
        huff_code hc = dict[c];
        int bit_count = hc.bit_count;
        int code = hc.code;

        int avail = bit_pos - bit_count;

        // current byte has enough available bits for code bits to fit
        if (avail >= 0) {
            b |= code << avail;
            debug("c: %d, b:  %d in avail >= 0\n", code, b);
            bit_pos -= bit_count;
            if (bit_pos == 0) {

                fstr[i++] = b;
                byte_count++;
                debug("wrote when avail >= 0: %d\n", fstr[i-1]);
                bit_pos = 8;
                b = 0;
            }
        }

        // byte does not have enough room so fit in as many as bits from code as possible
        else  {
            do {
                b |= code >> (bit_count - bit_pos);
                fstr[i++] = b;
                bit_count -= bit_pos;
                bit_pos = 8;
                debug("c: %c, b: %d in bit_pos > 0\n", c, b);
                debug("wrote when bit_pos > 0: %d\n", fstr[i-1]);
                b = 0;
                byte_count++;
            }
            while (bit_count >= bit_pos);
            b |= code << (bit_pos - bit_count);
            bit_pos -= bit_count;
        }
    }

    if (bit_pos != 8) {
        fstr[i++] = b;
        byte_count++;
        debug("wrote when bit_pos != 8: %d\n", fstr[i-1]);
    }

    printf("\nbyte_count: %d, %d writes, malloc_size %d\n\n", byte_count, i, malloc_size);

    fset(fname, fstr, i);
    free(fstr);
}

int bit_range(int val, int s, int f)
{
    int ret = pow(2, s-1); // if (s == 8, then s ^ 7)
    for(int i=1; i < f; i++)
        ret |= ret >> 1;
    val &= ret;
    return val >> (s-f);
}

char* huffman_fget(char* fname, int* len)
{
    uint8* str = (uint8*)fget(fname, len);

    char bit_count[50000][20] = {0};
    char values[50000][20] = {0};
    char length[50000] = {0};
    // getting tablee
    int i = 0;

    int table_size = str[i++];
    printf("table size: %d\n", table_size);

    for(int j=0; j < table_size; j++) {

        int code = str[i++];
        code |= (str[i++] << 8); // code
        int len = length[code];

        bit_count[code][len] = str[i++]; // bit_count
        values[code][len] = str[i++]; //value
        length[code]++;

        printf("code: %d ", code);
        printf("bit_count: %d ", bit_count[code][len]);
        printf("Val: %c\n", values[code][len]);

    }

    // get fewest_bits
    int fewest_bits = str[i++];
    printf("fewest_bits: %d\n", fewest_bits);

    // get 8 bytes total-bits
    size_t total_chars=0;
    for(int j=0; j < 8; j++)
        total_chars |= str[i++] << (j*8);
    printf("total_chars %lu\n\n", (unsigned long)total_chars);

    char* result = malloc(total_chars + 1);

    int bit_pos = 8;
    int c = str[i++];

    for(int j=0; j < total_chars; j++) {

        int b = 0, m = 0;
        int cbc = fewest_bits-1; // current bit count, offset from bit_pos

        do {
            ++cbc;
            if (bit_pos >= cbc) {
                b = bit_range(c, bit_pos, cbc);
                debug("in bit_pos >= cbc! b: %d, c: %d, cbc: %d, bit_pos: %d\n", b, c, cbc, bit_pos);
            }
            else if (bit_pos > 0) {
                b = bit_range(c, bit_pos, bit_pos);
                debug("b: %d, c: %d, cbc: %d, bit_pos: %d\n", b, c, cbc, bit_pos);
                c =  (b << 8) | str[i++];
                cbc = bit_pos;
                bit_pos += 8;
                debug("b: %d, c: %d, cbc: %d, bit_pos: %d\n", b, c, cbc, bit_pos);

            }
            else {
                debug("blah........\n");
                bit_pos = 8;
                c = str[i++];
                cbc = fewest_bits-1;
            }

            for(m=0; m < length[b] && bit_count[b][m] != cbc; m++)
                ;
        }
        while (m == length[b] && cbc < 10);

        bit_pos -= cbc;
        debug("output %c\n", values[b][m]);
        result[j] = values[b][m];
    }
    result[total_chars] = '\0';
    *len = total_chars;
    return result;
}
