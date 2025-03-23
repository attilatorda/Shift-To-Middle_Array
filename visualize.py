import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

def plot_benchmark(file_path):
    df = pd.read_csv(file_path)
    
    # Extract unique container names
    containers = df['Type'].unique()
    
    # Define consistent colors
    colors = {
        'ShiftToMiddleArray': 'red',
        'ExpandingRingBuffer': 'green'
    }
    
    # Assign the third container color dynamically
    remaining_containers = [c for c in containers if c not in colors]
    if remaining_containers:
        colors[remaining_containers[0]] = 'blue'
    
    # Group by Container Sizes
    test_sizes = df['Size'].unique()
    test_sizes.sort()
    
    plt.figure(figsize=(10, 6))
    
    for i, size in enumerate(test_sizes):
        subset = df[df['Size'] == size]
        min_time = subset['Time'].min()
        percentages = (subset['Time'] / min_time) * 100
        
        # Plot bars
        for j, (container, time) in enumerate(zip(subset['Type'], percentages)):
            color = colors.get(container, 'gray')
            plt.bar(i + j * 0.3, time, width=0.3, color=color, label=container if i == 0 else "")
            
            # Mark if above 200%
            #if time > 200:
            #    plt.annotate('!', (i + j * 0.3, 190), ha='center', fontsize=12, color='black')
    
    plt.xticks(np.arange(len(test_sizes)), test_sizes, rotation=45)
    plt.xlabel('Container Size')
    plt.ylabel('Relative Time (%)')
    plt.title(os.path.basename(file_path).replace('_', ' ').replace('.csv', ''))
    plt.legend()
    
    # Save figure
    save_path = file_path.replace('.csv', '.png')
    plt.savefig(save_path)
    print(f"Saved visualization: {save_path}")
    plt.show()

# Process all uploaded files
files = [
    "benchmark_results_deque.csv",
    "benchmark_results_list.csv",
    "benchmark_results_list_java.csv",
    "benchmark_results_list_trove.csv"
]

for file in files:
    plot_benchmark(file)
