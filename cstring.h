#ifndef STRING__H
#define STRING__H

#include <stdbool.h>
#include <stddef.h>

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
char string_at(struct string const *, size_t pos);

/// @brief Assign content.
void string_assign_c_str(struct string *, const char *s);

/// @brief Append string.
/// @discussion String object unchanged if @c s is NULL or empty.
void string_append_c_str(struct string *, const char *s);

/// @brief Append buffer.
/// @discussion String object unchanged if @c s is NULL or empty or @c n is zero.
void string_append_buffer(struct string *, const char *s, size_t n);

/// @brief Append @c n characters.
/// @discussion String object unchanged if @c n is zero.
void string_append_fill(struct string *, size_t n, char c);

/// @brief Insert string at @c pos.
/// @discussion String object unchanged if @c s is NULL or empty.
void string_insert_c_str(struct string *, size_t pos, const char *s);

/// @brief Insert @c n characters from buffer @c s  at @c pos.
/// @discussion String object unchanged if @c s is NULL or empty or @c n is zero.
void string_insert_buffer(struct string *, size_t pos, const char *s, size_t n);

/// @brief Insert @c n characters at @c pos.
/// @discussion String object unchanged if @c n is zero.
void string_insert_fill(struct string *, size_t pos, size_t n, char c);

/// @brief Erase @c len characters from @c pos.
void string_erase(struct string *, size_t pos, size_t len);

/// @param pos Start position (if out of bounds then the returned string is empty).
/// @param len Length (truncated if too long).
/// @return string Substring.
struct string *string_substr(struct string const *, size_t pos, size_t len);

/// @return Last character or NUL if string is empty.
char string_back(struct string const *);

/// @brief Append character.
void string_push_back(struct string *, char c);

/// @brief Erase last character.
void string_pop_back(struct string *str);

/// @return Reference to C string or NULL if the string object is empty.
const char *string_c_str(struct string const *);

/// @brief Move (detach) C string.
/// @return Copy of C string (caller frees).
char *string_c_str_move(struct string *);

#endif
