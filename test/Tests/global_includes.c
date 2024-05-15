// RUN: maki %s | jq '[.[] | select(.IsDefinitionLocationValid == null or .IsDefinitionLocationValid == true)] | sort_by(.PropertiesOf, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
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