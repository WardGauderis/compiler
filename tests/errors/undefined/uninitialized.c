// this is a test file for checking: 'Use of an undefined or uninitialized variable.'

int a;
{
printf(a);
a = a + 5;
printf(a);
a + 5;
}

