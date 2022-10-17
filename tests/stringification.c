#define STRINGIFY(x) #x

#define h1(t) "<h1>" #t "</h1>"
#define p(t) "<p>" STRINGIFY(t) "</p>"

int main(int argc, char const *argv[])
{
    STRINGIFY(Hello !);
    h1(Header);
    p(some text);
    return 0;
}
