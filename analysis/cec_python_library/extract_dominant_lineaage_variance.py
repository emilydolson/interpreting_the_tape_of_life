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
import glob
import os

def main():

	glob_pattern = sys.argv[1]
	outfilename = sys.argv[2]

	all_data = pd.DataFrame()

	# print(glob_pattern, glob.glob(glob_pattern))

	problem_map = {0:4, 1:5, 2:6, 6:12}

	for dirname in glob.glob(glob_pattern):
		print(dirname)
		run_log = dirname + "/run.log"
		filename = dirname + "/pop_5000/phylogeny_5000.csv"

		if not (os.path.exists(run_log) and os.path.exists(filename)):
			print("skipped")
			continue

		local_data = {}

		with open(run_log) as run_log_file:
			for line in run_log_file:
				if line.startswith("Doing initial"):
					break
				elif not line.startswith("set"):
					continue
				line = line.split()
				local_data[line[1]] = line[2]

		# Create function
		f = CEC2013(problem_map[int(local_data["PROBLEM"])])
		assert(f.get_dimension() == 2)			

		df = pd.read_csv(filename, index_col="id")

		df.rename(str.strip, axis="columns", inplace=True)
		# print(df.columns.values)
		genotypes = [i.strip("[] ").split() for i in df["info"]]
		genotypes = map(lambda x: [float(x[0]), float(x[1])], genotypes)
		df["fitness"] = [f.evaluate(np.array(x)) for x in genotypes]
		genotypes = zip(*genotypes)

		df["x"] = genotypes[0]
		df["y"] = genotypes[1]

		paths = []
		ids  = []

		next_gen = set([])
		curr_id = int(df[df["num_orgs"] > 0]["fitness"].idxmax())

		path = []
		fits = []
		x_magnitude = 0
		y_magnitude = 0
		total_magnitude = 0
		deleterious_steps = 0
		ben_steps = 0
		neutral_steps = 0
		while (curr_id > 1):
			# print(df.loc[curr_id, :])
			fits.append(df.loc[curr_id, "fitness"])
			
			# path.append(" ".join(["{:.2f}".format(i) for i in [df.loc[curr_id, "x"], df.loc[curr_id, "y"], df.loc[curr_id, "fitness"]]]))
			if (df.loc[curr_id, "parent_id"] == 1):
				break
			x_magnitude += abs(df.loc[df.loc[curr_id, "parent_id"], "x"] - df.loc[curr_id, "x"]) 
			y_magnitude += abs(df.loc[df.loc[curr_id, "parent_id"], "y"] - df.loc[curr_id, "y"])
			total_magnitude += abs(df.loc[df.loc[curr_id, "parent_id"], "x"] - df.loc[curr_id, "x"]) + abs(df.loc[df.loc[curr_id, "parent_id"], "y"] - df.loc[curr_id, "y"])
			if (df.loc[curr_id, "fitness"] > df.loc[df.loc[curr_id, "parent_id"], "fitness"]):
				ben_steps += 1
			elif (df.loc[curr_id, "fitness"] == df.loc[df.loc[curr_id, "parent_id"], "fitness"]):
				neutral_steps += 1
			else:
				deleterious_steps += 1
				
			curr_id = df.loc[curr_id, "parent_id"]

		paths.append(",".join(path))
		ids.append(curr_id)

		fits = pd.DataFrame(fits)
		temp = pd.DataFrame({"path": paths, "lin_id": ids})
		for k in local_data:
			temp[k] = local_data[k]
		temp["phenotypic_volatility"] = fits.var()
		temp["rolling_mean_1000_volatility"] = fits.rolling(1000).mean().var()
		temp["rolling_mean_500_volatility"] = fits.rolling(500).mean().var()
		temp["rolling_mean_100_volatility"] = fits.rolling(100).mean().var()
		temp["rolling_mean_50_volatility"] = fits.rolling(50).mean().var()
		temp["x_magnitude"] = x_magnitude
		temp["y_magnitude"] = y_magnitude
		temp["total_magnitude"] = total_magnitude

		all_data = pd.concat([all_data, temp])


	all_data.to_csv(outfilename,index=False)

	#print(df)
	# Evaluate :-)
	# x = np.ones(2)
	# value = f.evaluate(x)




if __name__ == "__main__":
	main()
