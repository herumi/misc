#include <stdio.h>

extern "C" {

//__declspec(dllimport)
int fff(int, int);
//__declspec(dllimport)
int ggg(int, int);

}

int main()
{
	printf("fff=%d\n", fff(2, 3));
	printf("ggg=%d\n", ggg(5, 9));
}
