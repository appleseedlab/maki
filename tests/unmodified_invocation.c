// The bodies of all invocations are not modified values

#define INC_G g++

int g = 0;

int main(int argc, char const *argv[])
{
    // Impure, but not expanded where a modifiable value is required
    INC_G;

    {
        int g = 0;
        // Impure, but not expanded where a modifiable value is required
        INC_G;
    }

    return 0;
}
