'''
Aggregate lineage .dat files, extract genotype sequences and compressed phenotype sequences.
'''

import argparse, os, copy, errno, csv, subprocess, sys

output_dump_dir = "./avida_analysis_dump"

treatment_whitelist = ["change", "l9"]

tasks = ["not", "nand", "and", "ornot", "or", "andnot", "nor", "xor", "equals"]

def ParseDetailFile(detail_fpath):
    """
    Given file pointer to detail file, extract information into form below:
    return [{"detail":value, "detail":value, ...}, ...]
    """
    orgs = []
    with open(detail_fpath, "r") as detail_fp:
        ######################
        # Step 1) Build Legend
        ###
        # Travel to the legend.
        for line in detail_fp:
            if line == "# Legend:\n": break
        # Consume the legend.
        details = []
        for line in detail_fp:
            if line == "\n": break
            details.append(line.split(":")[-1].strip())
        ######################
        # Step 2) Consume Organisms
        ###
        for line in detail_fp:
            org_dets = line.strip().split(" ")
            org = {details[i].lower():org_dets[i] for i in range(0, len(org_dets))}
            orgs.append(org)
    return orgs

def mkdir_p(path):
    """
    This is functionally equivalent to the mkdir -p [fname] bash command
    """
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def main():
    parser = argparse.ArgumentParser(description="Data aggregation script.")
    parser.add_argument("data_directory", type=str, help="Target experiment directory.")
    
    args = parser.parse_args()

    data_directory = args.data_directory

    # Grab a list of treatments in data directory.
    treatments = [d for d in os.listdir(data_directory) if d in treatment_whitelist]
    runs_by_treatment = {t:[d for d in os.listdir(os.path.join(data_directory, t))] for t in treatments}

    print("Treatments: " + str(treatments))
    print("Runs by treatments: " + str(runs_by_treatment))

    # Loop over 
    lineage_seq_content =  "treatment,run_id,max_update,total_muts,total_substitutions,total_insertions,total_deletions,"
    lineage_seq_content += "phen_seq_by_geno_unique_state_cnt,phen_seq_by_geno_length,phen_seq_by_geno_volatility,phen_seq_by_geno_chg_rate,"
    lineage_seq_content += "phen_seq_by_geno_state,phen_seq_by_geno_start,phen_seq_by_geno_duration,"
    lineage_seq_content += "phen_seq_by_phen_unique_state_cnt,phen_seq_by_phen_length,phen_seq_by_phen_volatility,phen_seq_by_phen_chg_rate,"
    lineage_seq_content += "phen_seq_by_phen_state,phen_seq_by_phen_start,phen_seq_by_phen_duration\n"
    
    for treatment in treatments:
        for run in runs_by_treatment[treatment]:
            print("Extracting lineage from: {}[{}]".format(treatment, run))
            run_dir = os.path.join(data_directory, treatment, run)
            # Grab the .dat lineage file
            dats = [f for f in os.listdir(run_dir) if "lineage_details-" in f]
            for dat in dats:
                dat_fpath = os.path.join(run_dir, dat)
                # Parse the detail file
                details = ParseDetailFile(dat_fpath)
                # Max update
                max_update = dat_fpath.split("-")[-1].split(".")[0]
                # Extract phenotype sequence
                phenotype_seq_states = []
                phenotype_seq_starts = []
                phenotype_seq_durations = []
                phenotype_seq_volatility = 0 # Number of state changes   
                sub_mut_cnt = 0
                ins_mut_cnt = 0
                dels_mut_cnt = 0
                for i in range(0, len(details)):
                    muts_from_parent = details[i]["mutations from parent"].split(",")
                    for mut in muts_from_parent:
                        if (len(mut) == 0): continue
                        if (mut[0] == "M"): sub_mut_cnt += 1
                        elif (mut[0] == "I"): ins_mut_cnt += 1
                        elif (mut[0] == "D"): dels_mut_cnt += 1
                        else: print("Unknown mutation type (" + str(mut) + ")!")
                    # State?
                    state = "-".join([task.upper() for task in tasks if details[i][task] == "1"])
                    if (i > 0):
                        if (phenotype_seq_states[-1] != state):
                            phenotype_seq_volatility += 1                
                    # Start?
                    start = int(details[i]["update born"])
                    if start < 0: start = 0 # Clamp start update at 0 for sanity
                    # Update previous duration
                    if i: phenotype_seq_durations.append(start - phenotype_seq_starts[-1])
                    phenotype_seq_starts.append(start)
                    phenotype_seq_states.append(state)
                phenotype_seq_durations.append(int(max_update) - phenotype_seq_starts[-1])
                total_muts = sub_mut_cnt + ins_mut_cnt + dels_mut_cnt

                phenotype_seq_unique_state_cnt = len(set(phenotype_seq_states))
                phenotype_seq_length = len(phenotype_seq_states)
                if (phenotype_seq_volatility):
                    phenotype_seq_chg_rate = sum(phenotype_seq_durations) / phenotype_seq_volatility
                else:
                    phenotype_seq_chg_rate = 0
                
                # Compress phenotype sequence
                compressed__phenotype_seq_starts = []
                compressed__phenotype_seq_states = []
                compressed__phenotype_seq_durations = []
                compressed__phenotype_seq_volatility = 0
                for i in range(0, len(phenotype_seq_states)):
                    # If we're at the first state, just set start, states, and duration from source.
                    if (i == 0):
                        compressed__phenotype_seq_starts.append(phenotype_seq_starts[0])
                        compressed__phenotype_seq_states.append(phenotype_seq_states[0])
                        compressed__phenotype_seq_durations.append(phenotype_seq_durations[0])
                        continue
                    # Are we the same?
                    if (phenotype_seq_states[i] == compressed__phenotype_seq_states[-1]):
                        # Increment duration
                        compressed__phenotype_seq_durations[-1]+=phenotype_seq_durations[i]
                        continue
                    else: # Are we different?
                        # Different!
                        compressed__phenotype_seq_starts.append(phenotype_seq_starts[i])
                        compressed__phenotype_seq_states.append(phenotype_seq_states[i])
                        compressed__phenotype_seq_durations.append(phenotype_seq_durations[i])
                        compressed__phenotype_seq_volatility += 1
                
                compressed__phenotype_seq_unique_state_cnt = len(set(compressed__phenotype_seq_states))
                compressed__phenotype_seq_length = len(compressed__phenotype_seq_states)
                if (compressed__phenotype_seq_volatility):
                    compressed__phenotype_seq_chg_rate = sum(compressed__phenotype_seq_durations) / compressed__phenotype_seq_volatility
                else:
                    compressed__phenotype_seq_chg_rate = 0
                
                # Write line of content!
                # "treatment,run_id,max_update,total_muts,total_substitutions,total_insertions,total_deletions,"
                #  phen_seq_by_geno_unique_state_cnt, phen_seq_by_geno_length, phen_seq_by_geno_volatility, phen_seq_by_geno_chg_rate
                # "phen_seq_by_geno_state,phen_seq_by_geno_start,phen_seq_by_geno_duration"
                #  phen_seq_by_phen_unique_state_cnt, phen_seq_by_phen_length, phen_seq_by_phen_volatility, phen_seq_by_phen_chg_rate
                # "phen_seq_by_phen_state,phen_seq_by_phen_start,phen_seq_by_phen_duration\n"
                phen_seq_by_geno_state = "\"{}\"".format(",".join(phenotype_seq_states))
                phen_seq_by_geno_start = "\"{}\"".format(",".join(map(str, phenotype_seq_starts)))
                phen_seq_by_geno_duration = "\"{}\"".format(",".join(map(str, phenotype_seq_durations)))
                phen_seq_by_phen_state = "\"{}\"".format(",".join(compressed__phenotype_seq_states))
                phen_seq_by_phen_start = "\"{}\"".format(",".join(map(str, compressed__phenotype_seq_starts)))
                phen_seq_by_phen_duration = "\"{}\"".format(",".join(map(str, compressed__phenotype_seq_durations)))
                
                lineage_seq_content += ",".join(map(str, [treatment, run, max_update, total_muts, sub_mut_cnt, ins_mut_cnt, dels_mut_cnt, 
                                                        phenotype_seq_unique_state_cnt,phenotype_seq_length,phenotype_seq_volatility,phenotype_seq_chg_rate,
                                                        phen_seq_by_geno_state, phen_seq_by_geno_start, phen_seq_by_geno_duration, 
                                                        compressed__phenotype_seq_unique_state_cnt,compressed__phenotype_seq_length,compressed__phenotype_seq_volatility,compressed__phenotype_seq_chg_rate,
                                                        phen_seq_by_phen_state, phen_seq_by_phen_start, phen_seq_by_phen_duration])) + "\n"

    # Write out sequences to file
    with open("lineage_sequences.csv", "w") as fp:
        fp.write(lineage_seq_content)

                
                    


            




if __name__ == "__main__":
    main()