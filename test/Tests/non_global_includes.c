// RUN: maki %s | jq '[.[] | select(.IsDefinitionLocationValid == null or .IsDefinitionLocationValid == true)] | sort_by(.PropertiesOf, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
struct person_t {
    char *name;
    int age;
#include "h1.h"
};
int x = 1
#include "h2.h"
    ;
int main(int argc, char const *argv[]) {
#include "h3.h"
    return 0;
}

// CHECK: []
