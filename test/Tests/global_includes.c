// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation, .Name)' | FileCheck %s --color
#include "h1.h"
struct person_t {
    char *name;
    int age;
};
#include "h2.h"
int x = 1;
#include "h3.h"
int main(int argc, char const *argv[]) {
    return 0;
}

// CHECK: []