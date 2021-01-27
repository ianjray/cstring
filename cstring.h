#ifndef STRING__H
#define STRING__H

#include <stdbool.h>
#include <stddef.h>


struct string;

/// @brief Configure allocator interface.
/// @discussion Defaults to stdlib if not set.
/// @param realloc Allocator function, see realloc(3).
/// @param dealloc Deallocator function, see free(3).
void string_allocator_set(
        void *(*realloc)(void *, size_t),
        void (*dealloc)(void *));

/// @brief Constructor.
struct string *string_new(void);

/// @brief Destructor.
void string_delete(struct string *);

/// @return bool True if the string is empty.
bool string_empty(struct string const *);

/// @return size_t The number of characters in the string.
size_t string_size(struct string const *);

/// @brief Reserves storage.
void string_reserve(struct string *str, size_t cap);

/// @return size_t Number of characters that there is currently room for.
size_t string_capacity(struct string const *);

/// @brief Clears the contents of the string.
void string_clear(struct string *);

/// @return char Character at given position.
char string_at(struct string *, size_t pos);

/// @brief Assign content.
void string_assign_c_str(struct string *, const char *s);

/// @brief Append string.
void string_append_c_str(struct string *, const char *s);

/// @brief Append buffer.
void string_append_buffer(struct string *, const char *s, size_t n);

/// @brief Append @c n characters.
void string_append_fill(struct string *, size_t n, char c);

/// @brief Insert string at @c pos.
void string_insert_c_str(struct string *, size_t pos, const char *s);

/// @brief Insert @c n characters from buffer @c s  at @c pos.
void string_insert_buffer(struct string *, size_t pos, const char *s, size_t n);

/// @brief Insert @c n characters at @c pos.
void string_insert_fill(struct string *, size_t pos, size_t n, char c);

/// @brief Erase @c len characters from @c pos.
void string_erase(struct string *, size_t pos, size_t len);

/// @brief Append character.
void string_push_back(struct string *, char c);

/// @brief Erase last character.
void string_pop_back(struct string *str);

/// @return char* Pointer to C string.
const char *string_c_str(struct string const *);

/// @brief Move (detach) C string.
/// @return char* Pointer to C string.
char *string_c_str_move(struct string *);

#endif
