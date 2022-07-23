extern "C" int fff(int, int);

extern "C" {

__declspec(dllexport) int ggg(int x, int y)
{
	return fff(x, y) + 100;
}

}
