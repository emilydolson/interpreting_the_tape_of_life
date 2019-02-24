import glob
import os
import pandas as pd

all_data = []

for path in glob.glob("*/*[0-9]"):
    if not os.path.exists(path+"/lineage.csv"):
       continue
    
    print(path)
    print(path.split("/")[-2])
    avg_data = pd.read_csv(path+"/data/average.dat", delimiter= " ", skiprows=19, header=None, names = ["update","merit", "gestation", "fitness", "repro_rate", "dep_size", "copied_size", "executed_size", "dep_abundance", "prop_birth", "breed_true", "dep_depth", "generation", "neutral", "label", "true_rep_rate"], index_col=False)
    count_data = pd.read_csv(path+"/data/count.dat", delimiter= " ", skiprows=19, header=None, names = ["update","insts_ex_update", "number_of_orgs", "number_of_genotypes", "num_threshold_genotypes", "dep_species", "dep_thresh_species", "dep_lineages", "num_births", "num_deaths", "num_breed_true", "num_breed_true_questionmark", "no_birth_orgs", "singlue_threaded", "multi_threaded", "modified"], index_col=False)
    dom_data = pd.read_csv(path+"/data/dominant.dat", delimiter= " ", skiprows=19, header=None, names = ["update","dom_merit", "dom_gestation", "dom_fitness", "dom_repro_rate", "dom_size", "dom_copied_size", "dom_executed_size", "dom_abundance", "dom_num_birth", "dom_breed_true", "dom_gene_depth", "dom_breed_in", "dom_max_fitness", "dom_genotype_id", "dom_genotype_name"], index_col=False)
    phen_data = pd.read_csv(path+"/data/phenotype_count.dat", delimiter= " ", header=None, skiprows=8, names=["update","unqiue_phentype_task_done","shannon_diversity_phenotype_task_done","unique_phenotypes_task_count","average_phenotype_shannon_diversity_task_count","average_task_diversity"], index_col=False)
    resource_data = pd.read_csv(path+"/data/resource.dat", delimiter= " ", header=None, skiprows=15, names=["update","resnot", "resnand", "resand", "resorn", "resor", "resandn","resnor","resxor","resequ"], index_col=False)
    task_data = pd.read_csv(path+"/data/tasks.dat", delimiter= " ", header=None, skiprows=15, names=["update","not","nand","and","orn","or","andn","nor","xor","equ"], index_col=False)
    time_data = pd.read_csv(path+"/data/time.dat", delimiter= " ", header=None, skiprows=7, names=["update","avida_time","average_generation","num_executed"], index_col=False)
    var_data = pd.read_csv(path+"/data/variance.dat", delimiter= " ", skiprows=19, header=None, names = ["update","var_merit", "var_gestation", "var_fitness", "var_repro_rate", "dep_var_size", "var_copied_size", "var_executed_size", "dep_var_abundance", "dep_var_prop_birth", "dep_var_breed_true", "dep_var_depth", "var_generation", "var_neutral", "var_label", "var_true_rep_rate"], index_col=False)

    avg_data = avg_data.set_index("update")
    count_data = count_data.set_index("update")
    dom_data = dom_data.set_index("update")
    phen_data = phen_data.set_index("update")
    resource_data = resource_data.set_index("update")
    task_data = task_data.set_index("update")
    time_data = time_data.set_index("update")
    var_data = var_data.set_index("update")

    #frames= [avg_data]
    frames = [task_data, count_data, avg_data, resource_data, dom_data, phen_data, time_data, var_data]

    #print(frames[1])
    #frames = []
    #frames.append(pd.read_csv(path+"/oee.csv", index_col="generation"))
    frames.append(pd.read_csv(path+"/dominant.csv", index_col="update"))
    frames.append(pd.read_csv(path+"/phylodiversity.csv", index_col="update"))
    frames.append(pd.read_csv(path+"/lineage.csv", index_col="update"))

    for i in range(len(frames)):
        frames[i] = frames[i][~frames[i].index.duplicated(keep='first')]
    
    #print(frames[1])
    df = pd.concat(frames, axis=1, join="inner")
    
    df["environment"] = path.split("/")[-2]
    df["rep"] = path.split("/")[-1]

    #print(df)
    
    all_data.append(df)
   
res = pd.concat(all_data)

res.to_csv("all_data.csv")
