'''
Run Avida analyze mode
'''
import argparse, os, copy, errno, csv, subprocess, sys

output_dump_dir = "./avida_analysis_dump"

treatment_whitelist = ["change", "l9"]

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
    parser.add_argument("avida_analysis_path", type=str, help="Which avida analysis script are we running?")
    
    args = parser.parse_args()

    data_directory = args.data_directory
    avida_junk_path = args.avida_junk_path
    avida_analysis_path = args.avida_analysis_path
    
    # Make a dump if it didn't already exist
    mkdir_p(output_dump_dir)

    # Grab list of treatments in data directory
    treatments = [d for d in os.listdir(data_directory) if d in treatment_whitelist]
    runs_by_treatment = {t:[d for d in os.listdir(os.path.join(data_directory, t))] for t in treatments}

    print("Treatments: " + str(treatments))
    print("Runs by treatment: " + str(runs_by_treatment))

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
            #detail-656225.spop
            det_num = max([int(f.split("-")[-1].split(".")[0]) for f in os.listdir(run_data_dir) if "detail-" in f and ".spop" in f])
            det_file = "detail-{}.spop".format(det_num)
            # Analysis parameters (input file path and output file path)
            in_det_fpath = os.path.join(run_data_dir, det_file)
            out_det_fpath = os.path.join(output_dump_dir, treatment, run, "lineage_details-{}.dat".format(det_num))
            out_det_fpath = out_det_fpath.strip("./")
            print("Out detail path: " + str(out_det_fpath))
            # Load analysis file
            analyze_fname = "analyze-{}-{}.cfg".format(treatment, run)
            temp_ascript_content = ""
            with open(avida_analysis_path, "r") as fp:
                temp_ascript_content = fp.read()
            temp_ascript_content = temp_ascript_content.replace("<input_file>", in_det_fpath)
            temp_ascript_content = temp_ascript_content.replace("<output_file>", out_det_fpath)
            with open(os.path.join(output_dump_dir, analyze_fname), "w") as fp:
                fp.write(temp_ascript_content)
            # Run analysis!
            avida_cmd = "./avida -a -set ANALYZE_FILE {}".format(analyze_fname)
            return_code = subprocess.call(avida_cmd, shell=True, cwd=output_dump_dir)
             


            

    '''
    # Run analyses!
    # - Load analysis file, find/replace a few things.
    temp_ascript_content = ""
    with open(avida_analysis_path, "r") as fp:
        temp_ascript_content = fp.read()
    temp_ascript_content = temp_ascript_content.replace("<treatments>", " ".join(treatments))
    
    print("Here's that analysis script: \n" + temp_ascript_content)

    with open(os.path.join(output_dump_dir, "analyze.cfg"), "w") as fp:
        fp.write(temp_ascript_content)

    # - Run!
    avida_cmd = "./avida -a -set ANALYZE_FILE analyze.cfg"
    return_code = subprocess.call(avida_cmd, shell=True, cwd=output_dump_dir)
    '''
    


if __name__ == "__main__":
    main()