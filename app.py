import streamlit as st
import pandas as pd
import plotly.express as px

# --- Configuration Constants for Robustness ---
# Explicitly define the column names expected from the CSV output
PROBE_COLS = ['Chaining_Probes', 'Linear_Probing_Probes', 'Quadratic_Probing_Probes', 'Double_Hashing_Probes']
TIME_COLS = ['Linear_Time_ms', 'Quadratic_Time_ms', 'Double_Time_ms']
ALL_COLS = ['Distribution', 'Load_Factor'] + PROBE_COLS + TIME_COLS

@st.cache_data
def load_data():
    """Loads and reshapes the simulation data from the CSV file."""
    try:
        # **DEFINITIVE FIX:** Skip the header row and explicitly assign column names.
        df = pd.read_csv('results_data.csv', 
                         header=None, # Skip the header row entirely
                         names=ALL_COLS, # Assign the correct column names
                         skiprows=1) # Start reading from the second row (the data)
    except FileNotFoundError:
        st.error("Error: 'results_data.csv' not found. Please ensure the data file is present.")
        return pd.DataFrame()
    except Exception as e:
        st.error(f"Error loading CSV data: {e}")
        return pd.DataFrame()

    # 1. Melt/Reshape for Probes data
    df_probes = df.melt(
        id_vars=['Distribution', 'Load_Factor'],
        value_vars=PROBE_COLS, 
        var_name='Technique',
        value_name='Metric_Value'
    ).assign(Metric_Type='Average Probes')

    # 2. Melt/Reshape for Timing data (only for open addressing)
    df_timing = df.melt(
        id_vars=['Distribution', 'Load_Factor'],
        value_vars=TIME_COLS, 
        var_name='Technique',
        value_name='Metric_Value'
    ).assign(Metric_Type='Insertion Time (ms)')

    # Combine reshaped dataframes
    combined_df = pd.concat([df_probes, df_timing])
    return combined_df

def create_plot(df, metric_type, title, y_axis_label, log_y=False):
    """Creates a Plotly line chart."""
    fig = px.line(
        df,
        x='Load_Factor',
        y='Metric_Value',
        color='Technique',
        line_dash='Distribution',
        title=title,
        template='plotly_dark'
    )
    
    # Customize layout
    fig.update_layout(
        xaxis_title="Load Factor (α)",
        yaxis_title=y_axis_label,
        legend_title="Resolution Technique",
        hovermode="x unified",
        margin=dict(l=20, r=20, t=50, b=20)
    )

    if log_y:
        fig.update_yaxes(type="log", showgrid=True)
    
    return fig

# --- Main Streamlit App ---
st.set_page_config(layout="wide", page_title="Dynamic Hash Table Analyzer")

st.title(" Dynamic Hash Table Analyzer")
st.markdown("Visualize collision resolution performance under varying load factors and data distributions.")

df = load_data()

if not df.empty:
    
    # --- Sidebar Filters ---
    st.sidebar.header("Analysis Filters")
    
    all_distributions = df['Distribution'].unique()
    selected_distributions = st.sidebar.multiselect(
        "Select Data Distribution(s)",
        options=all_distributions,
        default=all_distributions
    )
    
    max_load_factor = st.sidebar.slider(
        "Select Max Load Factor (α)",
        min_value=0.1,
        max_value=1.0,
        value=1.0,
        step=0.05
    )
    
    # Apply filters
    filtered_df = df[
        (df['Distribution'].isin(selected_distributions)) &
        (df['Load_Factor'] <= max_load_factor)
    ]
    
    if filtered_df.empty:
        st.warning("Please select at least one distribution and a valid load factor range.")
    else:
        # --- Tabbed Interface for Metrics ---
        tab_probes, tab_timing = st.tabs(["📊 Average Probes Analysis", "⏱️ Insertion Timing Analysis"])

        with tab_probes:
            st.header("Average Probes per Insertion")
            st.markdown("This chart shows the mean number of accesses (probes) required to insert a key. Lower is better.")
            
            df_probes_plot = filtered_df[filtered_df['Metric_Type'] == 'Average Probes']
            
            y_scale = st.radio("Y-Axis Scale (Probes)", ["Linear", "Logarithmic"], horizontal=True, key='probes_scale')
            
            fig_probes = create_plot(
                df_probes_plot,
                'Average Probes',
                "Collision Resolution Performance: Probes vs. Load Factor",
                "Average Probes",
                log_y=(y_scale == "Logarithmic")
            )
            st.plotly_chart(fig_probes, use_container_width=True)

        with tab_timing:
            st.header("Insertion CPU Time")
            st.markdown("This chart shows the actual CPU time spent during the insertion process (ms).")

            df_timing_plot = filtered_df[filtered_df['Metric_Type'] == 'Insertion Time (ms)']
            
            df_timing_plot = df_timing_plot[~df_timing_plot['Technique'].str.contains('Chaining')]
            
            fig_timing = create_plot(
                df_timing_plot,
                'Insertion Time (ms)',
                "Insertion Time vs. Load Factor (Open Addressing Only)",
                "CPU Time (ms)",
                log_y=False
            )
            st.plotly_chart(fig_timing, use_container_width=True)

        # Optional: Show raw data table
        if st.checkbox('Show Raw Data Table'):
            st.subheader("Raw Simulation Data")
            st.dataframe(filtered_df.sort_values(by=['Distribution', 'Load_Factor']), use_container_width=True)
