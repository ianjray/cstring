# cstring
String Object Implementation in C

The API is modelled on that of C++ std::string, as far as this is possible in C.

## API Design
The API is intentionally rather minimal, and the underlying storage is compatible with `string.h`.

* `std::string::assign` may be implemented as `string_clear` and `string_append_*`.
* `std::string::copy` may be implemented as `memcpy`.
* `std::string::find` may be implemented using `strstr` and `strchr`.
* `std::string::replace` may be implemented as `string_erase` and `string_insert_*`.
