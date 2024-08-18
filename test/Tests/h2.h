// RUN: maki %s -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' | FileCheck %s --color
#include "h4.h"

// CHECK: [
// CHECK:   {
// CHECK:     "Kind": "Include",
// CHECK:     "IsValid": true,
// CHECK:     "IncludeName": "{{.*}}/Tests/h4.h"
// CHECK:   }
// CHECK: ]