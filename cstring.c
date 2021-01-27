#include "cstring.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/// Growth factor for capacity when resizing.
/// 2.0x balances memory overhead (~50% extra on average after growth) with reallocation frequency.
/// See Herb Sutter's "Allocators" article for analysis of growth factors.
#define STRING_GROWTH_FACTOR 2

/// Small string optimization.
/// Holds up to 7 chars + NUL inline.
/// Arbitrary choice.
#define SSO_SIZE 8

struct string {
    /// Capacity of buffer (excluding NUL terminator).
    size_t cap;
    /// Length of buffer (excluding NUL terminator).
    size_t len;
    /// Buffer (always NUL terminated).
    char *buf;
    /// Internal storage (small string optimization).
    char sso[SSO_SIZE];
};

#define SSO_CAPACITY (sizeof(((struct string *)0)->sso) - 1 /* Space for NUL */)

struct string *string_new(void)
{
    struct string *str = NULL;

    str = calloc(1, sizeof(struct string));
    if (!str) {
        errno = ENOMEM;
        return NULL;
    }

    str->buf = str->sso;
    str->cap = SSO_CAPACITY;
    return str;
}

static bool internal_storage_used(const struct string *str)
{
    // Precondition.
    assert(str);
    return str->buf == str->sso;
}

void string_delete(struct string *str)
{
    if (!str) {
        return;
    }

    if (!internal_storage_used(str)) {
        free(str->buf);
    }
    str->buf = NULL;
    free(str);
}

bool string_empty(const struct string *str)
{
    return string_size(str) == 0;
}

size_t string_size(const struct string *str)
{
    if (!str) {
        return 0;
    }

    return str->len;
}

int string_reserve(struct string *str, size_t cap)
{
    char *buf;
    bool is_sso;

    if (!str) {
        return -EFAULT;
    }

    if (cap == SIZE_MAX) {
        // Cannot allocate enough memory to hold NUL terminator.
        return -ENOMEM;
    }

    if (cap < str->len) {
        return 0;
    }

    is_sso = internal_storage_used(str);
    if (is_sso) {
        buf = malloc(cap + 1);
    } else {
        buf = realloc(str->buf, cap + 1);
    }

    if (!buf) {
        return -ENOMEM;
    }

    if (is_sso) {
        memcpy(buf, str->sso, str->len + 1);
    }

    str->cap = cap;
    str->buf = buf;
    str->buf[str->len] = 0;
    return 0;
}

size_t string_capacity(const struct string *str)
{
    if (!str) {
        return 0;
    }

    return str->cap;
}

char string_at(const struct string *str, size_t pos)
{
    if (!str) {
        return 0;
    }

    if (pos >= str->len) {
        return 0;
    }

    return str->buf[pos];
}

const char *string_c_str(const struct string *str)
{
    if (!str) {
        errno = EFAULT;
        return NULL;
    }

    return str->buf;
}

char *string_c_str_move(struct string *str)
{
    char *buf;

    if (!str) {
        errno = EFAULT;
        return NULL;
    }

    if (internal_storage_used(str)) {
        // Duplicate internal storage.
        buf = strdup(str->sso);
        if (!buf) {
            errno = ENOMEM;
            return NULL;
        }

    } else {
        // Detach allocated buffer.
        buf = str->buf;
    }

    str->buf = str->sso;
    str->cap = SSO_CAPACITY;
    str->len = 0;
    str->buf[0] = 0;
    return buf;
}

void string_clear(struct string *str)
{
    if (!str) {
        return;
    }

    str->len = 0;
    str->buf[0] = 0;
}

/// Avoid performance issues with repeated small appends.
/// @return New capacity to reserve.
static size_t compute_growth(size_t current, size_t required)
{
    size_t doubled;

    // Precondition.
    assert(required > 0);

    if (current == 0) {
        return required;
    }

    if (current > SIZE_MAX / STRING_GROWTH_FACTOR) {
        // Doubling would overflow SIZE_MAX; grow to exact required size instead.
        return required;
    }

    doubled = current * STRING_GROWTH_FACTOR;
    return (doubled > required) ? doubled : required;
}

/// Insert @c n characters at position @c pos.
/// @return Pointer to the start of the inserted area on success, NULL on failure.
static char *impl_insert(struct string *str, size_t pos, size_t n)
{
    size_t required;
    char *dest;

    // Precondition.
    assert(str);

    if (n > SIZE_MAX - str->len) {
        // Check for overflow.
        errno = ENOMEM;
        return NULL;
    }

    required = str->len + n;
    if (required > str->cap) {
        int r = string_reserve(str, compute_growth(str->cap, required));
        if (r < 0) {
            return NULL;
        }
    }

    dest = &str->buf[pos];

    if (pos < str->len) {
        size_t rhs = str->len - pos;

        memmove(&str->buf[pos + n],
                &str->buf[pos],
                rhs + 1);
    } else {
        str->buf[pos + n] = 0;
    }

    str->len += n;
    return dest;
}

/// Insert buffer @c s of length @c n at position @c pos.
/// @return Zero on success, negative errno otherwise.
static int impl_insert_buffer(struct string *str, size_t pos, size_t n, const char *s)
{
    char *dest;

    // Preconditions.
    assert(str);
    assert(s);

    dest = impl_insert(str, pos, n);
    if (!dest) {
        return -errno;
    }

    memmove(dest, s, n);
    return 0;
}

/// Insert @c n copies of character @c c at position @c pos.
/// @return Zero on success, negative errno otherwise.
static int impl_insert_fill(struct string *str, size_t pos, size_t n, char c)
{
    char *dest;

    // Precondition.
    assert(str);

    dest = impl_insert(str, pos, n);
    if (!dest) {
        return -errno;
    }

    memset(dest, c, n);
    return 0;
}

int string_insert_buffer(struct string *str, size_t pos, size_t n, const char *s)
{
    if (!str) {
        return -EFAULT;
    }

    if (!s) {
        return -EFAULT;
    }

    if (pos > str->len) {
        return -ERANGE;
    }

    return impl_insert_buffer(str, pos, n, s);
}

int string_insert_c_str(struct string *str, size_t pos, const char *s)
{
    if (!str) {
        return -EFAULT;
    }

    if (!s) {
        return -EFAULT;
    }

    if (pos > str->len) {
        return -ERANGE;
    }

    return impl_insert_buffer(str, pos, strlen(s), s);
}

int string_insert_fill(struct string *str, size_t pos, size_t n, char c)
{
    if (!str) {
        return -EFAULT;
    }

    if (pos > str->len) {
        return -ERANGE;
    }

    return impl_insert_fill(str, pos, n, c);
}

int string_erase(struct string *str, size_t pos, size_t len)
{
    size_t rhs;
    size_t n;

    if (!str) {
        return -EFAULT;
    }

    if (pos > str->len) {
        return -ERANGE;
    }

    // For consistency with string_insert_*().
    if (pos == str->len) {
        return 0;
    }

    if (len == 0) {
        return 0;
    }

    //    rhs
    //   /--------------\
    //    len   n
    //   /----\/--------\
    // --+--+--+--+--+--+
    //   |  |  |  |  |  |
    // --+--+--+--+--+--+
    //    ^              ^
    //    pos            str->len
    rhs = str->len - pos;
    n = 0;

    if (len > rhs) {
        // Erase as many as possible.
        len = rhs;
    }

    n = rhs - len;

    memmove(&str->buf[pos],
            &str->buf[pos + len],
            n + 1);

    str->len -= len;
    return 0;
}

int string_push_back(struct string *str, char c)
{
    if (!str) {
        return -EFAULT;
    }

    return impl_insert_fill(str, str->len, 1, c);
}

int string_pop_back(struct string *str)
{
    if (!str) {
        return -EFAULT;
    }

    if (str->len < 1) {
        return -ERANGE;
    }

    str->buf[--str->len] = 0;
    return 0;
}

int string_append_buffer(struct string *str, size_t n, const char *s)
{
    if (!str) {
        return -EFAULT;
    }

    if (!s) {
        return -EFAULT;
    }

    return impl_insert_buffer(str, str->len, n, s);
}

int string_append_c_str(struct string *str, const char *s)
{
    if (!str) {
        return -EFAULT;
    }

    if (!s) {
        return -EFAULT;
    }

    return impl_insert_buffer(str, str->len, strlen(s), s);
}

int string_append_fill(struct string *str, size_t n, char c)
{
    if (!str) {
        return -EFAULT;
    }

    return impl_insert_fill(str, str->len, n, c);
}

struct string *string_substr(const struct string *str, size_t pos, size_t len)
{
    struct string *sub;
    int r;

    if (!str) {
        errno = EFAULT;
        return NULL;
    }

    if (pos > str->len) {
        errno = ERANGE;
        return NULL;
    }

    if (len > SIZE_MAX - pos) {
        // Cap in case of numerical overflow.
        len = str->len - pos;

    } else if (pos + len > str->len) {
        // Cap to [pos, size()).
        len = str->len - pos;
    }

    sub = string_new();
    if (!sub) {
        return NULL;
    }

    r = string_append_buffer(sub, len, &str->buf[pos]);
    if (r < 0) {
        string_delete(sub);
        errno = -r;
        return NULL;
    }

    return sub;
}
