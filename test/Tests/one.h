// RUN: maki %s | jq '[.[] | select(.IsDefinitionLocationValid == null or .IsDefinitionLocationValid == true)] | sort_by(.PropertiesOf, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
#define ONE 1

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Definition",
// CHECK:     "Name": "ONE",
// CHECK:     "IsObjectLike": true,
// CHECK:     "IsDefinitionLocationValid": true,
// CHECK:     "Body": "1",
// CHECK:     "DefinitionLocation": "{{.*}}/Tests/one.h:2:9",
// CHECK:     "EndDefinitionLocation": "{{.*}}/Tests/one.h:2:13"
// CHECK:   }
// CHECK: ]