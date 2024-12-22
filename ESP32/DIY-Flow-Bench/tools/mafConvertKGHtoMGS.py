import json

def convert_mgs_to_kgh(mg_per_sec):
    """Convert milligrams/second to kilograms/hour and round to integer"""
    return round(mg_per_sec * 3.6)

def main():
    # Read original JSON
    with open('/Users/mick/Documents/ESP32 Projects/DIY-Flow-Bench/ESP32/DIY-Flow-Bench/mafData/mafData.json', 'r') as file:
        data = json.load(file)
    
    # Convert values
    converted_data = {
        k: convert_mgs_to_kgh(v) for k, v in data.items()
    }
    
    # Write converted data to new file
    with open('/Users/mick/Documents/ESP32 Projects/DIY-Flow-Bench/ESP32/DIY-Flow-Bench/mafData/mafData_kgh.json', 'w') as file:
        json.dump(converted_data, file, indent=4)
    
    print(f"Conversion complete. Data saved to mafData_kgh.json")
    print(f"Sample conversion: {data['100']} mg/s = {converted_data['100']} kg/h")

if __name__ == "__main__":
    main()