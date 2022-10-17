#define WORD 32
#define BYTE 4 + 4

struct X
{
    int x : WORD;
    char c : BYTE;
};

int main(int argc, char const *argv[])
{
    return 0;
}
