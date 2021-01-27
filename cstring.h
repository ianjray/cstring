#ifndef LIBCSTRING_CSTRING_H_
#define LIBCSTRING_CSTRING_H_

/// String library.
///
/// Stores and manipulates sequences of characters.

/// Functions in the API are grouped by four return type conventions:
///
/// 1. Functions that do not return a value are NULL safe.
///
/// 2. Functions that *produce or return an object pointer* return that pointer on success, or NULL on failure with errno set to indicate the error.
///
/// 3. Functions that *perform an operation* return zero on success, or -1 on failure with errno set to indicate the failure.
///
/// 4. Other functions that *return non-pointer* return the value directly, and silently accept invalid arguments.

#include <stdbool.h>
#include <stddef.h>

#ifdef __has_attribute
# define PUBLIC __attribute__ ((visibility("default")))
#else
# define PUBLIC /*NOTHING*/
#endif

/// String object.
///
/// Strings are objects that represent sequences of characters.
///
/// This library is **not** thread-safe.
/// Caller must synchronize access to string objects.
/// Multiple readers are safe if no writers are active.
struct string;

/// Constructor.
/// Create a new empty string.
/// @return Pointer to string on success.
/// @return NULL on failure, and errno is set to:
///   - ENOMEM: Insufficient memory.
/// @note Memory ownership: Caller must string_delete() the returned pointer.
struct string *string_new(void) PUBLIC;

/// Destructor.
/// @note Memory ownership: Object takes ownership of the pointer.
void string_delete(struct string *) PUBLIC;

/// Test if string is empty.
/// @return True if string is empty or NULL, false otherwise.
bool string_empty(const struct string *) PUBLIC;

/// Get number of characters in string.
/// @return The number of characters in the string, or zero if empty or NULL.
size_t string_size(const struct string *) PUBLIC;

/// Erases all characters from the string.
void string_clear(struct string *) PUBLIC;

/// Reserves storage.
/// @param cap The new capacity.
/// @note Silently ignores reserving less storage than current @c string_size(), thus this function cannot cause data loss.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ENOMEM: Insufficient memory.
int string_reserve(struct string *, size_t cap) PUBLIC;

/// Get capacity.
/// The capacity represents the size of the allocated internal storage.
/// @return size_t Number of characters that there is currently room for.
/// @see string_reserve.
size_t string_capacity(const struct string *) PUBLIC;

/// Get character at position.
/// @return Character at given position, or zero if position invalid or string invalid.
/// @note Cannot distinguish between NUL character and invalid access.
///       Call string_size() first to validate position, or use string_c_str() for raw access.
char string_at(const struct string *, size_t pos) PUBLIC;

/// Get C string.
/// @return Pointer to string on success.
/// @return NULL on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
/// @note Memory ownership: Owned by the object; valid until object modified or deleted.
/// @warning Recommend that this internal pointer is not stored by the caller; call this API every time the information is needed.
const char *string_c_str(const struct string *) PUBLIC;

/// Move C string.
/// Detaches C string from this object, leaving a valid but empty string object.
/// @return Pointer to string on success.
/// @return NULL on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ENOMEM: Insufficient memory.
/// @note Memory ownership: Caller must free() the returned pointer.
char *string_c_str_move(struct string *) PUBLIC;

/// Insert @c n characters from buffer @c s at @c pos.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ENOMEM: Insufficient memory.
///   - ERANGE: Position invalid.
/// @note Memory ownership: Caller retains ownership of @c s.
int string_insert_buffer(struct string *, size_t pos, size_t n, const char *s) PUBLIC;

/// Insert string at @c pos.
/// @see string_insert_buffer.
int string_insert_c_str(struct string *, size_t pos, const char *s) PUBLIC;

/// Insert @c n characters at @c pos.
/// @see string_insert_buffer.
int string_insert_fill(struct string *, size_t pos, size_t n, char c) PUBLIC;

/// Erase @c len characters from @c pos.
/// @note Erasing at or past the end is a no-op (returns 0 successfully).
/// @param len Number of characters to erase (if the string is shorter, as many as possible are erased).
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ERANGE: Position invalid.
int string_erase(struct string *, size_t pos, size_t len) PUBLIC;

/// Append character.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ENOMEM: Insufficient memory.
int string_push_back(struct string *, char c) PUBLIC;

/// Erase last character.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ERANGE: String empty.
int string_pop_back(struct string *) PUBLIC;

/// Append @c n characters from buffer @c s at end of string.
/// Append buffer.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ENOMEM: Insufficient memory.
/// @note Memory ownership: Caller retains ownership of @c s.
int string_append_buffer(struct string *, size_t n, const char *s) PUBLIC;

/// Append string.
/// @see string_append_buffer.
int string_append_c_str(struct string *, const char *s) PUBLIC;

/// Append @c n characters.
/// @see string_append_buffer.
int string_append_fill(struct string *, size_t n, char c) PUBLIC;

/// Generate substring.
/// Get substring [pos, pos + len) or [pos, size()) if @c len is too big.
/// @param pos Start position in the range 0..size().
/// @param len Length (truncated if too long).
/// @return Pointer to substring on success.
/// @return NULL on failure, and errno is set to:
///   - EFAULT: NULL pointer argument.
///   - ENOMEM: Insufficient memory.
///   - ERANGE: Position invalid.
/// @note Memory ownership: Caller must string_delete() the returned pointer.
struct string *string_substr(const struct string *, size_t pos, size_t len) PUBLIC;

#endif // LIBCSTRING_CSTRING_H_
