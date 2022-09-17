#define GOTO_END goto end

int main(int argc, char const *argv[])
{
    while (1)
        GOTO_END;

end:
    return 0;
}
