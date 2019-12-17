#include "fft.h"
#include "settings.h"

/* if flag is EVEN (0), it takes only the even elements
 * otherwise if flag is ODD (1) it takes only the odd ones
 */
cplx *split_array(cplx *a, const int len, const int flag)
{
	int i, cnt = 0;
	cplx *ret = malloc((len/2)*sizeof(cplx));

	if(!ret) return NULL;

	for(i=0+flag; i<len; i=i+2){
		ret[cnt] = a[i];
		cnt++;
	}
	return ret;
}

/* recursively compute the fft on an array of complex numbers
 * splitting the array in two parts each recursion
 */
cplx *_fast_ft(cplx *compArray, const int len)
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
print_components(cplx *a, const int len)
{
  int i;
  for(i=0; i<len; i++){
    /*creal and cimag extract the real and imaginary parts of a[i]*/
    fprintf(stdout, "%g, %g\n", creal(a[i]), cimag(a[i])); 
  }
  fprintf(stdout, "\n");
}

unsigned int
amplitude(cplx c, const unsigned int n)
{
  // compute a normalized amplitude (power spectrum, not squared)
  double sq = sqrt(pow(creal(c)/n, 2) + pow(cimag(c)/n, 2));

  return round(20*log10(sq)); // dB scale
}

void
fast_fft(const int inLen, uint16_t *sig, unsigned int *fftSig)
{
  int i;
  cplx *inputComponents;
  cplx *outputComponents;

  if(inLen % 2 != 0){
    fprintf(stderr, "The length of the array MUST be a power of 2.");
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

const double upperbound(const double bfreq, const double x)
{
    return bfreq*pow(2, 1/(2*x));
}

const double lowerbound(const double bfreq, const double x)
{
    return bfreq / pow(2, 1/(2*x));
}

const double next_bfreq(const double bfreq, const double x)
{
    return bfreq*pow(2, 1/x);
}


// average fftBuf using a logarithmic scale for bins, to respect octaves
void
average_signal(unsigned int *fftBuf, const int inLen, const int max, const double bf, const int oratio, unsigned int* fftAvg)
{
	int i = 0;
    int j = 0;
    int amp = 0;
    int first = 0;
    int last = 1;
    int div = 43;
    double f = bf;

    while(last <= inLen/2) {

        int prev = first;
        first = floor(lowerbound(f, oratio) / div);

        if(first <= prev) {
            f = next_bfreq(f, oratio);
            continue;
        }

        last = ceil(upperbound(f, oratio) / div);
        if(last == first) first--;

        for(j=first; j<last; ++j) {
            amp += fftBuf[j];
        }

        const int val = amp/abs(last-first);
        for(int j=i; j<i+FOCUS; ++j) {
            if(j < max) fftAvg[j] = val;
        }

        /* fprintf(stderr, "%d: %d, first: %d, f: %f, last: %d\n", i, fftAvg[i], first, f, last); */

        amp = 0;
        i += FOCUS;

        if(i > max) break;
        f = next_bfreq(f, oratio);
    }
}

// compute the average energy of a sample array (in amplitude form)
void avgEnergy(unsigned int* fftBuf, const int inLen, unsigned int* energyBuf)
{
    unsigned int i;
    for(i=0; i<inLen/2; ++i) {
        *energyBuf += pow(fftBuf[i], 2);
    }
    *energyBuf = (unsigned int)*energyBuf/(inLen/2);
}
