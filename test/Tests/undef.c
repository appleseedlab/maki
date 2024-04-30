// RUN: maki %s | jq '[.[] | select(.IsDefinitionLocationValid == null or .IsDefinitionLocationValid == true)] | sort_by(.PropertiesOf, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
#define ONE 1
#define FOO() 2
int main(int argc, char const *argv[]) {
#undef ONE
#undef FOO
    return 0;
}

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "InspectedByCPP",
// CHECK:     "Name": "FOO"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "InspectedByCPP",
// CHECK:     "Name": "ONE"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "ONE",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "1",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/undef.c:2:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/undef.c:2:13"
// CHECK:   },
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "FOO",
// CHECK:     "IsObjectLike": false,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "2",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/undef.c:3:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/undef.c:3:15"
// CHECK:   }
// CHECK: ]
