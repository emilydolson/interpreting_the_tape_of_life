'''
Script to make a bank of treatment, run_id, org_id, sequence, task profile for all genotype_details-*.dat files in given data directory.
'''
import argparse, os, copy, errno, csv, subprocess, sys

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
    parser.add_argument("where_all_the_detail_things_are", type=str, help="Target experiment directory.")

    args = parser.parse_args()

    data_directory = args.where_all_the_detail_things_are

    # Grab a list of treatments in data directory
    
    detail_files = [f for f in os.listdir(data_directory) if "phylogeny_sequence_details" in f]
    # print("Detail files found: " + str(detail_files))
    bank_content = "treatment,run_id,sequence,gestation_time,task_profile," + ",".join(tasks) + "\n"
    for detail_file in detail_files:
        print("Pulling sequences from: {}".format(detail_file))
        detail_path = os.path.join(data_directory, detail_file)
        details = ParseDetailFile(detail_path)
        treatment = detail_file.split("__")[-1].split("-")[0]
        run_id = detail_file.split("__")[-1].split("-")[-1]
        for i in range(0, len(details)):
            gestation_time = details[i]["gestation time"]
            sequence = details[i]["genome sequence"]
            task_profile = "-".join([task for task in tasks if details[i][task] == "1"])
            task_performance = [details[i][task] for task in tasks]
            bank_content += ",".join(map(str, [treatment,run_id,sequence,gestation_time,task_profile])) + "," + ",".join(map(str,task_performance)) + "\n"
    with open("genotype_bank.csv", "w") as fp:
        fp.write(bank_content)
    
      

if __name__ == "__main__":
    main()