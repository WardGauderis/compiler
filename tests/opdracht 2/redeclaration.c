// this is a test file for checking: 'Redeclaration or redefinition of an existing variable.'

int main()
{
    int a   = 0; // no error
    a       = 1; // no error
    char* a = 2; // error
}