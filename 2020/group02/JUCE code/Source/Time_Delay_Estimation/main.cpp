#include <iostream>
#include <stdlib.h>
#include "alignsigs.h"
using namespace std;

/*
* double alignsigs(double X1[48000], double X2[48000], double index1, double index2){...};
* 
*	INPUTS:
*		X1			input buffer 1, size 48.000 
*		X2			input buffer 2, size 48.000
*		index1		starting index of buffer 1
*		index2		starting index of buffer 2
* 
*	OUTPUT
*		delay		delay estimation between the two input signals
*	
*/


int main() {
	//declare variables
	double* X1 = new double[48000];		//input buffer 1
	double* X2 = new double[48000];		//input buffer 2
	double index1 = 12345;		//starting index input buffer 1
	double index2 = 12121;		//starting index input buffer 2

	//test data (delete)
	double a = 30;
	double b = 10;
	for (int i = 0; i < 48000; i++) {
		X1[i] = a;
		X2[i] = b;

		a++;
		b++;
		if (a == 50) { a = 10; }
		if (b == 50) { b = 10; }
	}

	//find the delay
	double delay = alignsigs(X1, X2, index1, index2);
	printf("\nThe delay between the two signals is\t%0.0f samples\n", delay); //print delay

	//free memory
	delete[] X1;
	delete[] X2;
}