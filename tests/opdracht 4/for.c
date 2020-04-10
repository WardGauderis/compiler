int main(){
    for(int a = 6; a < 8; a++)
    {
        continue;
    }
    for(;;)
    {
        break;
        return; // no error
    }
    continue; // error
    break; // error
}
