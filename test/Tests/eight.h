// RUN: maki %s | jq '[.[] | select(.IsDefinitionLocationValid == null or .IsDefinitionLocationValid == true)] | sort_by(.PropertiesOf, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
#define EIGHT 8

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "EIGHT",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "8",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/eight.h:2:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/eight.h:2:15"
// CHECK:   }
// CHECK: ]
