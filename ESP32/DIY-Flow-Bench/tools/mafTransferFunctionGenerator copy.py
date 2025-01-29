import json
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import os
from pathlib import Path

def load_maf_data(filename):
    with open(filename, 'r') as f:
        data = json.load(f)
    x = np.array([float(k) for k in data.keys()])
    y = np.array([float(v) for v in data.values()])
    
    return x, y

def polynomial_fit(x, y, degree):
    coeffs = np.polyfit(x, y, degree)
    y_fit = np.polyval(coeffs, x)
    r2 = r_squared(y, y_fit)
    return coeffs, r2, y_fit

def r_squared(y_true, y_pred):
    ss_res = np.sum((y_true - y_pred) ** 2)
    ss_tot = np.sum((y_true - np.mean(y_true)) ** 2)
    return 1 - (ss_res / ss_tot)

def main():
    # Load data
    dirname = os.path.dirname(__file__)
    filename = os.path.join(dirname, 'mafData.json')
    x, y = load_maf_data(filename)
    
    # Polynomial fit
    degree = 6
    coeffs, r2, y_fit = polynomial_fit(x, y, degree)
    
    # Print results
    print("\nPolynomial coefficients (highest to lowest order):")
    print([f"{c:.6f}" for c in coeffs])
    print(f"\nR² value: {r2:.6f}")
    print("\nFor use in ESP32 code:")
    for i, c in enumerate(reversed(coeffs)):
        print(f"float c{i} = {c:.6f}f;")
    
    # Plot results
    plt.figure(figsize=(12, 8))
    plt.scatter(x, y, label='Raw Data', alpha=0.5)
    plt.plot(x, y_fit, 'r-', label='Polynomial Fit')
    plt.xlabel('Voltage (mV)')
    plt.ylabel('Flow (kg/h)')
    plt.title('MAF Sensor Transfer Function')
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    main()