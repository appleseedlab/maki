// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
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

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Include",
// CHECK:     "IsValid": false,
// CHECK:     "IncludeName": "{{.*}}/Tests/h1.h"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Include",
// CHECK:     "IsValid": false,
// CHECK:     "IncludeName": "{{.*}}/Tests/h2.h"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Include",
// CHECK:     "IsValid": false,
// CHECK:     "IncludeName": "{{.*}}/Tests/h4.h"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Include",
// CHECK:     "IsValid": false,
// CHECK:     "IncludeName": "{{.*}}/Tests/h3.h"
// CHECK:   }
// CHECK: ]
