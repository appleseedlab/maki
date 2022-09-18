#define INC_G g++;

int g = 0;

int main(int argc, char const *argv[])
{
    INC_G;

    {
        int g = 0;
        INC_G;
    }

    return 0;
}
