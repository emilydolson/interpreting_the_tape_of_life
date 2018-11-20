'''
Run Avida analyze mode
'''
import argparse, os, copy, errno, csv, subprocess, sys

output_dump_dir = "./avida_analysis_dump"

treatment_whitelist = ["change", "l9", "limres", "empty"]

avida_lineage_analysis_path="avida__analyze_fdom_lineages.cfg"
avida_genotypes_analysis_path="avida__analyze_all.cfg"

default_lineage_target_update = 200000
default_lineage_target_generation = 10000

def ParseTimeFile(detail_fpath):
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
            if "1" in line: break
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
    parser.add_argument("avida_junk_path", type=str, help="Where should we look for all of the necessary avida junk?") # TODO - change this to 'avida junk path'
    # parser.add_argument("avida_analysis_path", type=str, help="Which avida analysis script are we running?")
    parser.add_argument("-extract_lineages_by_update", "-elu", action="store_true", help="Should we extract lineage information (from detail file @ target update)?")
    parser.add_argument("-extract_lineages_by_generation", "-elg", action="store_true", help="Should we extract lineage information (from detail file @ target generation)?")
    parser.add_argument("-extract_genotypes", "-egb", action="store_true", help="Should we extract genotype info from all detail files?")

    parser.add_argument("-update", "-u", type=int, help="Target update to pull from")
    parser.add_argument("-generation", "-gen", type=int, help="Target generation to extract from")
    
    args = parser.parse_args()

    data_directory = args.data_directory
    avida_junk_path = args.avida_junk_path

    if (args.update != None): 
        lineage_target_update = args.update
    else:
        lineage_target_update = default_lineage_target_update

    if (args.generation != None): 
        lineage_target_generation = args.generation
    else:
        lineage_target_generation = default_lineage_target_generation
    
    # Make a dump if it didn't already exist
    mkdir_p(output_dump_dir)

    # Grab list of treatments in data directory
    treatments = [d for d in os.listdir(data_directory) if d in treatment_whitelist]
    runs_by_treatment = {t:[d for d in os.listdir(os.path.join(data_directory, t))] for t in treatments}

    print("Treatments: " + str(treatments))
    print("Runs by treatment: " + str(runs_by_treatment))
    print("Pulling out lineages at update: " + str(lineage_target_update))

    # Copy avida executable into output dump to be used
    return_code = subprocess.call("cp {} {}".format(os.path.join(avida_junk_path, "*"), output_dump_dir), shell=True)
    
    # NOTE - have to loop over individual replicates (non-consistent detail file name)
    for treatment in treatments:
        runs = runs_by_treatment[treatment]
        for run in runs:
            run_dir = os.path.join(data_directory, treatment, run)
            print("Run directory: " + run_dir)
            # Now that we're in the run dir, what detail file (spop file) do we want to pull?
            run_data_dir = os.path.join(run_dir, "data")

            ############ Extract lineage at target lineage update ############
            if (args.extract_lineages_by_update):
                #detail-656225.spop
                # det_num = max([int(f.split("-")[-1].split(".")[0]) for f in os.listdir(run_data_dir) if "detail-" in f and ".spop" in f])
                det_file = "detail-{}.spop".format(lineage_target_update)
                det_num = det_file.split("-")[-1].split(".")[0]
                # Analysis parameters (input file path and output file path)
                in_det_fpath = os.path.join(run_data_dir, det_file)
                # Does this detail file exist?
                if (not os.path.isfile(in_det_fpath)): 
                    print("ERROR! Cannot file detail file: {}".format(in_det_fpath))
                    exit(-1)
                out_det_fpath = os.path.join(treatment, run, "lineage_details-update-{}.dat".format(det_num))
                out_det_fpath = out_det_fpath.strip("./")
                print("Out detail path: " + str(out_det_fpath))
                # Load analysis file
                analyze_fname = "lineage-analyze-{}-{}-update.cfg".format(treatment, run)
                temp_ascript_content = ""
                with open(avida_lineage_analysis_path, "r") as fp:
                    temp_ascript_content = fp.read()
                temp_ascript_content = temp_ascript_content.replace("<input_file>", in_det_fpath)
                temp_ascript_content = temp_ascript_content.replace("<output_file>", out_det_fpath)
                with open(os.path.join(output_dump_dir, analyze_fname), "w") as fp:
                    fp.write(temp_ascript_content)
                # Run analysis!
                avida_cmd = "./avida -a -set ANALYZE_FILE {}".format(analyze_fname)
                return_code = subprocess.call(avida_cmd, shell=True, cwd=output_dump_dir)
            
            ############ Extract lineage at target lineage update ############
            if (args.extract_lineages_by_generation):
                # Find appropriate detail file!
                time_fpath = os.path.join(run_data_dir, "time.dat")
                time_details = ParseTimeFile(time_fpath)
                # Find first time after 10k
                target_update = 0
                for i in range(0, len(time_details)):
                    gen = float(time_details[i]["average generation"])
                    if (gen >= lineage_target_generation):
                        target_update = int(time_details[i]["update"])
                        break
                # Update:
                print(target_update)
                # Find appropriate detail file
                det_file_updates = [int(f.split("-")[-1].split(".")[0]) for f in os.listdir(run_data_dir) if "detail-" in f and ".spop" in f]
                det_file_updates.sort()
                det_target_update = 0
                for u in det_file_updates:
                    if (u >= det_target_update):
                        det_target_update = u
                        break
                det_file = "detail-{}.spop".format(det_target_update)
                det_num = det_file.split("-")[-1].split(".")[0]
                # Analysis parameters (input file path and output file path)
                in_det_fpath = os.path.join(run_data_dir, det_file)
                # Does this detail file exist?
                if (not os.path.isfile(in_det_fpath)): 
                    print("ERROR! Cannot file detail file: {}".format(in_det_fpath))
                    exit(-1)
                out_det_fpath = os.path.join(treatment, run, "lineage_details-gen{}-{}.dat".format(lineage_target_generation,det_num))
                out_det_fpath = out_det_fpath.strip("./")
                print("Out detail path: " + str(out_det_fpath))
                # Load analysis file
                analyze_fname = "lineage-analyze-{}-{}-gen.cfg".format(treatment, run)
                temp_ascript_content = ""
                with open(avida_lineage_analysis_path, "r") as fp:
                    temp_ascript_content = fp.read()
                temp_ascript_content = temp_ascript_content.replace("<input_file>", in_det_fpath)
                temp_ascript_content = temp_ascript_content.replace("<output_file>", out_det_fpath)
                with open(os.path.join(output_dump_dir, analyze_fname), "w") as fp:
                    fp.write(temp_ascript_content)
                # Run analysis!
                avida_cmd = "./avida -a -set ANALYZE_FILE {}".format(analyze_fname)
                return_code = subprocess.call(avida_cmd, shell=True, cwd=output_dump_dir)
                
            ############ Run analyze mode over all genotypes found in all detail files ############
            if (args.extract_genotypes):
                # Get a list of detail files.
                det_files = [f for f in os.listdir(run_data_dir) if "detail-" in f and ".spop" in f]
                # For each detail file..
                for det_file in det_files:
                    det_num = det_file.split("-")[-1].split(".")[0]
                    # Parameters to provide to avida analyze mode: input and output file paths
                    in_det_fpath = os.path.join(run_data_dir, det_file)
                    out_det_fpath = os.path.join(treatment, run, "genotype_details-{}.dat".format(det_num))
                    out_det_fpath = out_det_fpath.strip("./")
                    print("Out detail path: " + str(out_det_fpath))
                    # Load the analysis file
                    analyze_fname="genotypes-analyze-{}-{}.cfg".format(treatment, run)
                    temp_ascript_content = ""
                    with open(avida_genotypes_analysis_path, "r") as fp:
                        temp_ascript_content = fp.read()
                    temp_ascript_content = temp_ascript_content.replace("<input_file>", in_det_fpath)
                    temp_ascript_content = temp_ascript_content.replace("<output_file>", out_det_fpath)
                    with open(os.path.join(output_dump_dir, analyze_fname), "w") as fp:
                        fp.write(temp_ascript_content)
                    # Run analysis!
                    avida_cmd = "./avida -a -set ANALYZE_FILE {}".format(analyze_fname)
                    return_code = subprocess.call(avida_cmd, shell=True, cwd=output_dump_dir)

            

if __name__ == "__main__":
    main()