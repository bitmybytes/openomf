#include "utils/str.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void str_create(str *string) {
    string->len = 0;
    string->data = NULL;
}

void str_create_from_cstr(str *string, const char *cstr) {
    if(cstr) {
        string->len = strlen(cstr);
        string->data = malloc(string->len + 1);
        memcpy(string->data, cstr, string->len);
        string->data[string->len] = 0;
    } else {
        string->len = 0;
        string->data = NULL;
    }
}

void str_create_from_data(str *string, const char *data, size_t len) {
    string->len = len;
    string->data = malloc(len + 1);
    memcpy(string->data, data, len);
    string->data[string->len] = 0;
}

void str_free(str *string) {
    if(string->data != NULL) {
        free(string->data);
    }
    string->data = NULL;
    string->len = 0;
}

size_t str_size(const str *string) {
    return string->len;
}

void str_remove_at(str *string, size_t pos) {
   for(size_t i = pos; i < string->len - 1; i++) {
       string->data[i] = string->data[i + 1];
   }
   string->len--;
}

void str_substr(str *dst, const str *src, size_t start, size_t end) {
    if(src->data) {
        size_t len = end - start;
        dst->data = realloc(dst->data, len + 1);
        dst->len = len;
        memcpy(dst->data, src->data + start, len);
        dst->data[len] = 0;
    } else {
        dst->data = NULL;
        dst->len = 0;
    }
}

void str_copy(str *dst, const str *src) {
    if(src->data) {
        dst->data = realloc(dst->data, src->len + 1);
        dst->len = src->len;
        memcpy(dst->data, src->data, dst->len);
        dst->data[dst->len] = 0;
    } else {
        dst->data = NULL;
        dst->len = 0;
    }
}

void str_append(str *dst, const str *src) {
    dst->data = realloc(dst->data, dst->len + src->len + 1);
    memcpy(dst->data + dst->len, src->data, src->len);
    dst->len += src->len;
    dst->data[dst->len] = 0;
}

void str_append_c(str *dst, const char *src) {
    size_t srclen = strlen(src);
    dst->data = realloc(dst->data, dst->len + srclen + 1);
    memcpy(dst->data + dst->len, src, srclen);
    dst->len += srclen;
    dst->data[dst->len] = 0;
}

void str_prepend(str *dst, const str *src) {
    dst->data = realloc(dst->data, dst->len + src->len + 1);
    memmove(dst->data + src->len, dst->data, dst->len);
    memcpy(dst->data, src->data, src->len);
    dst->len += src->len;
    dst->data[dst->len] = 0;
}

int str_first_of(const str *string, char find, size_t *pos) {
    for(size_t i = 0; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return 1;
        }
    }
    return 0;
}

int str_next_of(const str *string, char find, size_t *pos) {
    for(size_t i = *pos; i < string->len; i++) {
        if(string->data[i] == find) {
            *pos = i;
            return 1;
        }
    }
    return 0;
}

int str_last_of(const str *string, char find, size_t *pos) {
    for(size_t i = string->len; i > 0; i--) {
        if(string->data[i-1] == find) {
            *pos = i-1;
            return 1;
        }
    }
    return 0;
}

int str_equal(const str *string, const str *string_b) {
    if(string->len != string_b->len) {
        return 0;
    }
    if(strcmp(string->data, string_b->data) != 0) {
        return 0;
    }
    return 1;
}

int str_cmp(const str *cmp_a, const str *cmp_b) {
    return strcmp(cmp_a->data, cmp_b->data);
}

char str_at(const str *string, size_t pos) {
    if(pos >= str_size(string)) {
        return 0;
    }
    return string->data[pos];
}

void str_toupper(str *string) {
    for(size_t i = 0; i < str_size(string); i++) {
        string->data[i] = toupper(string->data[i]);
    }
}

void str_tolower(str *string) {
    for(size_t i = 0; i < str_size(string); i++) {
        string->data[i] = tolower(string->data[i]);
    }
}

int str_to_int(const str *string, int *result) {
    *result = atoi(str_c(string));
    return 1;
}

int str_to_float(const str *string, float *result) {
    *result = atof(str_c(string));
    return 1;
}

int str_to_long(const str *string, long *result) {
    *result = atol(str_c(string));
    return 1;
}

const char* str_c(const str *string) {
    // At the moment, the internal representation of
    // string is compatible with C strings. So just return
    // a pointer to that data
    return string->data;
}
