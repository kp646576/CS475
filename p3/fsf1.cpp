#include <omp.h>
#include <stdio.h>

struct s
{
	float value;
	int pad[NUM];
} Array[4];

int main(int argc, char ** argv)
{
	omp_set_num_threads( NUMT );

	int someBigNumber = 1000000000;

	double time0 = omp_get_wtime( );
	#pragma omp parallel for
	for( int i = 0; i < 4; i++ )
	{
		for( int j = 0; j < someBigNumber; j++ )
		{
			Array[ i ].value = Array[ i ].value + 2.;
		}
	}
	double time1 = omp_get_wtime( );
	double timediff = someBigNumber / (time1 - time0);
	printf("%d,%d,%f\n", NUMT, NUM, timediff);
}
