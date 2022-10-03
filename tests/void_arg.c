#define WRAP(CALL_TO_F) \
    do                  \
    {                   \
        CALL_TO_F;      \
    } while (0)

void foo() {}

int main(int argc, char const *argv[])
{
    // Should have an ambiguous signature
    WRAP(foo());
    return 0;
}
