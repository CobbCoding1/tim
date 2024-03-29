#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifndef VIEW_MALLOC
#define VIEW_MALLOC malloc
#endif

typedef struct {
    const char *data;
    size_t len;
} String_View;

#define View_Print "%.*s"
#define View_Arg(view) (int)view.len, view.data
#define LITERAL_CREATE(lit) view_create(lit, sizeof(lit)-1)
#define LITERAL_VIEW(str) {.data=(str), .len=sizeof((str))-1}

String_View view_create(const char *str, size_t len);
char *view_to_cstr(String_View view);
String_View view_trim_left(String_View view);
String_View view_trim_right(String_View view);
int view_cmp(String_View a, String_View b);
int view_starts_with_c(String_View view, char c);
int view_starts_with_s(String_View a, String_View b);
int view_ends_with_c(String_View view, char c);
int view_ends_with_s(String_View a, String_View b);
int view_contains(String_View haystack, String_View needle);
size_t view_first_of(String_View view, char target);
size_t view_last_of(String_View view, char target);
size_t view_split(String_View view, char c, String_View *arr, size_t arr_s);
String_View view_chop(String_View view, char c);
String_View view_chop_left(String_View view);
size_t view_find(String_View haystack, String_View needle);
int view_to_int(String_View view);
float view_to_float(String_View view);

#endif

#ifdef VIEW_IMPLEMENTATION

float power(float base, float exponent) {
    float result = 1;
    for(size_t i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

String_View view_create(const char *str, size_t len) {
    String_View view = {
        .data = str,
        .len = len,
    };
    return view;
}

char *view_to_cstr(String_View view) {
    char *str = VIEW_MALLOC(sizeof(char) * view.len+1);
    strncpy(str, view.data, view.len);
    str[view.len] = '\0';
    return str;
}

String_View view_trim_left(String_View view) {
    size_t i = 0;
    while(i < view.len && isspace(view.data[i])) {
        i++;
    }
    return (String_View){
        .data = view.data + i,
        .len = view.len - i,
    };
}

String_View view_trim_right(String_View view) {
    size_t i = view.len - 1;
    while(i < view.len && isspace(view.data[i])) {
        i--;
    }
    return (String_View){
        .data = view.data,
        .len = i+1,
    };
}

int view_cmp(String_View a, String_View b) {
    if(a.len != b.len) return 0;
    return memcmp(a.data, b.data, a.len) == 0;
}

int view_starts_with_c(String_View view, char c) {
    return view.data[0] == c;
}

int view_starts_with_s(String_View a, String_View b) {
    String_View compare = view_create(a.data, b.len);
    return view_cmp(compare, b);
}

int view_ends_with_c(String_View view, char c) {
    return view.data[view.len-1] == c;
}

int view_ends_with_s(String_View a, String_View b) {
    String_View compare = view_create(a.data + a.len - b.len, b.len);
    return view_cmp(compare, b);
}

int view_contains(String_View haystack, String_View needle) {
    if(needle.len > haystack.len) return 0;
    String_View compare = view_create(haystack.data, needle.len);
    for(size_t i = 0; i < haystack.len; i++) {
        compare.data = haystack.data + i;
        if(view_cmp(needle, compare)) {
            return 1;
        }
    }
    return 0;
}

size_t view_first_of(String_View view, char target) {
    for(size_t i = 0; i < view.len; i++) {
        if(view.data[i] == target) {
            return i;
        }
    }
    return 0;
}

size_t view_last_of(String_View view, char target) {
    for(size_t i = view.len-1; i > 0; i--) {
        if(view.data[i] == target) {
            return i;
        }
    }
    return 0;
}

// needs work
size_t view_split(String_View view, char c, String_View *arr, size_t arr_s) {
    const char *cur = view.data;
    size_t arr_index = 0;
    size_t i;
    for(i = 0; i < view.len; i++) {
        if(view.data[i] == c) {
            if(arr_index < arr_s-2) {
                String_View new = {.data = cur, .len = view.data + i - cur};
                arr[arr_index++] = new;
                cur = view.data + i + 1;
            } else {
                String_View new = {.data = view.data + i+1, .len = view.len - i-1};
                arr[arr_index++] = new;
                return arr_index;
            }
        }
    }
    String_View new = {.data = cur, .len = view.data + i - cur};
    arr[arr_index++] = new;
    return arr_index;
}

String_View view_chop(String_View view, char c) {
    size_t i = 0;
    while(view.data[i] != c && i != view.len) i++;
    if(i < view.len) {
        i++; 
    }
    return (String_View) {
        .data = view.data + i,
        .len = view.len - i,
    };
}
    
String_View view_chop_left(String_View view) {
        if(view.len > 0) {
                view.data++;
                view.len--;
            }
        return view;
    }
}

size_t view_find(String_View haystack, String_View needle) {
    if(needle.len > haystack.len) return 0;
    String_View compare = view_create(haystack.data, needle.len);
    for(size_t i = 0; i < haystack.len; i++) {
        compare.data = haystack.data + i;
        if(view_cmp(needle, compare)) {
            return i;
        }
    }
    return 0;
}

int view_to_int(String_View view) {
    int result = 0;
    for(size_t i = 0; i < view.len; i++) {
        result = result * 10 + view.data[i] - '0';
    }
    return result;
}

float view_to_float(String_View view) {
    float result = 0;
    size_t dotpos = 0;
    for(size_t i = 0; i < view.len; i++) {
        if (view.data[i] == '.') {
            dotpos = view.len - i - 1;
        } else {
            result = result * 10 + (view.data[i] - '0');
        }
    }
    result /= power(10, dotpos);
    return result;
}

#endif 
