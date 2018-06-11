#include "fft.h"
#include <mpd/client.h>

/* if flag is EVEN (0), it takes only the even elements
 * otherwise if flag is ODD (1) it takes only the odd ones
 */
cplx *split_array(cplx *a, int len, int flag)
{
	int i, cnt = 0;
	cplx *ret = malloc((len/2)*sizeof(cplx));

	for(i=0+flag; i<len; i=i+2){
		ret[cnt] = a[i];
		cnt++;
	}
	return ret;
}

/* recursively compute the fft on an array of complex numbers
 * this algorithm involves splitting the array in two parts each recursion
 * to be more efficient 
 */
cplx *_fast_ft(cplx *compArray, int len)
{
	cplx omegaN, omega;
	cplx *evenA, *oddA, *transformedA;
	int i;

	/*termination*/
	if(len == 1){
		return compArray;
	}

	omega = 1;
	omegaN = cexp(2*PI*I/len); //the fourier coefficient
    evenA = _fast_ft(split_array(compArray, len, EVEN), len/2);
	oddA = _fast_ft(split_array(compArray, len, ODD), len/2);

	/*the final array*/
	transformedA = malloc(len*sizeof(cplx));

	for(i=0; i<(len/2); i++){
		transformedA[i] = evenA[i] + omega*oddA[i];
		transformedA[i+(len/2)] = evenA[i] - omega*oddA[i];
		omega = omegaN*omega;
	}
	free(evenA);
	free(oddA);
	free(compArray);
	return transformedA;
}

void
print_components(cplx *a, int len)
{
	int i;
	for(i=0; i<len; i++){
		/*creal and cimag extract the real and imaginary parts of a[i]*/
		fprintf(stdout, "%g, %g\n", creal(a[i]), cimag(a[i])); 
	}
	fprintf(stdout, "\n");
}

unsigned int
amplitude(cplx c, unsigned int n)
{
    // compute a normalized amplitude (actually power spectrum, since it's not squared
    // normalize on n (N_SAMPLES)
	double sq = 0;
	unsigned int res = 0;

	/*compute amplitude*/
    sq = sqrt(pow(creal(c)/n, 2) + pow(cimag(c)/n, 2));
	res = round(20*log10(sq)); // dB scale
    /*fprintf(stderr, "%f - %d\n", sq, res);*/
	return res;
}

void
fast_fft(int inLen, uint16_t *sig, unsigned int *fftSig)
{
	int i;
	cplx *inputComponents;
	cplx *outputComponents;

	if(inLen % 2 != 0){
		fprintf(stderr, "Note that the length of the array MUST be a power of 2.");
		exit(EXIT_FAILURE);
	}
	inputComponents = (cplx*)malloc((inLen)*sizeof(cplx));

	for(i=1; i<inLen; i++){
		inputComponents[i] = sig[i];
	}

	outputComponents = _fast_ft(inputComponents, inLen);
	for(i=0; i<inLen; i++){
		fftSig[i] = amplitude(outputComponents[i], inLen);
	}

	free(outputComponents);
}

void
average_signal(unsigned int *fftBuf, int inLen, int max, unsigned int* fftAvg)
{
	int i, j, step, k=0;
	unsigned int avg;

	step = inLen/(max);
	for(i=0; i<inLen; i=i+step){
		avg = 0;
		for(j=0; j<step; j++){
			avg += fftBuf[i+j];	
		}
		fftAvg[k++] = avg/step; //the 80 is a correction for the display
	}
}
