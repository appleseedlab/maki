typedef struct
{
    char *name;
    unsigned int age;
} Person;

#define ID(P) P
#define PTR(P) (&(P))

enum COLOR_TYPE
{
    RGB,
    HSL
};

struct color_t
{
    enum COLOR_TYPE ty;
    union
    {
        struct
        {
            unsigned int r, g, b;
        } rgb;
        struct
        {
            unsigned int h, s, l;
        } hsl;
    };
};

#define COLOR_GET_RGB(color) ((color).rgb)
#define COLOR_GET_RGB_PTR(color) ((color)->rgb)

int main(int argc, char const *argv[])
{
    Person p;
    // Anonymous types
    ID(p);
    // Anonymous types
    PTR(p);

    struct color_t c;
    // Local/Anonymous types
    COLOR_GET_RGB(c);
    // Local/Anonymous types
    COLOR_GET_RGB_PTR(&c);

    return 0;
}
