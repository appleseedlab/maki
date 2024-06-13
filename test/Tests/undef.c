// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
#define ONE 1
#define FOO() 2
int main(int argc, char const *argv[]) {
#undef ONE
#undef FOO
    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "ONE",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "1",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/undef.c:2:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/undef.c:2:13"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "FOO",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "2",
// CHECK:     "IsDefinedAtGlobalScope": true,
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/undef.c:3:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/undef.c:3:15"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "InspectedByCPP",
// CHECK:     "Name": "FOO"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "InspectedByCPP",
// CHECK:     "Name": "ONE"
// CHECK:   }
// CHECK: ]