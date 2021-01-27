#include "cstring.h"

#include <stdlib.h>
#include <string.h>


struct string {
    size_t cap;
    size_t len;
    char *buf;
};

static struct {
    void *(*realloc)(void *, size_t);
    void (*dealloc)(void *);
} mem = {
    realloc,
    free
};


void string_allocator_set(
        void *(*realloc)(void *, size_t),
        void (*dealloc)(void *))
{
    mem.realloc = realloc;
    mem.dealloc = dealloc;
}

struct string *string_new(void)
{
    struct string *str = mem.realloc(NULL, sizeof(struct string));
    str->cap = 0;
    str->len = 0;
    str->buf = NULL;
    return str;
}

void string_delete(struct string *str)
{
    mem.dealloc(str->buf);
    mem.dealloc(str);
}

bool string_empty(struct string const *str)
{
    return string_size(str) == 0;
}

size_t string_size(struct string const *str)
{
    return str->len;
}

void string_reserve(struct string *str, size_t cap)
{
    if (str->cap < cap) {
        str->cap = cap;
        str->buf = mem.realloc(str->buf, str->cap + 1);
        if (str->len == 0) {
            str->buf[0] = 0;
        }
    }
}

size_t string_capacity(struct string const *str)
{
    return str->cap;
}

void string_clear(struct string *str)
{
    str->len = 0;
    if (str->buf) {
        str->buf[0] = 0;
    }
}

char string_at(struct string *str, size_t pos)
{
    if (pos >= str->len) {
#ifdef UNITTEST_CSTRING
        return 0;
#else
        abort();
#endif
    }
    return str->buf[pos];
}

void string_assign_c_str(struct string *str, const char *s)
{
    size_t len = strlen(s);
    str->cap = len;
    str->len = len;
    str->buf = mem.realloc(str->buf, str->cap + 1);
    strcpy(str->buf, s);
}

static char *impl_append(struct string *str, size_t n)
{
    size_t pos = str->len;
    size_t len_ = str->len + n;
    if (len_ > str->cap) {
        str->cap = len_;
        str->buf = mem.realloc(str->buf, str->cap + 1);
    }

    str->buf[pos + n] = 0;

    str->len += n;
    return &str->buf[pos];
}

void string_append_c_str(struct string *str, const char *s)
{
    if (s && *s) {
        strcpy(impl_append(str, strlen(s)), s);
    }
}

void string_append_buffer(struct string *str, const char *s, size_t n)
{
    memcpy(impl_append(str, n), s, n);
}

void string_append_fill(struct string *str, size_t n, char c)
{
    memset(impl_append(str, n), c, n);
}

static char *impl_insert(struct string *str, size_t pos, size_t n)
{
    size_t len_ = str->len + n;
    if (len_ > str->cap) {
        str->cap = len_;
        str->buf = mem.realloc(str->buf, str->cap + 1);
    }

    if (pos < str->len) {
        size_t rhs = str->len - pos;
        memmove(&str->buf[pos + n],
                &str->buf[pos],
                rhs + 1);
    } else {
        str->buf[pos + n] = 0;
    }

    str->len += n;
    return &str->buf[pos];
}

void string_insert_c_str(struct string *str, size_t pos, const char *s)
{
    size_t n = strlen(s);
    memcpy(impl_insert(str, pos, n), s, n);
}

void string_insert_buffer(struct string *str, size_t pos, const char *s, size_t n)
{
    memcpy(impl_insert(str, pos, n), s, n);
}

void string_insert_fill(struct string *str, size_t pos, size_t n, char c)
{
    memset(impl_insert(str, pos, n), c, n);
}

#include <stdio.h>

void string_erase(struct string *str, size_t pos, size_t len)
{
    /*
     *    rhs
     *   /--------------\
     *    len   n
     *   /----\/--------\
     * --+--+--+--+--+--+
     *   |  |  |  |  |  |
     * --+--+--+--+--+--+
     *    ^              ^
     *    pos            str->len
     */
    if (pos < str->len) {
        size_t rhs = str->len - pos;
        size_t n;
        if (len > rhs) {
            len = rhs;
        }
        n = rhs - len;
        memmove(&str->buf[pos],
                &str->buf[pos + len],
                n + 1);
        str->len -= len;
    }
}

void string_push_back(struct string *str, char c)
{
    string_append_fill(str, 1, c);
}

void string_pop_back(struct string *str)
{
    if (str->len > 0) {
        str->buf[--str->len] = 0;
    }
}

const char *string_c_str(struct string const *str)
{
    return str->buf;
}

char *string_c_str_move(struct string *str)
{
    char *buf = str->buf;
    str->cap = 0;
    str->len = 0;
    str->buf = NULL;
    return buf;
}


#ifdef UNITTEST_CSTRING

#include <assert.h>
#include <stdlib.h>


//@unittest clang -g -fsanitize=address -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -Weverything -Werror -Wno-padded -Wno-poison-system-directories -DUNITTEST_CSTRING -o cstring.exe
//@exe cstring.exe
int main(void)
{
    struct string *s;
    char *str;

    string_allocator_set(realloc, free);

    s = string_new();

    assert(string_empty(s));
    assert(0 == string_size(s));

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
    string_assign_c_str(s, "one");
    assert(!string_empty(s));
    assert(3 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "one"));

    str = string_c_str_move(s);
    assert(0 == strcmp(str, "one"));
    free(str);
    assert(string_empty(s));
    assert(0 == string_size(s));

    string_append_fill(s, 1, 'o');
    string_append_fill(s, 3, 'n');
    string_append_fill(s, 1, 'e');
    assert(!string_empty(s));
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

    string_append_buffer(s, "defghi", 3);
    assert(9 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "NNNabcdef"));

    string_push_back(s, 'g');
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

    string_reserve(s, 0);
    assert(0 == string_c_str(s));
    string_reserve(s, 1);
    assert(0 == strcmp(string_c_str(s), ""));

    string_delete(s);
}

#endif
