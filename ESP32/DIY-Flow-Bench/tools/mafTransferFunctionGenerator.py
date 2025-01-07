# mafTransferFunctionGenerator.py
# This file is part of the DIY FLow Bench Project. https//github.com/DeeEmm/DIY-Flow-Bench
# Author: DeeEmm
# Description: This script reads the MAF sensor data from mafData.json file and fits a polynomial, exponential, and power law function to the data. 
# The best fit is then plotted along with the raw data. 
# # Coefficients of the best fit function are printed to the console. (generally 6th order polynomial)
# MAF Coefficients can be added to mafData.h 
# MAF sensor data must be in the format of a JSON file with the voltage as the key and the flow as the value. 
# MAF sensor key values must be in format 0-5000mv. Values can be scaled from other ranges using the mafScaleJsonValues.py script
# MAF sensor flow values must be in the format KG/H. Values can be converted from MG/S to KG/H using the mafConvertKGHtoMGS.py script
# Version 1 - 22.12.24


import json
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
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

def exponential_func(x, a, b, c):
    return a * np.exp(b * x) + c

def power_func(x, a, b, c):
    return a * np.power(x, b) + c

def r_squared(y_true, y_pred):
    ss_res = np.sum((y_true - y_pred) ** 2)
    ss_tot = np.sum((y_true - np.mean(y_true)) ** 2)
    return 1 - (ss_res / ss_tot)

def get_equation_text(fit_type, coeffs, r2):
    if fit_type.startswith('poly'):
        degree = int(fit_type.split('_')[1])
        terms = []
        for i, c in enumerate(reversed(coeffs)):
            if i == 0:
                terms.append(f"{c:.2f}")
            elif i == 1:
                terms.append(f"{c:.2f}x")
            else:
                terms.append(f"{c:.2f}x^{i}")
        eq = "y = " + " + ".join(reversed(terms))
    elif fit_type == 'exponential':
        eq = f"y = {coeffs[0]:.2f}e^({coeffs[1]:.2f}x) + {coeffs[2]:.2f}"
    else:  # power
        eq = f"y = {coeffs[0]:.2f}x^{coeffs[1]:.2f} + {coeffs[2]:.2f}"
    return f"${eq}$\n$R^2 = {r2:.4f}$"

def main():
    # Load data
    x, y = load_maf_data('/Users/mick/Documents/ESP32 Projects/DIY-Flow-Bench/ESP32/DIY-Flow-Bench/tools/mafData.json')
    
    # Remove zero values
    mask = (x != 0) & (y != 0)
    x, y = x[mask], y[mask]
    
    # Try different fits
    fits = {}
    
    # Polynomial fits
    for degree in range(1, 7):
        coeffs, r2, y_fit = polynomial_fit(x, y, degree)
        fits[f'poly_{degree}'] = {'coeffs': coeffs, 'r2': r2, 'y_fit': y_fit}
    
    # Exponential fit
    try:
        popt_exp, _ = curve_fit(exponential_func, x, y)
        y_fit_exp = exponential_func(x, *popt_exp)
        r2_exp = r_squared(y, y_fit_exp)
        fits['exponential'] = {'coeffs': popt_exp, 'r2': r2_exp, 'y_fit': y_fit_exp}
    except:
        print("Exponential fit failed")
    
    # Power law fit
    try:
        popt_pow, _ = curve_fit(power_func, x, y)
        y_fit_pow = power_func(x, *popt_pow)
        r2_pow = r_squared(y, y_fit_pow)
        fits['power'] = {'coeffs': popt_pow, 'r2': r2_pow, 'y_fit': y_fit_pow}
    except:
        print("Power law fit failed")
    
    # Find best fit
    best_fit = max(fits.items(), key=lambda x: x[1]['r2'])
    
    # Print results
    print("\nBest fit type:", best_fit[0])
    print(f"R² value: {best_fit[1]['r2']:.6f}")
    
    if best_fit[0].startswith('poly'):
        coeffs = best_fit[1]['coeffs']
        print("\nPolynomial coefficients (highest to lowest order):")
        print([f"{c:.6f}" for c in coeffs])
        print("\nFor use in ESP32 code:")
        for i, c in enumerate(reversed(coeffs)):
            print(f"float c{i} = {c:.6f}f;")
    else:
        coeffs = best_fit[1]['coeffs']
        print("\nFunction coefficients (a, b, c):")
        print([f"{c:.6f}" for c in coeffs])
        print("\nFor use in ESP32 code:")
        print(f"float a = {coeffs[0]:.6f}f;")
        print(f"float b = {coeffs[1]:.6f}f;")
        print(f"float c = {coeffs[2]:.6f}f;")
    
    # Plot results
    plt.figure(figsize=(12, 8))
    plt.scatter(x, y, label='Raw Data', alpha=0.5)
    plt.plot(x, best_fit[1]['y_fit'], 'r-', label=f'Best Fit ({best_fit[0]})')
    eq_text = get_equation_text(
            best_fit[0], 
            best_fit[1]['coeffs'], 
            best_fit[1]['r2']
    )
    plt.text(0.95, 0.95, eq_text,
            transform=plt.gca().transAxes,
            verticalalignment='top',
            horizontalalignment='right',
            bbox=dict(facecolor='white', alpha=0.8))
    
    plt.xlabel('Voltage (mV)')
    plt.ylabel('Flow (arbitrary units)')
    plt.title('MAF Sensor Transfer Function')
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    main()