filename="$1"

./maki $filename \
    -fplugin-arg-maki---no-system-macros \
    -fplugin-arg-maki---no-builtin-macros \
    -fplugin-arg-maki---no-invalid-macros \
    | jq 'sort_by(.Kind, .DefinitionLocation, .InvocationLocation)' \
    | sed 's/\/home\/bpappas\/github.com\/appleseedlab\/maki\/test/{{.*}}/' \
    | sed 's/^/\/\/ CHECK: /' \
    >> $filename

    # | jq '[.[] | select(.Kind == "Include")]' \
    # | jq '[.[] | select(.IncludeName | (startswith("/usr")) | not)]' \