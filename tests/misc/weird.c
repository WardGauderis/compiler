

int a = 1;
float x = 5.;
int y = (x = 7.) && (a = 0);
printf(x-y);