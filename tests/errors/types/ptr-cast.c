// this folder is for checking: 'Operations or assignments of incompatible types.'
// this file specifically tests casting pointers

int* a; // warning unassigned
float* b; // warning unassigned

{
a = b; // warning: assignment from incompatible pointer type [-Wincompatible-pointer-types]
(int)a; // warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]
(float)a; // error: pointer value used where a floating point value was expected
(float*)a; //
(char)b; // warning: cast from pointer to integer of different size
}



