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

void string_append_buffer(struct string *str, const char *s, size_t n)
{
    if (str && s && *s && n) {
        memcpy(impl_append(str, n), s, n);
    }
}

void string_append_c_str(struct string *str, const char *s)
{
    if (str && s && *s) {
        strcpy(impl_append(str, strlen(s)), s);
    }
}

void string_append_fill(struct string *str, size_t n, char c)
{
    if (str && n) {
        memset(impl_append(str, n), c, n);
    }
}

char string_at(const struct string *str, size_t pos)
{
    if (str) {
        if (pos >= str->len) {
            return 0;
        }
        return str->buf[pos];
    }
    return 0;
}

char string_back(const struct string *str)
{
    if (str && str->len > 0) {
        return str->buf[str->len - 1];
    }
    return 0;
}

const char *string_c_str(const struct string *str)
{
    if (str) {
        return str->buf;
    }
    return NULL;
}

char *string_c_str_move(struct string *str)
{
    if (str) {
        char *buf = str->buf;
        str->cap = 0;
        str->len = 0;
        str->buf = NULL;
        return buf;
    }
    return NULL;
}

size_t string_capacity(const struct string *str)
{
    if (str) {
        return str->cap;
    }
    return 0;
}

void string_clear(struct string *str)
{
    if (str) {
        str->len = 0;
        if (str->buf) {
            str->buf[0] = 0;
        }
    }
}

void string_delete(struct string *str)
{
    if (str) {
        mem.dealloc(str->buf);
        mem.dealloc(str);
    }
}

bool string_empty(const struct string *str)
{
    return string_size(str) == 0;
}

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
    if (str && pos < str->len) {
        size_t rhs = str->len - pos;
        size_t n = 0;

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

void string_insert_buffer(struct string *str, size_t pos, const char *s, size_t n)
{
    if (str && s && *s && n) {
        memcpy(impl_insert(str, pos, n), s, n);
    }
}

void string_insert_c_str(struct string *str, size_t pos, const char *s)
{
    if (str && s && *s) {
        size_t n = strlen(s);
        memcpy(impl_insert(str, pos, n), s, n);
    }
}

void string_insert_fill(struct string *str, size_t pos, size_t n, char c)
{
    if (str && n) {
        memset(impl_insert(str, pos, n), c, n);
    }
}

struct string *string_new(void)
{
    struct string *str = mem.realloc(NULL, sizeof(struct string));
    str->cap = 0;
    str->len = 0;
    str->buf = mem.realloc(NULL, 1);
    str->buf[0] = 0;
    return str;
}

void string_pop_back(struct string *str)
{
    if (str && str->len > 0) {
        str->buf[--str->len] = 0;
    }
}

void string_push_back(struct string *str, char c)
{
    string_append_fill(str, 1, c);
}

void string_reserve(struct string *str, size_t cap)
{
    if (str && str->cap < cap) {
        str->cap = cap;
        str->buf = mem.realloc(str->buf, str->cap + 1);
    }
}

size_t string_size(const struct string *str)
{
    if (str) {
        return str->len;
    }
    return 0;
}

struct string *string_substr(const struct string *str, size_t pos, size_t len)
{
    struct string *sub = string_new();

    if (!str || len == 0) {
        // Return a newly constructed (empty) string.

    } else if (pos < str->len) {
        if (pos + len > str->len) {
            // Cap.
            len = str->len - pos;
        }

        string_append_buffer(sub, &str->buf[pos], len);
    }

    return sub;
}
