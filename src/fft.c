#include "fft.h"
#include "settings.h"

/**
 * ===
 * FFT
 * ===
*/

/**
 * split an array
 * - if flag is EVEN (0), take only even elements
 * - else if flag is ODD (1) it takes only the odd ones
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

/**
 * recursively compute the fft on an array of complex numbers
 * splitting the array in two parts each recursion
*/
cplx *_fast_ft(cplx *compArray, const int len)
{
  cplx omegaN, omega;
  cplx *evenA, *oddA, *transformedA;
  int i;

  // recursive termination
  if(len == 1) {
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

/**
 * compute the amplitude of each fft component (cplx)
 * - amplitude = sqrt((real**2)/nc + (img**2)/nc)
 * - nc: number of components (samples)
*/
unsigned int
amplitude(cplx c, const unsigned int n)
{
  // compute a normalized amplitude (power spectrum, not squared)
  double sq = sqrt(pow(creal(c)/n, 2) + pow(cimag(c)/n, 2));

  return round(20*log10(sq)); // dB scale
}

/**
 * FFT algorithm entry point
 * receives an input len, a raw buffer and an output buffer
 * - computes the FFT of the raw buffer as a complex array
 * - computes the amplitude of the complex FFT components
 * - stores the result in the output buffer
*/
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

  for(i=1; i<N_SAMPLES; ++i){
      inputComponents[i] = sig[i];
  }
  for(i=N_SAMPLES; i<inLen; ++i) {
      inputComponents[i] = 0;
  }

  outputComponents = _fast_ft(inputComponents, inLen);
  for(i=0; i<N_SAMPLES; ++i){
    fftSig[i] = amplitude(outputComponents[i], inLen);
  }

  free(outputComponents);
}


/**
 * ===============================================
 * Compute logarithmic avg of fft amplitude buffer
 * ===============================================
*/

/**
 * upper frequency bound for current bin
*/
const double upperbound(const double bfreq, const double x)
{
    return bfreq*pow(2, 1/(2*x));
}

/**
 * lower frequency bound for current bin
*/
const double lowerbound(const double bfreq, const double x)
{
    return bfreq / pow(2, 1/(2*x));
}

/**
 * central (base) frequency of next bin 
*/
const double next_bfreq(const double bfreq, const double x)
{
    return bfreq*pow(2, 1/x);
}


/**
 * ===================================
 * Average frequency signal algorithm
 * ===================================
 * - while (!bin limit reached):
 * --- compute bin upper and lower bound from bfreq
 * --- sum each fftBuf[i] for i in (lower, upper)
 * --- divide by (upper-lower) to get the avg
 * --- if more than one fftAvg per bin, replicate on FOCUS
 * --- compute next bfreq
 */
void
average_signal(unsigned int *fftBuf, const int inLen, const int max, const double bf, const int oratio, unsigned int* fftAvg)
{
	int i = 0;
    int amp = 0;
    int first = 0;
    int last = 1;
    int div = 43/(N_SAMPLES/1024); // 1024 samples -> 43 reads / second
    double f = bf;
    unsigned int noteIdx = 0;
    unsigned int noteVal = 0;

    while(last <= N_SAMPLES/2) {

        int prev = first;
        first = floor(lowerbound(f, oratio) / div);

        if(first <= prev) {
            f = next_bfreq(f, oratio);
            continue;
        }

        last = ceil(upperbound(f, oratio) / div);
        if(last == first) first--;

        for(int j=first; j<last; ++j) {
            amp += fftBuf[j];
        }

        const int val = amp/abs(last-first);
        if(val > noteVal && i >= 1) {
            noteVal = val;
            noteIdx = i;
        }
        for(int j=i; j<i+FOCUS; ++j) {
            if(j < max) fftAvg[j] = val;
        }


        /* fprintf(stderr, "%d: %d, first: %d, f: %f, last: %d\n", i, fftAvg[i], first, f, last); */

        amp = 0;
        i += FOCUS;

        if(i > max) break;
        f = next_bfreq(f, oratio);
    }

    for(i=0; i<max; ++i) {
        if(i == noteIdx) continue; // boost current max (note)
        else fftAvg[i] -= 5;
    }
}

/**
 * ===================================================
 * Energy computation for beat tracking (beat_track.h)
 * ===================================================
 * Energy = average of the square of the amplitude
 * stored in fftBuf
*/
void avgEnergy(unsigned int* fftBuf, const int inLen, unsigned int* energyBuf)
{
    unsigned int i;
    for(i=0; i<inLen/2; ++i) {
        *energyBuf += pow(fftBuf[i], 2);
    }
    *energyBuf = (unsigned int)*energyBuf/(inLen/2);
}
