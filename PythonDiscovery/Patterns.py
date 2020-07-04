# A Plotly OHLC Chart With A Rangeslider
# This code is adapted from the Plotly site
# Source: https://plot.ly/python/ohlc-charts/

# Import the necessary libraries
import pandas as pd
import numpy as np

from sklearn.decomposition import PCA
from sklearn.preprocessing import scale
from sklearn.cluster import KMeans
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from time import time

df = pd.read_sql_table('finam_daily', 'postgres://ml_user:ml_user@localhost:5432/ML', schema='original', index_col='date')

df_chg = df.pct_change()
shifted_df = df_chg
shifted_df = shifted_df.rename(columns={"sber_o": "open_0", "sber_h": "high_0", "sber_l": "low_0", "sber_c": "close_0"})
target_df = shifted_df

for i in range(3):
    dist = i + 1
    shifted_df = df_chg.shift(dist)
    shifted_df = shifted_df.rename(columns={"sber_o": "open_" + str(dist), "sber_h": "high_" + str(dist), "sber_l": "low_" + str(dist), "sber_c": "close_" + str(dist)})
    target_df = pd.concat([target_df, shifted_df], axis=1)

target_df = target_df.dropna()

print('target_df')
print(type(target_df))
print(target_df)

print('scaling')
target_df = scale(target_df)

print('scaled target_df')
print(type(target_df))
print(target_df.shape)
print(target_df)

# PCA
components_num = 6 #target_df.shape[1]
clasters_num = 3

print('components_num', components_num)

pca = PCA(n_components = components_num)
target_pca = pca.fit(target_df).transform(target_df)

print('explained variance ratio: %s' % str(pca.explained_variance_ratio_))
print('sum of explained variance: %s' % str(sum(pca.explained_variance_ratio_)))
#~PCA

# k-means

def cluster_indices(labels_array, cluster_num):
    return np.where(labels_array == cluster_num)[0]

def bench_k_means(estimator, name, data):
    t0 = time()
    estimator.fit(data)
    print('k-means labels: ', estimator.labels_)

kmeans_all_comp = KMeans(init='k-means++', n_clusters=clasters_num, n_init=10)
bench_k_means(kmeans_all_comp, "k-means++", target_pca)
cluster_0 = target_pca[cluster_indices(kmeans_all_comp.labels_, 0)]
print('cluster 0:', cluster_0)

#~k-means

# #############################################################################
# Visualize the results on PCA-reduced data

reduced_data = PCA(n_components=2).fit_transform(target_df)
kmeans = KMeans(init='k-means++', n_clusters=clasters_num, n_init=10)
kmeans.fit(reduced_data)

# Step size of the mesh. Decrease to increase the quality of the VQ.
h = .02     # point in the mesh [x_min, x_max]x[y_min, y_max].

# Plot the decision boundary. For that, we will assign a color to each
x_min, x_max = reduced_data[:, 0].min() - 1, reduced_data[:, 0].max() + 1
y_min, y_max = reduced_data[:, 1].min() - 1, reduced_data[:, 1].max() + 1
xx, yy = np.meshgrid(np.arange(x_min, x_max, h), np.arange(y_min, y_max, h))

# Obtain labels for each point in mesh. Use last trained model.
Z = kmeans.predict(np.c_[xx.ravel(), yy.ravel()])

# Put the result into a color plot
Z = Z.reshape(xx.shape)
plt.figure(1)
plt.clf()
plt.imshow(Z, interpolation='nearest',
           extent=(xx.min(), xx.max(), yy.min(), yy.max()),
           cmap=plt.cm.Paired,
           aspect='auto', origin='lower')

plt.plot(reduced_data[:, 0], reduced_data[:, 1], 'k.', markersize=2)
# Plot the centroids as a white X
centroids = kmeans.cluster_centers_
plt.scatter(centroids[:, 0], centroids[:, 1],
            marker='x', s=169, linewidths=3,
            color='w', zorder=10)
plt.title('K-means clustering on the digits dataset (PCA-reduced data)\n'
          'Centroids are marked with white cross')
plt.xlim(x_min, x_max)
plt.ylim(y_min, y_max)
plt.xticks(())
plt.yticks(())
plt.show()

# ##################################################################################

# Random test data
#np.random.seed(19680801)
#all_data = [np.random.normal(0, std, size=100) for std in range(1, 4)]
#labels = ['x1', 'x2', 'x3']



fig, ax = plt.subplots(nrows=1, ncols=1, figsize=(9, 4))

# notch shape box plot
#bplot = ax.boxplot(all_data,
#                   notch=True,  # notch shape
#                   vert=True,  # vertical box alignment
#                   patch_artist=True,  # fill with color
#                   labels=labels)  # will be used to label x-ticks

bplot = ax.boxplot(cluster_0,
                   notch=True,  # notch shape
                   vert=True,  # vertical box alignment
                   patch_artist=True,  # fill with color
                   labels=labels)  # will be used to label x-ticks

ax.set_title('Notched box plot')

# fill with colors
colors = ['pink', 'lightblue', 'lightgreen']
for patch, color in zip(bplot['boxes'], colors):
    patch.set_facecolor(color)

# adding horizontal grid lines
ax.yaxis.grid(True)
ax.set_xlabel('Three separate samples')
ax.set_ylabel('Observed values')

plt.show()