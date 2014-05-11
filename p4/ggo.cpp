#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Global variables:
int		NowYear;		// 2014 - 2019
int		NowMonth;		// 0 - 11
int		NowNumDeer;
int 	NowNumMartian;
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

pthread_barrier_t DoneComputing, DoneGrainDeer, DoneAssigning, DonePrinting;
/*
** Add my own variable
*/

float Ranf(float low, float high)
{
	float r = (float) rand();		// 0 - RAND_MAX
	return(low  +  r * (high - low) / (float) RAND_MAX);
}

int Ranf()
{
	return rand() % 3;
}

// r --> 0
// p --> 1
// s --> 2
// 0 > 2
// 1 > 0
// 2 > 1
int RPC(int p1, int p2)
{
	int result = 2;
	if (p1 == p2)
		result = 0;
	else if (p1 == 2 && p2 == 1)
		result = 1;
	else if (p1 == 1 && p2 == 0)
		result = 1;
	else if (p1 == 0 && p2 == 2)
		result = 1;
	return result;
}

// Watcher
/*
	Keeps running until the year is 2020
	Print current set of global state variables
	Increment month counter
	Use new month counter to calculate Temperature and Precipitation
*/
void * watcher(void * args)
{
	FILE * f;
	f = fopen("./results.csv", "a");

	// Exit if the year is 2020
	while (NowYear < 2020) {
		pthread_barrier_wait(&DoneAssigning);

		// Write to CSV file
		fprintf(f, "%d, %d, %f, %f, %f\n", NowMonth, NowNumDeer, NowPrecip, NowTemp, NowHeight);

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

	fclose(f);
	// pthread_exit(NULL);
}

// Grain Growth
void * grain_growth(void * args)
{
	while(true) {
		float tfexp = pow( (NowTemp-MIDTEMP)/10, 2 );
		float tempFactor = pow( M_E, -(tfexp) );
		float pfexp = pow( (NowPrecip-MIDPRECIP)/10, 2 );
		float precipFactor =  pow( M_E, -(pfexp) );

		// Compute new grain height
		printf("GG: Prev Now Height = %f\n", NowHeight);
		float tmpHeight = NowHeight;
		tmpHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
	 	tmpHeight -= (float) NowNumDeer * ONE_DEER_EATS_PER_MONTH;
		if (tmpHeight < 0)
	 		tmpHeight = 0;
		
		pthread_barrier_wait(&DoneComputing);

		// Write to global NowHeight
		NowHeight = tmpHeight;
	 	printf("GG: New Now Height = %f\n", NowHeight);

	 	pthread_barrier_wait(&DoneAssigning);
	 	pthread_barrier_wait(&DonePrinting);
 	}
}

// Grain Deer
void * grain_deer(void * args)
{
	while (true) {
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
		
		pthread_barrier_wait(&DoneGrainDeer);
		pthread_barrier_wait(&DoneAssigning);
		pthread_barrier_wait(&DonePrinting);
	}
}

/*
	At the beginning of every year a martian and a deer play
	a round of rock-paper-scissors to determine which race
	will offer the sacrefice to the weather gods.  In the event
	of a draw both races will provide a scarefice.
*/
void * martian(void * args)
{
	while (true) {
		
		int DeerSacrefice = 0;
		int MartianSacrefice = 0;
		if (NowMonth % 12 == 0) {
			int m = Ranf();
			int d = Ranf();
			int result = RPC(m, d);
			if (result == 1 || result == 0)
				DeerSacrefice++;
			else if (result == 2 || result == 0)
				MartianSacrefice++;
		}
		pthread_barrier_wait(&DoneGrainDeer);
		NowNumDeer -= DeerSacrefice;
		NowNumMartian -= MartianSacrefice;

		pthread_barrier_wait(&DoneAssigning);
	 	pthread_barrier_wait(&DonePrinting);
	}
}

int main(int argc, char ** argv)
{
	// Starting Values
	NowNumMartian = 1;
	NowNumDeer    = 1;
	NowHeight     = 1.;
	NowMonth      = 0;
	NowYear       = 2014;

	pthread_barrier_init(&DoneComputing, NULL, 2);
	pthread_barrier_init(&DoneGrainDeer, NULL, 2);
	pthread_barrier_init(&DoneAssigning, NULL, 4);
	pthread_barrier_init(&DonePrinting,  NULL, 4);

	pthread_t w, gg, gd, m;

	pthread_create(&w,  NULL, watcher,      NULL);
	pthread_create(&gg, NULL, grain_growth, NULL);
	pthread_create(&gd, NULL, grain_deer,   NULL);
	pthread_create(&m,  NULL, martian,      NULL);

	pthread_join(w, NULL);
	if (pthread_cancel(gg) != 0)
		printf("pthread_cancel(gg) error.\n");
	if (pthread_cancel(gd) != 0)
		printf("pthread_cancel(gd) error.\n");
	if (pthread_cancel(m) != 0)
		printf("pthread_cancel(gd) error.\n");

	pthread_barrier_destroy(&DoneComputing);
	pthread_barrier_destroy(&DoneAssigning);
	pthread_barrier_destroy(&DonePrinting);
	pthread_barrier_destroy(&DoneGrainDeer);

	return 0;
}