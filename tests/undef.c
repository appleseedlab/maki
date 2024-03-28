#define ONE 1
#define FOO() 2

int main(int argc, char const *argv[])
{
#undef ONE
#undef FOO
    return 0;
}


// No expected invocation properties
