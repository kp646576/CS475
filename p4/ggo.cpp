#include <math.h>
#include <pthread.h>
#include <stdio.h>

// Global variables:
int		NowYear;		// 2014 - 2019
int		NowMonth;		// 0 - 11
int		NowNumDeer;
float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches


// Units of grain growth are inches. 
const float GRAIN_GROWS_PER_MONTH   =	8.0;
const float ONE_DEER_EATS_PER_MONTH =	0.5;

// Units of precipitation are inches.
const float AVG_PRECIP_PER_MONTH 	=	6.0;
const float AMP_PRECIP_PER_MONTH 	=	6.0;
const float RANDOM_PRECIP 			=	2.0;

// Units of temperature are degrees Farhrenheit (°F). 
const float AVG_TEMP 				=	50.0;
const float AMP_TEMP 				=	20.0;
const float RANDOM_TEMP 			=	10.0;

const float MIDTEMP 				=	40.0;
const float MIDPRECIP 				=	10.0;

pthread_barrier_t DoneComputing, DoneAssigning, DonePrinting;
/*
Questions:
** Add my own variable into the mix
1. When does the program end? When the year is at 2019? YES

*/
// Watcher
// TODO: Add in barriers
// TODO: Need pthread_exit()?
/*
	Keeps running until the year is 2020
	Print current set of global state variables
	Increment month counter
	Use new month counter to calculate Temperature and Precipitation
*/
void * watcher(void * args)
{
	// Exit if the year is 2020
	while (NowYear < 2020) {
		pthread_barrier_wait(&DoneAssigning);

		// Print global variables
		printf("Now Year = %d\n",     NowYear);
		printf("Now Month = %d\n",    NowMonth);
		printf("Now Num Deer = %d\n", NowNumDeer);
		printf("Now Precip = %f\n",   NowPrecip);
		printf("Now Temp = %f\n",     NowTemp);
		printf("Now Height = %f\n",   NowHeight);

		// Update month count and re-calculate temperature and precipitation
		NowMonth++;
		if (NowMonth % 12 == 0)
			NowYear++;

		float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

		float temp = AVG_TEMP - AMP_TEMP * cos( ang );
		NowTemp = temp + Ranf( -RANDOM_TEMP, RANDOM_TEMP );

		float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
		NowPrecip = precip + Ranf( -RANDOM_PRECIP, RANDOM_PRECIP );
		if( NowPrecip < 0. )
			NowPrecip = 0.;

		pthread_barrier_wait(&DonePrinting);
	}
	pthread_exit();
}

// Grain Growth
void * grain_growth(void * args)
{
	while(True) {
		float tempFactor = pow(M_E, -( pow(T-MIDTEMP/10, 2) ));
		float percipFactor =  pow(M_E, -( pow(P-MIDPRECIP/10, 2) ));

		// Compute new grain height
		printf("GG: Prev Now Height = %d\n", NowHeight);
		float tmpHeight = NowHeight;
		tmpHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
	 	tmpHeight -= (float) NowNumDeer * ONE_DEER_EATS_PER_MONTH;
		if (tmpHeight < 0)
	 		tmpHeight = 0;
		
		pthread_barrier_wait(&DoneComputing);

		// Write to global NowHeight
		NowHeight = tmpHeight;
	 	printf("GG: New Now Height = %d\n", NowHeight);

	 	pthread_barrier_wait(&DoneAssigning);
	 	pthread_barrier_wait(&DonePrinting);
 	}
}

// Grain Deer
void * grain_deer(void * args)
{
	while (True) {
		// Carrying Capacity of graindeer = inches height of grain
		printf("GD: Prev Now Num Deer = %d\n", NowNumDeer);
		int tmpNumDeer = NowNumDeer;

		if (tmpNumDeer > NowHeight)
			tmpNumDeer--;
		else 
			tmpNumDeer++;

		pthread_barrier_wait(&DoneComputing);

		// Write to global NowNumDeer
		NowNumDeer = tmpNumDeer;
		printf("GD: New Now Num Deer = %d\n", NowNumDeer);
		
		pthread_barrier_wait(&DoneAssigning);
		pthread_barrier_wait(&DonePrinting);
	}
}

int main(int argc, char ** argv)
{
	// Starting Values
	NowNumDeer = 1;
	NowHeight  = 1.;
	NowMonth   = 0;
	NowYear    = 2014;

	pthread_t w, gg, gd;
	int val = 0;
	pthread_create(&w,  NULL, watcher,      (address_t) &val);
	pthread_create(&gg, NULL, grain_growth, (address_t) &val);
	pthread_create(&gd, NULL, grain_deer,   (address_t) &val);

	pthread_join(w, NULL);
	if (pthread_cancel(gg) != 0)
		printf("pthread_cancel(gg) error.\n");
	if (pthread_cancel(gd) != 0)
		printf("pthread_cancel(gd) error.\n");

	return 0;
}