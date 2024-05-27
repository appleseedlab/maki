// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color

#define ZERO 0
typedef struct foo {
#define ONE 1
} foo;

typedef union bar {
#define TWO 2
// COM: This #include'd definition of EIGHT is not at the global scope
#include "eight.h"
} bar;

int baz =
#define THREE 3
    0;

int main(void) {
#define FOUR 4
    return 0;
}
#define FIVE 5

// COM: This separate definition of EIGHT is at the global scope
#include "eight.h"


// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "THREE",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "3",
// CHECK:     "IsDefinedAtGlobalScope": false,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:15:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:15:15"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "FOUR",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "4",
// CHECK:     "IsDefinedAtGlobalScope": false,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:19:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:19:14"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "FIVE",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "5",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:22:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:22:14"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "ZERO",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "0",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:3:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:3:14"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "ONE",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "1",
// CHECK:     "IsDefinedAtGlobalScope": false,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:5:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:5:13"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "TWO",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "2",
// CHECK:     "IsDefinedAtGlobalScope": false,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:9:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/define_at_non_global_scope.c:9:13"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "EIGHT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "8",
// CHECK:     "IsDefinedAtGlobalScope": false,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/eight.h:2:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/eight.h:2:15"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "EIGHT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "8",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/eight.h:2:9",
// CHECK:     "EndDefinitionLocation": "/home/bpappas/github.com/appleseedlab/maki/test/Tests/eight.h:2:15"
// CHECK:   }
// CHECK: ]
