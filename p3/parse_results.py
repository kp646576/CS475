import sys

f = open(sys.argv[1], 'r')
o = open(sys.argv[2], 'w')
lines = f.readlines()
count = 0
sum = 0.0
for i in lines:
	element = i.split(',')
	sum += float(element[2])
	count += 1
	if count == 11:
		o.write(','.join([str(element[0]), str(element[1]), str(sum /11)])+'\n')
		sum = 0.0
		count = 0
