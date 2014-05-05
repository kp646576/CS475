#!/bin/bash
for name in fsf1 fsf2
	do
		for t in 1 2 4
			do
				for i in {0..20..1}
					do
						echo g++ -DNUMT=$t -DNUM=$i $name.cpp -o $name -fopenmp
						g++ -DNUMT=$t -DNUM=$i $name.cpp -o $name -fopenmp
						for j in {0..10..1}
							do
								./$name >> ./results/$name.csv
							done
					done
			done
	done
