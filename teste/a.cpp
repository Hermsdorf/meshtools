#include <iostream>

using namespace std;
void mudarPont(int* p)
{
   (*p)++;
}

int main()
{
   int** pp;
   int*  px = new int();

   *px = 3;
   pp = &px;

   cout << "*pp = " << *pp << endl;
   cout << "**pp = " << **pp << endl;

   mudarPont(px);
   cout << "*px" << *px << endl;
   return 0;
}
