import sem
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pickle


if __name__ == '__main__':

    # sem.gridrunner.BUILD_GRID_PARAMS =\
    #    "-m a -P 10_Signet -q LOW@runner-01"
    # any intel CPU should be fine
    # sem.gridrunner.SIMULATION_GRID_PARAMS =\
    #    "-m a -P 10_Signet -q LOW@runner-01,LOW@runner-02,LOW@runner-03"

    sem.parallelrunner.MAX_PARALLEL_PROCESSES = 32

    ns_path = "ns3-mmwave-thz"
    script = "thz-transport-options-no-rts"
    campaign_dir = "thz-thz-tcp-cwnd"
    campaign_thz = sem.CampaignManager.new(
        ns_path, script, campaign_dir, runner_type="ParallelRunner",
        overwrite=False, check_repo=False)

    print(campaign_thz)

    numRuns = 1
    param_combination_thz = {
        "ipi": [1000],
        "packetSize": [15000],
        "transport": "tcp",
        "rts": [True, False],
        "ueDist": [1],
        "RngRun": list(range(numRuns))
    }

    print("Run param_combination_thz_udp")
    campaign_thz.run_missing_simulations(
        param_combination_thz
    )

    script = "mmwave-transport"
    campaign_dir = "thz-mmwave-tcp-cwnd"
    campaign_mmwave = sem.CampaignManager.new(
        ns_path, script, campaign_dir, runner_type="ParallelRunner",
        overwrite=False, check_repo=False)

    print(campaign_mmwave)

    numRuns = 1
    param_combination_mmwave = {
        "ipi": [1000],
        "packetSize": [1500],
        "transport": "tcp",
        "ueDist": [1],
        "RngRun": list(range(numRuns))
    }

    print("Run campaign_mmwave")
    campaign_mmwave.run_missing_simulations(
        param_combination_mmwave
    )

