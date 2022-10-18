#define ID(x) x

int main(int argc, char const *argv[])
{
    struct local_t
    {
        int x;
    };

    struct local_t l, *l_ptr, l_arr[5];

    ID(l);

    ID(l_ptr);

    ID(l_arr);

    return 0;
}
