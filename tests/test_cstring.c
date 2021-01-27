#include "cstring.h"

#include "memory_shim.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/// @return True if string contains expected content, false otherwise.
static bool verify_string_content(const struct string *s, const char *expected)
{
    const char *actual;

    assert(s);
    assert(expected);
    actual = string_c_str(s);
    return actual && strcmp(actual, expected) == 0;
}

static void test_string_new(void)
{
    struct string *s = NULL;

    g_calloc_fail = true;
    s = string_new();
    g_calloc_fail = false;
    assert(NULL == s);
}

static void test_string_delete(void)
{
    struct string *s = NULL;

    string_delete(NULL);

    s = string_new();

    string_delete(s);
}

static void test_string_empty(void)
{
    struct string *s = NULL;

    assert(string_empty(NULL));

    s = string_new();

    assert(string_empty(s));

    assert(0 == string_append_c_str(s, "a"));
    assert(!string_empty(s));

    string_delete(s);
}

static void test_string_size(void)
{
    assert(0 == string_size(NULL));
}

static void test_string_reserve(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_reserve(NULL, 8));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_reserve(s, SIZE_MAX));
    assert(ENOMEM == errno);

    g_malloc_fail = true;
    errno = 0;
    assert(-1 == string_reserve(s, 1));
    assert(ENOMEM == errno);
    g_malloc_fail = false;

    assert(0 == *string_c_str(s));

    assert(0 == string_reserve(s, 0));
    assert(0 == *string_c_str(s));

    assert(0 == string_reserve(s, 1));
    assert(0 == *string_c_str(s));

    assert(0 == string_reserve(s, 20));
    assert(20 == string_capacity(s));
    assert(0 == string_size(s));
    string_append_c_str(s, "abcdef");
    assert(20 == string_capacity(s));
    assert(6 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "abcdef"));

    assert(0 == string_reserve(s, 10));
    assert(10 == string_capacity(s));
    assert(6 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "abcdef"));

    assert(0 == string_reserve(s, 3));
    assert(10 == string_capacity(s));
    assert(6 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "abcdef"));

    assert(0 == string_reserve(s, 6));
    assert(6 == string_capacity(s));
    assert(6 == string_size(s));
    assert(0 == strcmp(string_c_str(s), "abcdef"));

    string_delete(s);
}

static void test_string_capacity(void)
{
    struct string *s = NULL;
    const struct string *cs = NULL;

    assert(0 == string_capacity(NULL));

    s = string_new();

    cs = s;
    // Small string optimization.
    assert(7 == string_capacity(cs));

    assert(0 == string_append_c_str(s, "123"));
    cs = s;
    assert(7 == string_capacity(cs));

    // Double on growth.
    assert(0 == string_append_c_str(s, "45678"));
    cs = s;
    assert(14 == string_capacity(cs));

    string_delete(s);
}

static void test_string_at(void)
{
    struct string *s = NULL;
    const struct string *cs = NULL;

    assert(0 == string_at(NULL, 0));

    s = string_new();

    assert(0 == string_at(s, 0));
    assert(0 == string_at(s, 1));

    assert(0 == string_append_c_str(s, "a"));

    cs = s;
    assert('a' == string_at(cs, 0));
    assert(0 == string_at(cs, 1));

    string_delete(s);
}

static void test_string_c_str(void)
{
    struct string *s = NULL;
    const char *unsafe1;
    const char *unsafe2;

    assert(NULL == string_c_str(NULL));

    s = string_new();

    unsafe1 = string_c_str(s);
    assert(NULL != unsafe1);
    // No reallocation due to small string optimization.
    string_push_back(s, 'a');
    unsafe2 = string_c_str(s);
    assert(unsafe2 == unsafe1);

    // Reallocation causes `unsafe1' to become dangling.
    string_append_c_str(s, "defeat sso");
    unsafe2 = string_c_str(s);
    assert(unsafe2 != unsafe1);

    string_delete(s);
}

static void test_string_c_str_move(void)
{
    struct string *s = NULL;
    char *str = NULL;

    errno = 0;
    assert(NULL == string_c_str_move(NULL));
    assert(EFAULT == errno);

    s = string_new();

    g_strdup_fail = true;
    errno = 0;
    str = string_c_str_move(s);
    assert(ENOMEM == errno);
    g_strdup_fail = false;

    assert(0 == strcmp(string_c_str(s), ""));
    str = string_c_str_move(s);
    assert(0 == strcmp(str, ""));
    free(str);

    assert(0 == strcmp(string_c_str(s), ""));
    str = string_c_str_move(s);
    assert(0 == strcmp(str, ""));
    free(str);

    assert(0 == string_append_c_str(s, "abcde"));
    str = string_c_str_move(s);
    assert(0 == strcmp(str, "abcde"));
    assert(7 == string_capacity(s));
    free(str);
    assert(0 == strcmp(string_c_str(s), ""));

    assert(0 == string_append_c_str(s, "abcde"));
    assert(7 == string_capacity(s));
    assert(0 == string_append_c_str(s, "fgh"));
    assert(14 == string_capacity(s));
    str = string_c_str_move(s);
    assert(0 == strcmp(str, "abcdefgh"));
    assert(7 == string_capacity(s));
    free(str);
    assert(0 == strcmp(string_c_str(s), ""));

    string_delete(s);
}

static void test_string_clear(void)
{
    struct string *s = NULL;

    string_clear(NULL);

    s = string_new();

    string_clear(s);

    string_delete(s);
}

static void test_string_insert_buffer(void)
{
    // Gain access to string internals for test purposes.
    struct test_string {
        size_t cap;
        size_t len;
        char *buf;
        char sso[8];
    };

    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_insert_buffer(NULL, 1, 3, "foo"));
    assert(EFAULT == errno);

    s = string_new();

    // Test compute_growth() overflow when cap cannot be doubled.
    ((struct test_string *)s)->cap = SIZE_MAX / 2 + 1;
    ((struct test_string *)s)->len = SIZE_MAX / 2 + 1;
    g_malloc_fail = true;
    errno = 0;
    assert(-1 == string_insert_buffer(s, 0, 1, "a"));
    assert(ENOMEM == errno);
    g_malloc_fail = false;
    ((struct test_string *)s)->cap = 0;
    ((struct test_string *)s)->len = 0;

    errno = 0;
    assert(-1 == string_insert_buffer(s, 1, 3, NULL));
    assert(EFAULT == errno);
    assert(0 == strcmp(string_c_str(s), ""));

    errno = 0;
    assert(-1 == string_insert_buffer(s, 1, 3, "abcDEF"));
    assert(ERANGE == errno);

    assert(0 == string_insert_buffer(s, 0, 3, "abcDEF"));
    assert(0 == strcmp(string_c_str(s), "abc"));

    errno = 0;
    assert(-1 == string_insert_buffer(s, 1, SIZE_MAX, "XYZ"));
    assert(ENOMEM == errno);

    assert(0 == string_insert_buffer(s, 3, 0, "abcDEF"));
    assert(0 == strcmp(string_c_str(s), "abc"));

    assert(0 == string_insert_buffer(s, 3, 1, "dEF"));
    assert(0 == strcmp(string_c_str(s), "abcd"));

    assert(0 == string_insert_buffer(s, 2, 20, "12345678901234567890X"));
    assert(0 == strcmp(string_c_str(s), "ab12345678901234567890cd"));

    string_delete(s);
}

static void test_string_insert_c_str(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_insert_c_str(NULL, 1, "foo"));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_insert_c_str(s, 1, NULL));
    assert(EFAULT == errno);
    assert(0 == strcmp(string_c_str(s), ""));

    errno = 0;
    assert(-1 == string_insert_c_str(s, 1, "abc"));
    assert(ERANGE == errno);

    assert(0 == string_insert_c_str(s, 0, "abc"));
    assert(0 == strcmp(string_c_str(s), "abc"));

    assert(0 == string_insert_c_str(s, 3, ""));
    assert(0 == strcmp(string_c_str(s), "abc"));

    assert(0 == string_insert_c_str(s, 3, "d"));
    assert(0 == strcmp(string_c_str(s), "abcd"));

    assert(0 == string_insert_c_str(s, 2, "x"));
    assert(0 == strcmp(string_c_str(s), "abxcd"));

    string_delete(s);
}

static void test_string_insert_fill(void)
{
    struct string *s = NULL;

    string_insert_fill(s, 0, 3, 'y');

    errno = 0;
    assert(-1 == string_insert_fill(NULL, 0, 3, 'f'));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_insert_fill(s, 1, 1, 'o'));
    assert(ERANGE == errno);
    assert(0 == strcmp(string_c_str(s), ""));

    assert(0 == string_insert_fill(s, 0, 1, 'o'));
    assert(0 == strcmp(string_c_str(s), "o"));

    assert(0 == string_insert_fill(s, 0, 0, 'X'));
    assert(0 == strcmp(string_c_str(s), "o"));

    g_malloc_fail = true;
    errno = 0;
    assert(-1 == string_insert_fill(s, 1, 20, 'X'));
    assert(ENOMEM == errno);
    g_malloc_fail = false;

    assert(0 == string_insert_fill(s, 1, 3, 'n'));
    assert(0 == strcmp(string_c_str(s), "onnn"));

    assert(0 == string_insert_fill(s, 4, 1, 'e'));
    assert(0 == strcmp(string_c_str(s), "onnne"));

    assert(0 == string_insert_fill(s, 2, 1, 'x'));
    assert(0 == strcmp(string_c_str(s), "onxnne"));

    string_delete(s);
}

static void test_string_erase(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_erase(NULL, 1, 2));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_erase(s, 1, 2));
    assert(ERANGE == errno);
    assert(string_empty(s));

    assert(0 == string_erase(s, 0, 2));
    assert(string_empty(s));

    assert(0 == string_append_c_str(s, "abcdefghi"));
    assert(0 == string_erase(s, 5, 99));
    assert(0 == strcmp(string_c_str(s), "abcde"));

    assert(0 == string_erase(s, 4, 99));
    assert(0 == strcmp(string_c_str(s), "abcd"));

    assert(0 == string_erase(s, 0, 0));
    assert(0 == strcmp(string_c_str(s), "abcd"));

    assert(0 == string_erase(s, 0, 1));
    assert(0 == strcmp(string_c_str(s), "bcd"));

    assert(0 == string_erase(s, 1, 1));
    assert(0 == strcmp(string_c_str(s), "bd"));

    assert(0 == string_erase(s, 1, 1));
    assert(0 == strcmp(string_c_str(s), "b"));

    assert(0 == string_erase(s, 1, 1));
    assert(0 == strcmp(string_c_str(s), "b"));

    string_delete(s);
}

static void test_string_push_back(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_push_back(NULL, 'f'));
    assert(EFAULT == errno);

    s = string_new();

    assert(0 == string_push_back(s, 0));
    assert(0 == *string_c_str(s));
    assert(1 == string_size(s));

    assert(0 == string_push_back(s, 0));
    assert(0 == *string_c_str(s));
    assert(2 == string_size(s));

    string_delete(s);
}

static void test_string_pop_back(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_pop_back(NULL));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_pop_back(s));
    assert(ERANGE == errno);

    assert(0 == string_insert_c_str(s, 0, "abc"));
    assert(0 == strcmp(string_c_str(s), "abc"));

    assert(0 == string_pop_back(s));
    assert(0 == strcmp(string_c_str(s), "ab"));

    assert(0 == string_pop_back(s));
    assert(0 == strcmp(string_c_str(s), "a"));

    assert(0 == string_pop_back(s));
    assert(0 == strcmp(string_c_str(s), ""));

    errno = 0;
    assert(-1 == string_pop_back(s));
    assert(ERANGE == errno);

    string_delete(s);
}

static void test_string_append_buffer(void)
{
    struct string *s = NULL;
    const struct string *cs = NULL;

    errno = 0;
    assert(-1 == string_append_buffer(NULL, 3, "foo"));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_append_buffer(s, 3, NULL));
    assert(EFAULT == errno);
    cs = s;
    assert(verify_string_content(cs, ""));

    assert(0 == string_append_buffer(s, 3, "abcDEF"));
    cs = s;
    assert(verify_string_content(cs, "abc"));

    errno = 0;
    assert(-1 == string_append_buffer(s, SIZE_MAX, "XYZ"));
    assert(ENOMEM == errno);

    g_malloc_fail = true;
    errno = 0;
    assert(-1 == string_append_buffer(s, 20, "12345678901234567890"));
    assert(ENOMEM == errno);
    g_malloc_fail = false;

    assert(0 == string_append_buffer(s, 0, "ABCDEF"));
    cs = s;
    assert(verify_string_content(cs, "abc"));

    assert(0 == string_append_buffer(s, 1, "dEF"));
    cs = s;
    assert(verify_string_content(cs, "abcd"));

    string_delete(s);
}

static void test_string_append_c_str(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_append_c_str(NULL, "foo"));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(-1 == string_append_c_str(s, NULL));
    assert(EFAULT == errno);
    assert(0 == strcmp(string_c_str(s), ""));

    assert(0 == string_append_c_str(s, "abc"));
    assert(0 == strcmp(string_c_str(s), "abc"));

    assert(0 == string_append_c_str(s, ""));
    assert(0 == strcmp(string_c_str(s), "abc"));

    assert(0 == string_append_c_str(s, "d"));
    assert(0 == strcmp(string_c_str(s), "abcd"));

    string_delete(s);
}

static void test_string_append_fill(void)
{
    struct string *s = NULL;

    errno = 0;
    assert(-1 == string_append_fill(NULL, 3, 'f'));
    assert(EFAULT == errno);

    s = string_new();

    assert(0 == string_append_fill(s, 1, 'o'));
    assert(0 == strcmp(string_c_str(s), "o"));

    assert(0 == string_append_fill(s, 0, 'X'));
    assert(0 == strcmp(string_c_str(s), "o"));

    string_reserve(s, 10);

    g_realloc_fail = true;
    errno = 0;
    assert(-1 == string_append_fill(s, 20, 'X'));
    assert(ENOMEM == errno);
    g_realloc_fail = false;

    assert(0 == string_append_fill(s, 3, 'n'));
    assert(0 == strcmp(string_c_str(s), "onnn"));

    assert(0 == string_append_fill(s, 1, 'e'));
    assert(0 == strcmp(string_c_str(s), "onnne"));

    string_delete(s);
}

static void test_string_substr(void)
{
    struct string *s = NULL;
    struct string *sub = NULL;

    errno = 0;
    assert(NULL == string_substr(NULL, 2, 3));
    assert(EFAULT == errno);

    s = string_new();

    errno = 0;
    assert(NULL == string_substr(s, 2, 3));
    assert(ERANGE == errno);

    assert(0 == string_append_c_str(s, "abcd"));

    sub = string_substr(s, string_size(s), 0);
    assert(0 == *string_c_str(sub));
    string_delete(sub);

    sub = string_substr(s, string_size(s) - 3, 0);
    assert(0 == *string_c_str(sub));
    string_delete(sub);

    sub = string_substr(s, string_size(s) - 3, 1);
    assert(0 == strcmp(string_c_str(sub), "b"));
    string_delete(sub);

    sub = string_substr(s, string_size(s) - 3, 10);
    assert(0 == strcmp(string_c_str(sub), "bcd"));
    string_delete(sub);

    sub = string_substr(s, 2, SIZE_MAX);
    assert(0 == strcmp(string_c_str(sub), "cd"));
    string_delete(sub);

    sub = string_substr(s, 2, 3);
    assert(0 == strcmp(string_c_str(sub), "cd"));
    string_delete(sub);

    assert(0 == string_append_c_str(s, "e"));

    g_calloc_fail = true;
    errno = 0;
    sub = string_substr(s, 2, 3);
    g_calloc_fail = false;
    assert(NULL == sub);
    assert(ENOMEM == errno);

    assert(0 == string_append_c_str(s, "fghijklmnopqrstuvwxyz"));

    g_malloc_fail = true;
    errno = 0;
    sub = string_substr(s, 2, 20);
    g_malloc_fail = false;
    assert(NULL == sub);
    assert(ENOMEM == errno);

    sub = string_substr(s, 2, 3);
    assert(NULL != sub);
    assert(0 == strcmp(string_c_str(sub), "cde"));
    string_delete(sub);

    sub = string_substr(s, 3, 0);
    assert(string_empty(sub));
    assert(0 == *string_c_str(sub));
    string_delete(sub);

    string_delete(s);
}

int main(void)
{
    test_string_new();
    test_string_delete();
    test_string_empty();
    test_string_size();
    test_string_reserve();
    test_string_capacity();
    test_string_at();
    test_string_c_str();
    test_string_c_str_move();
    test_string_clear();
    test_string_insert_buffer();
    test_string_insert_c_str();
    test_string_insert_fill();
    test_string_erase();
    test_string_push_back();
    test_string_pop_back();
    test_string_append_buffer();
    test_string_append_c_str();
    test_string_append_fill();
    test_string_substr();
    return 0;
}
