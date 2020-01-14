import numpy as np
import pandas as pd
from sqlalchemy import create_engine
import matplotlib.pyplot as plt

# 'imoex_ft_delta', 'sber_ft_delta', 'djind_ft_delta'
axis_series_names = ['djind_ft_delta', 'sber_target']

df = pd.read_sql_table('daily_0001', 'postgres://ml_user:ml_user@localhost:5432/ML', schema='ready', index_col='date')

print(df.head())

#ax1 = df.plot.scatter('imoexft_delta', 'sberft_delta')

plt.scatter(df[axis_series_names[0]], df[axis_series_names[1]], s=1)
plt.show()