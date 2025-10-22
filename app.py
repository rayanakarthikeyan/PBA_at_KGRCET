import streamlit as st
import pandas as pd
import plotly.express as px
import numpy as np
import io

# --- CONFIGURATION ---
DATA_FILE = "results_data.csv"

# --- DATA LOADING ---

@st.cache_data
def load_and_process_data(data_path):
    """Loads the pre-generated CSV data file."""
    try:
        df = pd.read_csv(data_path)
        return df
    except FileNotFoundError:
        st.error(f"Data file '{data_path}' not found. Please ensure it is uploaded.")
        st.stop()
    except Exception as e:
        st.error(f"Error loading data: {e}")
        st.stop()

# --- STREAMLIT UI SETUP ---

st.set_page_config(
    page_title="Dynamic Hash Table Analyzer",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Title and Description
st.title("🧮 Dynamic Hash Table Analyzer")
st.markdown("Visualize how different collision resolution techniques perform under varying **load factors** and **data distributions** based on C simulation results.")

# Load the data once
df_raw = load_and_process_data(DATA_FILE)

# --- SIDEBAR CONTROLS ---

st.sidebar.header("Analysis Parameters")

# 1. Select Dataset
dataset_options = df_raw['Distribution'].unique()
selected_dataset = st.sidebar.selectbox(
    "Select data distribution",
    dataset_options,
    format_func=lambda x: f"{x} Keys (Simulated)",
    index=0
)

# 2. Select Collision Resolution Technique
all_techniques = ['Chaining_Probes', 'Linear_Probing_Probes', 'Quadratic_Probing_Probes', 'Double_Hashing_Probes']
selected_techniques = st.sidebar.multiselect(
    "Select Collision Resolution Technique(s)",
    all_techniques,
    default=all_techniques,
    format_func=lambda x: x.replace('_Probes', '').replace('_', ' ').title()
)

# 3. Select Load Factor (Max)
max_alpha = df_raw['Load_Factor'].max()
max_load_factor = st.sidebar.slider(
    "Select Max Load Factor (α)",
    min_value=0.1,
    max_value=max_alpha,
    value=1.0,
    step=0.05
)

st.sidebar.markdown("---")
st.sidebar.markdown(
    """
    **Core Engine:** C Simulation (for performance)\n
    **Data Source:** `results_data.csv`
    """
)

# --- MAIN ANALYSIS ---

if not selected_techniques:
    st.warning("Please select at least one collision resolution technique to visualize.")
else:
    # 1. Filter Data
    df_filtered = df_raw[df_raw['Distribution'] == selected_dataset].copy()
    df_filtered = df_filtered[df_filtered['Load_Factor'] <= max_load_factor]

    # 2. Prepare Data for Plotly (Melt to long format)
    df_plot = df_filtered.melt(
        id_vars=['Load_Factor'],
        value_vars=selected_techniques,
        var_name='Technique',
        value_name='Average Probes'
    )
    
    # 3. Clean up technique names for the legend
    df_plot['Technique'] = df_plot['Technique'].str.replace('_Probes', '').str.replace('_', ' ').str.title()
    
    # 4. PLOTTING
    st.subheader(f"Performance for **{selected_dataset}** Key Distribution")

    # Define color map for consistency
    color_map = {
        'Chaining': '#FF6384',  # Red
        'Linear Probing': '#36A2EB', # Blue
        'Quadratic Probing': '#FFCE56', # Yellow
        'Double Hashing': '#4BC0C0' # Teal
    }

    fig = px.line(
        df_plot,
        x='Load_Factor',
        y='Average Probes',
        color='Technique',
        title='Average Probes (C) vs. Load Factor (α)',
        labels={'Load_Factor': 'Load Factor (α = N/M)', 'Average Probes': 'Avg Probes (Successful Insertion)'},
        color_discrete_map=color_map
    )
    
    # Customize layout
    fig.update_layout(
        legend_title_text='Resolution Technique',
        xaxis_title="Load Factor (α)",
        yaxis_title="Average Probes (C)",
        hovermode="x unified",
        # Use a background closer to the original sample UI
        paper_bgcolor='rgba(0,0,0,0)', 
        plot_bgcolor='rgba(0,0,0,0)',
        font=dict(color="#FFFFFF"),
    )
    fig.update_traces(mode='lines+markers', marker=dict(size=5, line=dict(width=1, color='DarkSlateGrey')))
    fig.update_xaxes(showgrid=True, gridcolor='rgba(255,255,255,0.1)', zeroline=False)
    fig.update_yaxes(showgrid=True, gridcolor='rgba(255,255,255,0.1)', zeroline=False)
    
    st.plotly_chart(fig, use_container_width=True)

    # --- ANALYSIS SUMMARY ---
    st.markdown("---")
    if selected_dataset == "Uniform":
        st.markdown(
            """
            ### Summary of Uniform Data Performance
            In the **uniform** case, performance closely matches theoretical expectations:
            * **Separate Chaining:** Performance is nearly constant, demonstrating its resilience to load factor changes.
            * **Linear Probing:** Shows the **steepest performance degradation** as $\\alpha$ approaches 1, a classic sign of **primary clustering**.
            * **Double/Quadratic Probing:** These perform similarly, significantly better than Linear Probing because they avoid primary clustering.
            """
        )
    elif selected_dataset == "Skewed":
        st.markdown(
            """
            ### Summary of Skewed Data Performance
            When data is intentionally **skewed** (clustered) to cause collisions:
            * **Linear Probing:** The performance drop is **more drastic** than the uniform case, highlighting its vulnerability to non-random data.
            * **Chaining Performance:** Even chaining shows a slightly higher probe count due to increased initial collisions.
            * **Quadratic/Double Hashing:** These methods are much more resilient to poor key distribution than Linear Probing.
            """
        )

    # --- RAW DATA DISPLAY (OPTIONAL) ---
    with st.expander("View Filtered Data Table"):
        st.dataframe(df_filtered.head(10).style.format(precision=4), use_container_width=True)

