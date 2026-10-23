import sys
import uproot
import pandas as pd
import awkward as ak

def convert_awkward_to_native(awkward_array):
    """Convert awkward arrays (including strings) to native Python types."""
    try:
        return ak.to_list(awkward_array)
    except Exception as e:
        print(f"Error converting {awkward_array}: {e}")
        return awkward_array

def root_to_hdf5(path, tree, output):
    """
    Convert a specific tree from a ROOT file to a HDF5 file.
    
    Parameters:
    - path: Path to the ROOT file.
    - tree: Name of the tree in the ROOT file.
    - output: Path where the HDF5 file should be saved.
    """
    
    # Open the ROOT file and extract the desired tree
    with uproot.open(path) as file:
        tree_data = file[tree].arrays(library="ak")
    
    # Convert awkward arrays to native types for pandas
    converted_data = {key: convert_awkward_to_native(tree_data[key]) for key in tree_data.fields}

    # Convert to DataFrame
    df = pd.DataFrame(converted_data)
    
    print(df.columns)

    # Save to HDF5
    df.to_hdf(output, key=tree, mode="w", format="table", index=False)
    print(f"Converted '{tree}' from '{path}' to '{output}'")
    
if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python script_name.py <path> <tree> <output.h5>")
        sys.exit(1)
    
    path = sys.argv[1]
    tree = sys.argv[2]
    output = sys.argv[3]
    
    root_to_hdf5(path, tree, output)


