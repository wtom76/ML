import numpy as np
import pandas as pd
from sqlalchemy import create_engine
import matplotlib.pyplot as plt

axis_series_names = ['sber', 'sber']

df_raw = pd.read_sql_table('finam_daily', 'postgres://ml_user:ml_user@localhost:5432/ML', schema='original', index_col='date')
df_norm = pd.read_sql_table('daily_0001', 'postgres://ml_user:ml_user@localhost:5432/ML', schema='ready', index_col='date')
df_merged = df_norm.merge(df_raw, left_index = True, right_index = True, suffixes=('_n', '_r'))


df_merged.plot(y=['sber_o_n', 'sber_o_r'], subplots=True, kind='line')
plt.show()
