#include <unistd.h>
int main(int c, char** a)
{
	execv("./sequential_min_max", a);
	return 0;
}