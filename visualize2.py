import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

def plot_benchmark(file_path):
    df = pd.read_csv(file_path)
    
    # Extract unique container names
    containers = ['std::queue', 'ExpandingRingBuffer', 'ShiftToMiddleArray']
    
    # Define consistent colors
    colors = {
        'ShiftToMiddleArray': 'red',
        'ExpandingRingBuffer': 'green',
        'std::queue': 'blue'
    }
    
    test_sizes = df['Size'].unique()
    test_sizes.sort()
    
    plt.figure(figsize=(14, 8))
    
    # Define bar width and spacing
    bar_width = 0.4
    spacing = 0.3  # Increase spacing for better separation
    num_groups = len(test_sizes) * 3  # Each size has three groups (Time1, Time2, Time3)
    
    x_positions = np.arange(num_groups) * (len(containers) * (bar_width + spacing) + spacing)
    
    # Dictionary to track labels added to legend
    label_added = {}

    for i, size in enumerate(test_sizes):
        subset = df[df['Size'] == size]
        
        for k, time_col in enumerate(['Time1', 'Time2', 'Time3']):
            subset_time = subset[['Type', time_col]].set_index('Type')
            min_time = subset_time[time_col].min()
            percentages = (subset_time[time_col] / min_time) * 100
            
            x_base = x_positions[i * 3 + k]  # Base x-position for Time1, Time2, Time3
            
            for j, container in enumerate(containers):
                if container not in subset_time.index:
                    continue
                
                color = colors.get(container, 'gray')
                x_pos = x_base + j * (bar_width + spacing)
                label = f'{container}' if container not in label_added else ""
                label_added[container] = True  # Mark container as labeled
                
                # Plot bar
                plt.bar(x_pos, percentages[container], width=bar_width, color=color, label=label)
                
                # Mark if above 200%
                #if percentages[container] > 200:
                #    plt.annotate('!', (x_pos, 190), ha='center', fontsize=12, color='black')
    
    plt.xticks(x_positions[::3] + (len(containers) * bar_width / 2), test_sizes, rotation=45)
    plt.xlabel('Container Size')
    plt.ylabel('Relative Time (%)')
    plt.title(os.path.basename(file_path).replace('_', ' ').replace('.csv', ''))
    
    # Add legend outside the plot
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    # Save figure
    save_path = file_path.replace('.csv', '.png')
    plt.tight_layout()  # Adjust layout to prevent overlap
    plt.savefig(save_path)
    print(f"Saved visualization: {save_path}")
    plt.show()

# Process all uploaded files
files = ["benchmark_results_queue.csv"]

for file in files:
    plot_benchmark(file)
