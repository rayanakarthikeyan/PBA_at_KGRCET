import streamlit as st
import pandas as pd
import plotly.express as px
import numpy as np

# --- Configuration ---
st.set_page_config(layout="wide", page_title="Dynamic Hash Table Analyzer")

# Column Definitions for robust CSV loading
ALL_COLS = [
    'Key_Index', 'Load_Factor', 'Scale', 'Distribution',
    'Chaining_Probes', 'Linear_Probing_Probes', 'Quadratic_Probing_Probes', 'Double_Hashing_Probes',
    'Chaining_Time_ms', 'Linear_Probing_Time_ms', 'Quadratic_Probing_Time_ms', 'Double_Hashing_Time_ms'
]

PROBE_COLS = [
    'Chaining_Probes', 'Linear_Probing_Probes', 'Quadratic_Probing_Probes', 'Double_Hashing_Probes'
]

TIME_COLS = [
    'Chaining_Time_ms', 'Linear_Probing_Time_ms', 'Quadratic_Probing_Time_ms', 'Double_Hashing_Time_ms'
]

# Mapping technique names for display
TECHNIQUE_MAP = {
    'Chaining_Probes': 'Separate Chaining',
    'Linear_Probing_Probes': 'Linear Probing',
    'Quadratic_Probing_Probes': 'Quadratic Probing',
    'Double_Hashing_Probes': 'Double Hashing',
    'Chaining_Time_ms': 'Separate Chaining',
    'Linear_Probing_Time_ms': 'Linear Probing',
    'Quadratic_Probing_Time_ms': 'Quadratic Probing',
    'Double_Hashing_Time_ms': 'Double Hashing',
}

@st.cache_data
def load_data():
    """Loads and reshapes the CSV data into a long format for plotting."""
    try:
        # Load data, skipping header and assigning column names explicitly
        df = pd.read_csv(
            'results_data.csv',
            header=None,
            names=ALL_COLS,
            skiprows=1  # Skip the first row (the original header)
        )
    except Exception as e:
        st.error(f"Error loading CSV data: {e}")
        st.stop()

    # --- Reshape Probe Data ---
    df_probes = df.melt(
        id_vars=['Scale', 'Distribution', 'Load_Factor', 'Key_Index'],
        value_vars=PROBE_COLS,
        var_name='Technique_Raw',
        value_name='Metric_Value'
    ).assign(Metric_Type='Total Probes')

    # --- Reshape Timing Data ---
    df_time = df.melt(
        id_vars=['Scale', 'Distribution', 'Load_Factor', 'Key_Index'],
        value_vars=TIME_COLS,
        var_name='Technique_Raw',
        value_name='Metric_Value'
    ).assign(Metric_Type='Insertion Time (ms)')

    # Combine data frames
    df_long = pd.concat([df_probes, df_time], ignore_index=True)

    # Clean up technique names
    df_long['Technique'] = df_long['Technique_Raw'].map(lambda x: TECHNIQUE_MAP[x])
    
    return df_long

df = load_data()


# --- Sidebar Filtering Options ---

st.sidebar.header("Analysis Filters")

# Filter 0: Scale Selector (NEW)
selected_scale = st.sidebar.radio(
    "1. Select Simulation Scale",
    ('Macro (10k keys)', 'Micro (≤10 keys)'),
    horizontal=True,
    help="Macro scale shows real-world performance; Micro scale shows single-key collision effects."
)
scale_filter = 'Macro' if 'Macro' in selected_scale else 'Micro'


# Filter 1: Select Metric Type
selected_metric_type = st.sidebar.radio(
    "2. Select Analysis Metric",
    ('Total Probes', 'Insertion Time (ms)'),
    horizontal=True
)

# Filter 2: Select Distribution
available_distributions = df['Distribution'].unique()
selected_distributions = st.sidebar.multiselect(
    "3. Select Data Distribution(s)",
    available_distributions,
    default=available_distributions
)

# Filter 3: Select Technique
available_techniques = df['Technique'].unique()
selected_techniques = st.sidebar.multiselect(
    "4. Select Collision Technique(s)",
    available_techniques,
    default=available_techniques
)

# Filter 4: Load Factor Range
max_load_factor = st.sidebar.slider(
    "5. Max Load Factor (α)",
    min_value=0.1,
    max_value=1.0,
    value=1.0,
    step=0.05,
    help="Filter the analysis up to this load factor."
)

# --- Apply Filters ---
df_filtered = df[
    (df['Metric_Type'] == selected_metric_type) &
    (df['Scale'] == scale_filter) &
    (df['Distribution'].isin(selected_distributions)) &
    (df['Technique'].isin(selected_techniques)) &
    (df['Load_Factor'] <= max_load_factor)
].copy()


# --- Main App Title and Layout ---
st.title(" Dynamic Hash Table Analyzer")
st.markdown("Visualize collision resolution performance under varying load factors and data distributions.")

st.subheader(f"{selected_metric_type} vs. Load Factor (α) - {scale_filter} Scale")
st.write("The Y-axis shows the **cumulative total** cost of all insertions up to the given Load Factor.")

col1, col2 = st.columns([1, 4])

with col1:
    # Y-Axis Scale Selector
    y_scale = st.radio(
        "Y-Axis Scale",
        ('Linear', 'Logarithmic'),
        horizontal=True,
        key='y_scale_selector'
    )
    y_axis_type = 'log' if y_scale == 'Logarithmic' and selected_metric_type == 'Total Probes' else 'linear'

    # Single-Key Index Selector (for focused analysis)
    max_key_index = int(df_filtered['Key_Index'].max())
    if max_key_index > 1:
        key_index = st.slider(
            "Highlight Key Insertion Index",
            min_value=1,
            max_value=max_key_index,
            value=max_key_index if max_key_index < 10 else 6,
            step=1,
            help="Focuses the chart on a specific insertion step for micro-analysis."
        )
    else:
        key_index = 1

with col2:
    # --- Plot Generation ---
    title = f"Collision Resolution Performance: {selected_metric_type} vs. Load Factor"
    y_label = selected_metric_type
    
    # Create the figure
    fig = px.line(
        df_filtered,
        x='Load_Factor',
        y='Metric_Value',
        color='Technique',
        line_dash='Distribution',
        title=title,
        labels={'Metric_Value': y_label, 'Load_Factor': 'Load Factor (α)'},
        height=600
    )

    # Set Y-axis scale based on user selection
    fig.update_yaxes(type=y_axis_type)

    # Highlight a specific insertion point (Key_Index)
    if key_index in df_filtered['Key_Index'].values:
        df_key = df_filtered[df_filtered['Key_Index'] == key_index]
        fig.add_scatter(
            x=df_key['Load_Factor'],
            y=df_key['Metric_Value'],
            mode='markers',
            marker=dict(size=12, symbol='star', line=dict(width=2, color='Red')),
            name=f'Key Index {key_index}',
            showlegend=False
        )

    st.plotly_chart(fig, use_container_width=True)

# --- Raw Data Section ---
if st.checkbox("Show Raw Data Table", value=False):
    st.subheader(f"Raw Simulation Data ({scale_filter} Scale)")
    st.dataframe(df_filtered)

st.caption("Developed using C for simulation and Python/Streamlit for visualization.")
