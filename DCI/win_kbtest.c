#include <stdio.h>
#include <conio.h>

int main() {
	int k = 0;
	while (k < 5) {
		if (_kbhit) {
			printf("The %c key was hit.\n", _getch());
			k++;
		}
	}
	
	return 0;
}