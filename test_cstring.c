#include "cstring.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


int main(void)
{
    struct string *s = NULL;
    struct string *sub = NULL;
    char *str = NULL;

    string_append_buffer(NULL, "foo", 1);
    string_append_c_str(NULL, "foo");
    string_append_fill(NULL, 3, 'f');
    assert(0 == string_at(NULL, 0));
    assert(0 == string_back(NULL));
    assert(NULL == string_c_str(NULL));
    assert(NULL == string_c_str_move(NULL));
    assert(0 == string_capacity(NULL));
    string_clear(NULL);
    string_delete(NULL);
    assert(string_empty(NULL));
    string_erase(NULL, 1, 2);
    string_insert_buffer(NULL, 1, "foo", 1);
    string_insert_c_str(NULL, 1, "foo");
    string_insert_fill(NULL, 1, 3, 'f');
    string_pop_back(NULL);
    string_push_back(NULL, 'f');
    string_reserve(NULL, 8);
    assert(0 == string_size(NULL));
    s = string_substr(NULL, 2, 3);
    assert(s);
    assert(string_empty(s));
    string_delete(s);

    string_allocator_set(realloc, free);

    s = string_new();

    assert(string_empty(s));
    assert(0 == *string_c_str(s));
    assert(0 == string_size(s));
    assert('\0' == string_back(s));

    string_push_back(s, 0);
    assert(0 == *string_c_str(s));
    assert(1 == string_size(s));
    assert('\0' == string_back(s));
    string_clear(s);

    string_append_fill(s, 4, 'x');
    assert(4 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "xxxx"));

    string_insert_buffer(s, 2, "abcdef", 3);
    assert(7 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "xxabcxx"));

    string_insert_c_str(s, 7, "z");
    assert(8 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "xxabcxxz"));

    string_insert_fill(s, 0, 3, 'y');
    assert(11 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "yyyxxabcxxz"));

    string_clear(s);
    string_append_c_str(s, "one");
    assert(!string_empty(s));
    assert(NULL != string_c_str(s));
    assert(3 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "one"));

    str = string_c_str_move(s);
    assert(0 == strcmp(str, "one"));
    free(str);
    assert(string_empty(s));
    assert(NULL == string_c_str(s));
    assert(0 == string_size(s));

    string_append_fill(s, 1, 'o');
    string_append_fill(s, 3, 'n');
    string_append_fill(s, 1, 'e');
    assert(!string_empty(s));
    assert(NULL != string_c_str(s));
    assert(5 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "onnne"));

    assert('o' == string_at(s, 0));
    assert('n' == string_at(s, 1));
    assert('e' == string_at(s, 4));
    assert(0 == string_at(s, 5));
    assert(0 == string_at(s, 9));

    string_clear(s);
    string_insert_fill(s, 0, 1, 'O');
    string_insert_fill(s, 1, 3, 'N');
    string_insert_fill(s, 4, 1, 'E');
    assert(5 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "ONNNE"));

    string_erase(s, 5, 99);
    assert(5 == string_size(s));
    string_erase(s, 4, 99);
    assert(4 == string_size(s));
    string_erase(s, 0, 0);
    assert(4 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "ONNN"));
    string_erase(s, 0, 1);
    assert(3 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "NNN"));

    string_append_c_str(s, "abc");
    assert(6 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "NNNabc"));

    sub = string_substr(s, 3, 0);
    assert(string_empty(sub));
    assert(0 == *string_c_str(sub));
    string_delete(sub);
    sub = string_substr(s, 6, 0);
    assert(string_empty(sub));
    assert(0 == *string_c_str(sub));
    string_delete(sub);
    sub = string_substr(s, 6, 1);
    assert(string_empty(sub));
    assert(0 == *string_c_str(sub));
    string_delete(sub);
    sub = string_substr(s, 5, 1);
    assert(string_size(sub) == 1);
    assert(0 == strcmp(string_c_str(sub), "c"));
    string_delete(sub);
    sub = string_substr(s, 5, 2);
    assert(string_size(sub) == 1);
    assert(0 == strcmp(string_c_str(sub), "c"));
    string_delete(sub);
    sub = string_substr(s, 2, 1);
    assert(0 == strcmp(string_c_str(sub), "N"));
    string_delete(sub);
    sub = string_substr(s, 2, 2);
    assert(0 == strcmp(string_c_str(sub), "Na"));
    string_delete(sub);

    string_append_buffer(s, "defghi", 3);
    assert('f' == string_back(s));
    assert(9 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "NNNabcdef"));

    string_push_back(s, 'g');
    assert('g' == string_back(s));
    assert(10 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "NNNabcdefg"));

    string_pop_back(s);
    assert(0 == strcmp(string_c_str(s), "NNNabcdef"));

    assert(10 == string_capacity(s));
    string_reserve(s, 3);
    assert(10 == string_capacity(s));

    string_reserve(s, 20);
    assert(20 == string_capacity(s));
    assert(0 == strcmp(string_c_str(s), "NNNabcdef"));

    string_delete(s);

    s = string_new();

    assert(0 == *string_c_str(s));
    string_reserve(s, 0);
    assert(0 == *string_c_str(s));
    string_reserve(s, 1);
    assert(0 == *string_c_str(s));

    string_delete(s);
}
