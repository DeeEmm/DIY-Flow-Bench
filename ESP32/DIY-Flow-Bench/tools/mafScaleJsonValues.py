import json

# Load the original data
with open('/Users/mick/Documents/ESP32 Projects/DIY-Flow-Bench/ESP32/DIY-Flow-Bench/mafData/mafData.json', 'r') as f:
    original_data = json.load(f)

# Calculate the scaling factor
old_max = 1024
new_max = 500
scale_factor = new_max / old_max

# Create new dictionary with scaled indices and interpolated values
new_data = {}
for new_index in range(new_max + 1):
    # Find the corresponding position in the original data
    old_position = new_index / scale_factor
    
    # Get the lower and upper indices for interpolation
    old_lower = int(old_position)
    old_upper = min(old_lower + 1, old_max)
    
    # Get values for interpolation
    value_lower = float(original_data[str(old_lower)])
    value_upper = float(original_data[str(old_upper)])
    
    # Calculate interpolation factor
    fraction = old_position - old_lower
    
    # Perform linear interpolation
    interpolated_value = value_lower + fraction * (value_upper - value_lower)
    
    new_data[str(new_index)] = round(interpolated_value)

# Save the new data
with open('/Users/mick/Documents/ESP32 Projects/DIY-Flow-Bench/ESP32/DIY-Flow-Bench/mafData/mafData_500.json', 'w') as f:
    json.dump(new_data, f, indent=4)
