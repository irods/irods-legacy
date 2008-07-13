#include <stdio.h>


int main (int argc, char** argv) {

	FILE* fp;
	int	i;
	char x[256];

	fp = fopen ("bigfile", "w");

	for (i=0; i<256; i++)
		x[i]=(char) i;

	for (i=0; i<100000; i++)
		fwrite(x, sizeof(x[0]), sizeof(x)/sizeof(x[0]), fp);

	fclose (fp);

}
