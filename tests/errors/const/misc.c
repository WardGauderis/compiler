// this is a test file for checking: 'Assignment to a const variable.'

{
    const float a = 0;
    a = 1; // error
    ++a; // error
    a++; //error
    int b = a; // no error
    a + 5; // no error
}