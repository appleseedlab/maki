#define A 1

#define FOO(A, B, C) 2

int bar = 0;

#define bar() 3

struct baz
{
};

#define baz() 4

int buzz()
{
    return 5;
}

#define buzz() 5

int main(int argc, char const *argv[])
{
    // A parameter has the same name as another declaration
    FOO(1, 2, 3);

    // Has the same name as another declaration (variable declaration)
    bar();

    // Has the same name as another declaration (struct declaration)
    baz();

    // Has the same name as another declaration (function declaration)
    buzz();

    // TODO: Test enums, unions, and typedefs
    // TODO: Test the opposite case as well

    return 0;
}
