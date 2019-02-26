'''
Collect avida genome sequences from all phylogeny csv files into a single
avida .spop file.

Then, run avida analyze mode on it.
'''

import argparse, os, copy, errno, csv, subprocess, sys

output_dump_dir = "./avida_analysis_dump"

treatment_whitelist = ["change", "l9", "limres", "empty"]

avida_genotypes_analysis_path="avida__analyze_all.cfg"

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
    parser.add_argument("avida_junk_path", type=str, help="Where should we look for all of the necessary avida junk?") 

    args = parser.parse_args()

    data_directory = args.data_directory
    avida_junk_path = args.avida_junk_path

    # Make a dump if it didn't already exist
    mkdir_p(output_dump_dir)

    # Grab list of treatments in data directory
    treatments = [d for d in os.listdir(data_directory) if d in treatment_whitelist]
    runs_by_treatment = {t:[d for d in os.listdir(os.path.join(data_directory, t))] for t in treatments}

    print("Treatments: " + str(treatments))
    print("Runs by treatment: " + str(runs_by_treatment))

    return_code = subprocess.call("cp {} {}".format(os.path.join(avida_junk_path, "*"), output_dump_dir), shell=True)
    
    
    for treatment in treatments:
        runs = runs_by_treatment[treatment]
        for run in runs:
            sequence_set = set([])
            print("Collecting from {}[{}]".format(treatment, run))
            phylo_spop_fname = "phylogeny-sequences-{}-{}.spop".format(treatment, run)
            # Have we already collected this in a previous attempt?
            if (os.path.isfile(os.path.join(output_dump_dir, phylo_spop_fname))):
                print("  Already collected this phylo seq data, skipping.")
                continue
            run_dir = os.path.join(data_directory, treatment, run)
            # Grab all of the phylogeny files
            phylo_snapshots = [f for f in os.listdir(run_dir) if "phylogeny-snapshot-" in f]
            for snapshot in phylo_snapshots:
                snapshot_path = os.path.join(run_dir, snapshot)
                snapshot_content = None
                with open(snapshot_path, "r") as fp:
                    snapshot_content = fp.read().split("\n")
                header = snapshot_content[0].split(",")
                header_lu = {header[i]:i for i in range(0, len(header))}
                snapshot_content = snapshot_content[1:]
                for line in snapshot_content:
                    if line == "": continue
                    sequence = line.split(",")[header_lu["sequence"]]
                    sequence_set.add(sequence)
            # For each run, write out a detail file.
            print("  For your entertainment, num sequences: {}".format(len(sequence_set)))
            
            phylo_seq_detail_content = "#filetype genotype_data\n"
            phylo_seq_detail_content += "#format id hw_type inst_set sequence length\n\n"
            id_index = 0
            for seq in sequence_set:
                phylo_seq_detail_content += " ".join(map(str,[id_index, "0", "heads_default", seq, len(seq)])) + "\n"
                id_index += 1
            with open(os.path.join(output_dump_dir, phylo_spop_fname), "w") as fp:
                fp.write(phylo_seq_detail_content)
    
    # Run analyze mode!
    # det_file = "phylogeny-sequences.spop"
    # # Analysis parameters (input file path and output file path)
    # in_det_fpath = os.path.join(det_file)

    # out_det_fpath = os.path.join("all-phylogeny-genotypes.dat")
    # print("Out detail path: " + str(out_det_fpath))
    # # Load analysis file
    # analyze_fname = "phylo-seq-analyze.cfg"
    # temp_ascript_content = ""
    # with open(avida_genotypes_analysis_path, "r") as fp:
    #     temp_ascript_content = fp.read()
    # temp_ascript_content = temp_ascript_content.replace("<input_file>", in_det_fpath)
    # temp_ascript_content = temp_ascript_content.replace("<output_file>", out_det_fpath)
    # with open(os.path.join(output_dump_dir, analyze_fname), "w") as fp:
    #     fp.write(temp_ascript_content)
    # # Run analysis!
    # avida_cmd = "./avida -a -set ANALYZE_FILE {}".format(analyze_fname)
    # return_code = subprocess.call(avida_cmd, shell=True, cwd=output_dump_dir)
            
    




                
                






if __name__ == "__main__":
    main()