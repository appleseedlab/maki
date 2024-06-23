#!/usr/bin/bash

rm -fr Tests/ NewTests
cp -r test/Tests/ .
mkdir NewTests

for fn in `ls Tests/`
do

    printf "%s\n\n%s" \
        "$(cat "Tests/$fn" \
            | sed 's/^\/\/ CHECK: .*//')" \
        "$(./build/bin/maki "Tests/$fn" -fplugin-arg-maki---no-system-macros -fplugin-arg-maki---no-builtin-macros -fplugin-arg-maki---no-invalid-macros \
            | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' \
            | sed 's/^/\/\/ CHECK: /' \
            | sed 's/\/home\/bpappas\/github.com\/appleseedlab\/maki/{{.*}}/' \
            | sed 's/unnamed at Tests/unnamed at {{.*}}\/Tests/')" \
        > "NewTests/$fn"
    
done