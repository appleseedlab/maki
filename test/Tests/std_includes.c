// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq '[.[] | select(.IncludeName | (startswith("/usr")) | not)]' | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color

#include <iso646.h>
#include <stdio.h>
#include <stdbool.h>

// COM: Maki should ignore the system header macros and print an empty JSON
// COM: array

int main(void) {
    (void)true;
    printf("Hello, world!\n");
    return 0;
}

// CHECK: []