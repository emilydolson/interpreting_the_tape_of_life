#!/home/emily/anaconda2/bin/python
###############################################################################
# Version: 1.1
# Last modified on: 3 April, 2016 
# Developers: Michael G. Epitropakis
#      email: m_(DOT)_epitropakis_(AT)_lancaster_(DOT)_ac_(DOT)_uk 
###############################################################################
from cec2013.cec2013 import *
import numpy as np
import sys
import pandas as pd

def main():

	problem = int(sys.argv[1])
	filename = sys.argv[2]

	# Create function
	f = CEC2013(problem)
	assert(f.get_dimension() == 2)

	df = pd.read_csv(filename)
	df.rename(str.strip, axis="columns", inplace=True)
	# print(df.columns.values)
	genotypes = [i.strip("[] ").split() for i in df["info"]]
	genotypes = map(lambda x: [float(x[0]), float(x[1])], genotypes)
	df["fitness"] = [f.evaluate(np.array(x)) for x in genotypes]
	genotypes = zip(*genotypes)

	df["x"] = genotypes[0]
	df["y"] = genotypes[1]

	print(df)
	# Evaluate :-)
	# x = np.ones(2)
	# value = f.evaluate(x)




if __name__ == "__main__":
	main()
