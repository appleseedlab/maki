typedef struct
{
    char *name;
    unsigned int age;
} Person;

#define PARENT_GET_AGE(P) ((P).age)
#define PARENT_PTR_GET_AGE(P) ((P)->age)

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

#define RGB_GET_R(rgb) ((rgb).r)
#define RGB_PTR_GET_R(rgb) ((rgb)->r)

int main(int argc, char const *argv[])
{
    Person p;
    p.age = 30;
    // Named argument types
    PARENT_GET_AGE(p);
    // Named argument types
    PARENT_PTR_GET_AGE(&p);

    struct color_t c;
    c.ty = RGB;
    c.rgb.r = 0x255;
    c.rgb.g = 0x255;
    c.rgb.b = 0;

    // Local/Anonymous types
    RGB_GET_R(c.rgb);
    // Local/Anonymous types
    RGB_PTR_GET_R(&c.rgb);

    return 0;
}
