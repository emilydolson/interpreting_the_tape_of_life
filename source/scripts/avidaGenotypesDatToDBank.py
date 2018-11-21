'''
Script to make a bank of treatment, run_id, org_id, sequence, task profile for all genotype_details-*.dat files in given data directory.
'''
import argparse, os, copy, errno, csv, subprocess, sys

output_dump_dir = "./avida_analysis_dump"

treatment_whitelist = ["change", "l9", "limres", "empty"]

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

    # Grab a list of treatments in data directory
    treatments = [d for d in os.listdir(data_directory) if d in treatment_whitelist]
    runs_by_treatment = {t:[d for d in os.listdir(os.path.join(data_directory, t))] for t in treatments}

    print("Treatments: " + str(treatments))
    print("Runs by treatments: " + str(runs_by_treatment))

    # treatment, run_id, org_id, sequence, task profile
    bank_content = "treatment,run_id,genotype_id,sequence,gestation_time,genome_length,task_profile,"+ ",".join(tasks) + "\n"
    
    for treatment in treatments:
        for run in runs_by_treatment[treatment]:
            print("Pulling genotypes from {}[{}]".format(treatment, run))
            genotype_set = set([])
            run_dir = os.path.join(data_directory, treatment, run)
            # Grab the .dat lineage file
            detail_files = [f for f in os.listdir(run_dir) if "genotype_details-" in f]
            # For each data file:
            for detail_file in detail_files:
                # Parse the detail file.
                detail_fpath = os.path.join(run_dir, detail_file)
                details = ParseDetailFile(detail_fpath)
                for i in range(0, len(details)):                
                    genotype_id = details[i]["genotype id"]
                    # If we've already seen this genotype (in genotype_set) skip ahead
                    if (genotype_id in genotype_set): continue
                    genotype_set.add(genotype_id)
                    # A never-before-seen genotype!
                    gestation_time = details[i]["gestation time"]
                    genome_length = details[i]["genome length"]
                    sequence = details[i]["genome sequence"]
                    task_profile = "-".join([task for task in tasks if details[i][task] == "1"])
                    task_performance = [details[i][task] for task in tasks]
                    bank_content += ",".join(map(str, [treatment,run,genotype_id,sequence,gestation_time,genome_length,task_profile])) + "," + ",".join(map(str,task_performance)) + "\n"
    with open("genotype_bank.csv", "w") as fp:
        fp.write(bank_content)
                

if __name__ == "__main__":
    main()