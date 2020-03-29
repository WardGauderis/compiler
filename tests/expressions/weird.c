int main()
{
    int   x = (10 == 6 >= (5 * 3 + 1) - -3 + 3);
    int   y = 5135 - +-+3 * 5 + (3 % 5 < 4);
    int   z = (5 && 3 || 7 < (-3 + 5) * (5));
    int   a = (5 - 3 / 8 != 8 < 5 % 3) && (5 - 3 / 8 == 8 < 5 % 3) || 5555555;
    float b = 5.0 / 0.0;
    printf(x);
    printf(y);
    printf(z);
    printf(a);
    printf(b);
}