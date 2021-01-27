# cstring
String Object Implementation in C

The API is modelled on that of C++ std::string, as far as this is possible in C.

## API Design
The API is intentionally rather minimal, and the underlying storage is compatible with `string.h`.

* `std::string::assign` may be implemented as `string_clear` and `string_append_*`.
* `std::string::copy` may be implemented as `memcpy`.
* `std::string::find` may be implemented using `strstr` and `strchr`.
* `std::string::replace` may be implemented as `string_erase` and `string_insert_*`.

## Example

```c
#include <assert.h>
#include <errno.h>
#include <libcstring/cstring.h>
#include <string.h>

int main(void)
{
    struct string *s = string_new();

    // Initially empty.
    assert(string_empty(s));
    assert(string_size(s) == 0);

    // "c"
    string_append_buffer(s, 1, "cXXX");

    assert(!string_empty(s));
    assert(string_size(s) == 1);
    assert(string_at(s, 0) == 'c');
    assert(string_at(s, 1) == 0);

    // C standard library string functions work with the underlying storage.
    assert(strchr(string_c_str(s), 'c') != NULL);
    assert(strchr(string_c_str(s), 'X') == NULL);

    // "cef"
    //   ^^
    string_append_c_str(s, "ef");

    // "cefggg"
    //     ^^^
    string_append_fill(s, 3, 'g');

    // Insert at end.
    // "cefggghh"
    //        ^^
    string_insert_fill(s, string_size(s), 2, 'h');

    // Insert at beginning.
    // "abcefggghh"
    //  ^^
    string_insert_c_str(s, 0, "ab");

    // "abcdefggghh"
    //     ^
    string_insert_buffer(s, 3, 1, "dXXX");

    // "abcdefggghh"
    //         ^^
    // "abcdefghh"
    string_erase(s, 7, 2);

    // "abcdefgh"
    string_pop_back(s);

    // "abcdefghi"
    //          ^
    string_push_back(s, 'i');

    assert(0 == strcmp(string_c_str(s), "abcdefghi"));

    // ""
    string_clear(s);

    // "The quick brown fox"
    string_append_c_str(s, "The quick brown fox");

    // "The  brown fox"
    //      ^
    const char *old_word = "quick";
    size_t index = (size_t)(strstr(string_c_str(s), old_word) - string_c_str(s));
    string_erase(s, index, strlen(old_word));

    // "The lazy brown fox"
    //      ^^^^
    string_insert_c_str(s, index, "lazy");

    assert(0 == strcmp(string_c_str(s), "The lazy brown fox"));

    string_delete(s);

    return 0;
}
```

## Installation

```bash
./configure
make
sudo make install
```

## Requirements

- C99 or later
- POSIX-compatible system (for `sys/types.h`)

## Thread Safety

This library is **not** thread-safe.
