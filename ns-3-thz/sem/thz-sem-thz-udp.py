import sem
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pickle


def get_avg_throughput(result):
    rxdata = result['output']['rx-data2.txt']
    # print(rxdata)
    rows = rxdata.split('\n')
    # print(rows)
    # print(type(rxdata))
    # print(type(rows))
    time = []
    size = []
    # delay = []
    for row in rows:
        numbers = row.split('\t')
        if(len(numbers) == 3):
            time.append(float(numbers[0]))
            size.append(float(numbers[1]))
            # delay.append(float(numbers[2].split('ns')[0]))
        # else:
            # print(numbers)

    if(len(rows) > 1):
        tot_elapsed_time = time[-1] - time[0]
        tot_size = sum(size)
        # print("time %f size %f throughput %f" %
        # (tot_elapsed_time, tot_size, tot_size * 8 / tot_elapsed_time))
        if(tot_elapsed_time <= 0):
            return 0
        return tot_size * 8 / tot_elapsed_time
    else:
        return 0
        # print("time %f size %f throughput %f" %
        # (10, 0, 0))


def get_avg_udp_latency(result):
    rxdata = result['output']['rx-data2.txt']
    # print(rxdata)
    rows = rxdata.split('\n')
    # print(rows)
    # print(type(rxdata))
    # print(type(rows))
    # time = []
    # size = []
    delay = []
    for row in rows:
        numbers = row.split('\t')
        if(len(numbers) == 3):
            # time.append(float(numbers[0]))
            # size.append(float(numbers[1]))
            delay.append(float(numbers[2].split('ns')[0]))
        # else:
            # print(numbers)

    if(len(delay) > 0):
        return np.mean(delay)
    else:
        return np.nan


def get_avg_tcp_latency(result):
    rxdata = result['output']['latency-2.txt']
    # print(rxdata)
    rows = rxdata.split('\n')
    # print(rows)
    # print(type(rxdata))
    # print(type(rows))
    # time = []
    # size = []
    delay = []
    for row in rows:
        numbers = row.split('\t')
        if(len(numbers) == 3):
            # time.append(float(numbers[0]))
            # size.append(float(numbers[1]))
            delay.append(float(numbers[2].split('ns')[0]))
        # else:
            # print(numbers)

    if(len(delay) > 0):
        return np.mean(delay)
    else:
        return np.nan


if __name__ == '__main__':

    # sem.gridrunner.BUILD_GRID_PARAMS =\
    #    "-m a -P 10_Signet -q LOW@runner-01"
    # any intel CPU should be fine
    # sem.gridrunner.SIMULATION_GRID_PARAMS =\
    #    "-m a -P 10_Signet -q LOW@runner-01,LOW@runner-02,LOW@runner-03"

    sem.parallelrunner.MAX_PARALLEL_PROCESSES = 32

    ns_path = "ns3-mmwave-thz"
    script = "thz-transport"
    campaign_dir = "thz-thz-udp"
    campaign_udp = sem.CampaignManager.new(
        ns_path, script, campaign_dir, runner_type="ParallelRunner",
        overwrite=True, check_repo=False)

    print(campaign_udp)

    numRuns = 5
    param_combination_thz_udp = {
        "ipi": [5, 30],
        "packetSize": [15000],
        "transport": "udp",
        "ueDist": [1, 3, 5, 10, 20],
        "RngRun": list(range(numRuns))
    }

    print("Run param_combination_thz_udp")
    campaign_udp.run_missing_simulations(
        param_combination_thz_udp
    )

    udp_throughput_results = campaign_udp.get_results_as_xarray(
        param_combination_thz_udp,
        get_avg_throughput,
        'AvgThroughput',
        1)

    udp_latency_results = campaign_udp.get_results_as_xarray(
        param_combination_thz_udp,
        get_avg_udp_latency,
        'AvgLatency',
        1)

    # print(udp_throughput_results)
    udp_th_average = udp_throughput_results.reduce(np.mean, 'RngRun')
    udp_th_std = udp_throughput_results.reduce(np.std, 'RngRun')
    # print(udp_th_average)
    # print(udp_th_std)
    # print(udp_latency_results)
    udp_latency_average = udp_latency_results.reduce(np.nanmean, 'RngRun')
    udp_latency_std = udp_latency_results.reduce(np.nanstd, 'RngRun')
    # print(udp_latency_average)
    # print(udp_latency_std)

    save_tuple = (udp_th_average, udp_th_std,
                  udp_latency_average, udp_latency_std)
    with open('thz-udp-blade.pickle', 'wb') as f:
        pickle.dump(save_tuple, f)

    plt.figure(figsize=[6, 6], dpi=100)
    legend_entries = []
    for ipival in param_combination_thz_udp['ipi']:
        avg = np.ravel(udp_th_average.sel(ipi=ipival).values / 1e6)
        std = np.ravel(udp_th_std.sel(ipi=ipival).values / 1e6)
        print(param_combination_thz_udp['ueDist'])
        print(avg)
        print(std)
        plt.errorbar(
            x=param_combination_thz_udp['ueDist'],
            y=avg, yerr=1.96 * std / np.sqrt(numRuns))
        legend_entries += ['ipi = %d' % (ipival)]
    plt.legend(legend_entries)
    plt.xlabel('distance')
    plt.ylabel('Throughput [Mbit/s]')
    plt.savefig('udp-throughput.png')

    plt.figure(figsize=[6, 6], dpi=100)
    legend_entries = []
    for ipival in param_combination_thz_udp['ipi']:
        avg = np.ravel(udp_latency_average.sel(ipi=ipival).values * 1e3)
        std = np.ravel(udp_latency_std.sel(ipi=ipival).values * 1e3)
        print(param_combination_thz_udp['ueDist'])
        print(avg)
        print(std)
        plt.errorbar(
            x=param_combination_thz_udp['ueDist'],
            y=avg, yerr=1.96 * std / np.sqrt(numRuns))
        legend_entries += ['ipi = %d' % (ipival)]
    plt.legend(legend_entries)
    plt.xlabel('distance')
    plt.ylabel('Latency [ms]')
    plt.savefig('udp-latency.png')
