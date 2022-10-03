#define WRAP_STMT(stmt) \
    do                  \
    {                   \
        stmt;           \
    } while (0)

int main(int argc, char const *argv[])
{
    // Should have an ambiguous signature
    WRAP_STMT(int x);
    return 0;
}
