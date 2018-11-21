import pandas as pd
import argparse
import os

class Node():
    def __init__(self, id, parent="", seq="", phen=""):
        self.id = id
        self.parent = parent
        self.children = []
        self.seq = seq
        self.phenotype = phen

def lookup_phenotype(row, genotype_bank):
    return row["sequence"]
    # return genotype_bank.loc[row["sequence"], "task_profile"]

def main():
    parser = argparse.ArgumentParser(description="Standards phylogeny file to ggmuller input files converter.")
    parser.add_argument("input", type=str, nargs='+', help="Input files")
    parser.add_argument("-output_prefix", "-out", type=str, help="Prefix to add to output file names")
    parser.add_argument("-treatment", "-treatment", type=str, required=True, help="What treatment is this run from?")
    parser.add_argument("-run_id", "-run", type=str, required=True, help="What run is this run from?")
    parser.add_argument("-genotype_bank", "-genotypes", type=str, required=True, help="File mapping genotypes to phenotypes")
    # Parse command line arguments.
    args = parser.parse_args()

    # # Extract/validate arguments
    # in_fp = args.input
    # if (not os.path.isfile(in_fp)):
    #     exit("Failed to find provided input file ({})".format(in_fp))
    
    
    if (args.output_prefix != None):
        adj_file_name = args.output_prefix + "_adjacency.csv"
        pop_file_name = args.output_prefix + "_pop_info.csv"
    else: 
        adj_file_name = ".".join(args.input.split(".")[:-1]) + "_adjacency.csv"
        pop_file_name = ".".join(args.input.split(".")[:-1]) + "_pop_info.csv"
        

    # adj_file = adj_file.astype(dtype={"Identity":"object","Parent":"object"})
    pop_file = pd.DataFrame({"Identity":[], "Population":[], "Time":[]})

    genotype_bank = pd.read_csv(args.genotype_bank, index_col="sequence")
    genotype_bank = genotype_bank[(genotype_bank["treatment"] == args.treatment) & (genotype_bank["run_id"] == int(args.run_id)) ]

    nodes = {}
    root = ""

    for filename in args.input:
        if (not os.path.isfile(filename)):
            exit("Failed to find provided input file ({})".format(filename))

        time = filename.split(".")[-2].split("-")[-1]

        df = pd.read_csv(filename)

        for i, row in df.iterrows():
            ancestors = row["ancestor_list"].strip("[] ").split(",")
            assert(len(ancestors) == 1)
            if ancestors[0] == "NONE":
                parent = -1
            else:
                parent = int(ancestors[0])

            if row["id"] in nodes:
                nodes[row["id"]].parent = parent
                nodes[row["id"]].sequence = row["sequence"]
                nodes[row["id"]].phenotype = lookup_phenotype(row, genotype_bank)
            else:
                nodes[row["id"]] = Node(row["id"], parent, row["sequence"], lookup_phenotype(row, genotype_bank))

            if parent == -1:
                root = row["id"]
            elif parent in nodes:
                nodes[parent].children.append(row["id"])
            else:
                nodes[parent] = Node(parent)
                nodes[parent].children.append(row["id"]) 


            pop_file = pop_file.append({"Identity":row["id"], "Population":row["num_orgs"], "Time":time}, ignore_index=True)
    
    adj_file, new_id_map = compress_phylogeny(root, nodes)
    print(pop_file)
    pop_file["Identity2"] = pop_file["Identity"].map(new_id_map)
    print(pop_file)
    # adj_file.drop_duplicates(inplace=True)
    # print(adj_file)

def compress_phylogeny(root, nodes):
    new_id_map = {}
    next_id = 1
    nodes[root].new_id = 0
    new_id_map[root] = 0
    frontier = nodes[root].children

    adj_file = pd.DataFrame({"Identity":[], "Parent":[]})
    
    while frontier:
        new_frontier = []

        for n in frontier:
            if nodes[n].phenotype == nodes[nodes[n].parent].phenotype:
                nodes[n].new_id = nodes[nodes[n].parent].new_id
            else:
                print(nodes[n].phenotype, nodes[nodes[n].parent].phenotype)
                nodes[n].new_id = next_id
                adj_file = adj_file.append({"Identity":next_id, "Parent":nodes[nodes[n].parent].new_id}, ignore_index=True)
                next_id += 1

            new_id_map[nodes[n].id] = nodes[n].new_id
            new_frontier.extend(nodes[n].children)

        frontier = new_frontier

    return adj_file, new_id_map
    


if __name__ == "__main__":
    main()