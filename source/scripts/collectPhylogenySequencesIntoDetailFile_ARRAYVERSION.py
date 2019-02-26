'''
Collect avida genome sequences from all phylogeny csv files into a single
avida .spop file.

Then, run avida analyze mode on it.
'''

import argparse, os, copy, errno, csv, subprocess, sys

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
    parser.add_argument("avida_dump_directory", type=str)
    parser.add_argument("treatment", type=str)
    parser.add_argument("run_id", type=str)

    args = parser.parse_args()

    data_directory = args.data_directory
    dump_directory = args.avida_dump_directory # Needs to have 

    # Grab list of treatments in data directory
    treatment = args.treatment
    run_id = args.run_id
    
    sequence_set = set([])
    print("Collecting from {}[{}]".format(treatment, run_id))
    phylo_spop_fname = "phylogeny-sequences-{}-{}.spop".format(treatment, run_id)
    # Have we already collected this in a previous attempt?
    if (not os.path.isfile(os.path.join(dump_directory, phylo_spop_fname))):
        print("  Haven't collected this spop before, collecting now.")  
        run_dir = os.path.join(data_directory, treatment, run_id)
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
        with open(os.path.join(dump_directory, phylo_spop_fname), "w") as fp:
            fp.write(phylo_seq_detail_content)
        print("  Wrote spop out to {}".format(os.path.join(dump_directory, phylo_spop_fname)))
    else:
        print("  Already collected, skipping straight to detailing.")

    # Detail that spop!
    print("Detailing spop from {}[{}]".format(treatment, run_id))
    # Analysis parameters (input file path and output file path)
    in_spop_fpath = os.path.join(phylo_spop_fname)
    out_det_fpath = os.path.join("phylogeny_sequence_details__{}-{}.dat".format(treatment, run_id))
    print("  In spop path: " + in_spop_fpath)
    print("  Out detail path: " + out_det_fpath)
    # Load analysis file
    analyze_fname = "phylo-seq-analyze__{}-{}.cfg".format(treatment, run_id)
    temp_ascript_content = ""
    with open(avida_genotypes_analysis_path, "r") as fp:
        temp_ascript_content = fp.read()
    temp_ascript_content = temp_ascript_content.replace("<input_file>", in_spop_fpath)
    temp_ascript_content = temp_ascript_content.replace("<output_file>", out_det_fpath)
    with open(os.path.join(dump_directory, analyze_fname), "w") as fp:
        fp.write(temp_ascript_content)
    # # Run analysis!
    avida_cmd = "./avida -a -set ANALYZE_FILE {}".format(analyze_fname)
    return_code = subprocess.call(avida_cmd, shell=True, cwd=dump_directory)
            
    




                
                






if __name__ == "__main__":
    main()