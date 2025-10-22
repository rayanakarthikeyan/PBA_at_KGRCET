# PBA_at_KGRCET
 Dynamic Hash Table Analyzer
 Project Overview
The Dynamic Hash Table Analyzer is a specialized tool designed to visualize the efficiency and performance degradation of various collision resolution techniques. This project leverages a high-performance C core to run thousands of hash table insertion simulations and uses a dynamic Streamlit/Python front-end for interactive data analysis and visualization.
The application allows users to analyze how the Average Number of Probes changes under increasing Load Factors (α) and different Data Distributions (Uniform vs. Skewed).
 Key Features
 * Hybrid Architecture: High-speed, reproducible simulation logic written in C generates reliable performance data.
 * Four Collision Techniques Compared:
   * Separate Chaining
   * Linear Probing
   * Quadratic Probing
   * Double Hashing
 * Distribution Analysis: Features pre-simulated data for Uniform (random keys) and Skewed (clustered keys) distributions.
 * Interactive Visualization: Utilizes Plotly charts within Streamlit for zoomable, hover-enabled plots that dynamically filter based on user-selected load factor and technique.

Deployed Application
[https://pba-at-kgr.streamlit.app/]
🛠️ Project Structure and File Breakdown
The project follows a decoupled architecture, separating the core computational engine from the visualization layer.
| File Name | Role & Description |
|---|---|
| hash_table_analyzer.c | Core C Engine. Implements all data structures, hash functions, and insertion logic. Generates the raw performance metrics to stdout. |
| app.py | Streamlit Application. Reads the CSV data, handles user filters, and renders the interactive Plotly graphs. |
| results_data.csv | Pre-Generated Data. The static data file containing the performance output from the C simulation (used for deployment). |
| requirements.txt | Lists the necessary Python packages (streamlit, pandas, plotly). |
⚙️ Local Setup and Data Regeneration
To run the application locally or regenerate the performance data, follow these steps.
1. Prerequisites
You must have a C compiler (like GCC) and Python 3 installed.
2. Install Python Dependencies
Install the required visualization libraries:
pip install -r requirements.txt

3. Generate New Simulation Data (Optional)
The deployed application uses the included results_data.csv. To run the C simulation yourself and update the data:
A. Compile the C Code
# Compile the C file, linking the math library (-lm)
gcc hash_table_analyzer.c -o analyzer -lm

B. Run and Capture Output
Execute the compiled program and pipe its CSV output directly to the data file. The C program's log messages are sent to stderr to ensure only clean CSV data is captured by stdout.
./analyzer > results_data.csv

4. Run the Streamlit Application
With the results_data.csv file in the same directory, start the Streamlit server:
streamlit run app.py

The application will automatically open in your browser, consuming the data from your newly generated CSV file.
💻 Simulation Logic (from C)
The C simulation uses the following fixed parameters:
 * Table Size (M): 10007 (A large prime number).
 * Max Insertions (N): 15000 (Allows testing up to \alpha \approx 1.5).
 * Data Points: Recorded every 500 successful insertions.
Hash Functions Used
 * Primary Hash h_1(k):
   
 * Secondary Hash h_2(k) (for Double Hashing):
   
Authors
 * @rayanakarthikeyan - Initial Work & C Engine and Visualization/Streamlit
