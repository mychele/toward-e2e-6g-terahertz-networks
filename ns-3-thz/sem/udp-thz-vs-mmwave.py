import pickle
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import tikzplotlib
import numpy as np


def main():
    # save_tuple = (udp_th_average, udp_th_std,
    #     udp_latency_average, udp_latency_std)
    with open('thz-udp-blade.pickle', 'rb') as f:
        save_tuple_5_30_thz = pickle.load(f)
        udp_th_average_5_30 = save_tuple_5_30_thz[0]
        udp_th_std_5_30 = save_tuple_5_30_thz[1]
    with open('thz-udp-ns381.pickle', 'rb') as f:
        save_tuple_20_thz = pickle.load(f)
        udp_th_average_20 = save_tuple_20_thz[0]
        udp_th_std_20 = save_tuple_20_thz[1]
    with open('thz-udp-ns382.pickle', 'rb') as f:
        save_tuple_10_thz = pickle.load(f)
        udp_th_average_10 = save_tuple_10_thz[0]
        udp_th_std_10 = save_tuple_10_thz[1]
    with open('mmwave-udp-blade.pickle', 'rb') as f:
        save_tuple_mmwave = pickle.load(f)
        udp_th_average_mmw = save_tuple_mmwave[0]
        udp_th_std_mmw = save_tuple_mmwave[1]

    numRunsThz = 5
    plt.figure(figsize=[6, 6], dpi=100)
    legend_entries = []
    for ipival in [5, 30]:
        avg = np.ravel(udp_th_average_5_30.sel(ipi=ipival).values / 1e6)
        std = np.ravel(udp_th_std_5_30.sel(ipi=ipival).values / 1e6)
        plt.errorbar(
            x=[1, 3, 5, 10, 20],
            y=avg, yerr=1.96 * std / np.sqrt(numRunsThz))
        legend_entries += ['ipi = %d' % (ipival)]
    for ipival in [10]:
        avg = np.ravel(udp_th_average_10.sel(ipi=ipival).values / 1e6)
        std = np.ravel(udp_th_std_10.sel(ipi=ipival).values / 1e6)
        plt.errorbar(
            x=[1, 3, 5, 10, 20],
            y=avg, yerr=1.96 * std / np.sqrt(numRunsThz))
        legend_entries += ['ipi = %d' % (ipival)]
    for ipival in [20]:
        avg = np.ravel(udp_th_average_20.sel(ipi=ipival).values / 1e6)
        std = np.ravel(udp_th_std_20.sel(ipi=ipival).values / 1e6)
        plt.errorbar(
            x=[1, 3, 5, 10, 20],
            y=avg, yerr=1.96 * std / np.sqrt(numRunsThz))
        legend_entries += ['ipi = %d' % (ipival)]
    plt.legend(legend_entries)
    plt.xlabel('distance')
    plt.ylabel('Throughput [Mbit/s]')
    plt.savefig('udp-throughput-thz.png')
    tikzplotlib.save("udp-throughput-thz.tex")

    numRunsMmWave = 20
    plt.figure(figsize=[6, 6], dpi=100)
    legend_entries = []
    for ipival in [10, 20, 50, 100, 1000]:
        avg = np.ravel(udp_th_average_mmw.sel(ipi=ipival).values / 1e6)
        std = np.ravel(udp_th_std_mmw.sel(ipi=ipival).values / 1e6)
        plt.errorbar(
            x=[1, 5, 10, 20, 30, 50, 100, 200],
            y=avg, yerr=1.96 * std / np.sqrt(numRunsMmWave))
        legend_entries += ['ipi = %d' % (ipival)]
    plt.legend(legend_entries)
    plt.xlabel('distance')
    plt.ylabel('Throughput [Mbit/s]')
    plt.savefig('udp-throughput-mmwave.png')
    tikzplotlib.save("udp-throughput-mmwave.tex")


if __name__ == '__main__':
    main()
