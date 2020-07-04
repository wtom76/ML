# A Plotly OHLC Chart With A Rangeslider
# This code is adapted from the Plotly site
# Source: https://plot.ly/python/ohlc-charts/

# Import the necessary libraries
import pandas as pd
import numpy as np

# The section of the Plotly library needed
import plotly.graph_objects as go

df = pd.read_sql_table('finam_daily', 'postgres://ml_user:ml_user@localhost:5432/ML', schema='original', index_col='date')

print (df.head())

# Obtain data from the data frame
fig = go.Figure(data=go.Ohlc(x=df.index,
                    open=df['sber_o'],
                    high=df['sber_h'],
                    low=df['sber_l'],
                    close=df['sber_c']))

# Add title and annotations
fig.update_layout(title_text='sber',
                  title={
                    'y':0.9,
                    'x':0.5,
                    'xanchor': 'center',
                    'yanchor': 'top'},
                  xaxis_rangeslider_visible=True, xaxis_title="Time", yaxis_title="Growth Rate Percentage")


fig.show()





#import pandas as pd
#import numpy as np
#from sqlalchemy import create_engine
#from mpl_finance import candlestick_ohlc
#import matplotlib.pyplot as plt

#axis_series_names = ['sber', 'sber']

#df_raw = pd.read_sql_table('finam_daily', 'postgres://ml_user:ml_user@localhost:5432/ML', schema='original', index_col='date')
#df_merged = df_norm.merge(df_raw, left_index = True, right_index = True, suffixes=('_n', '_r'))


#df_merged.plot(y=['sber_o_n', 'sber_o_r'], subplots=True, kind='line')
#plt.show()
