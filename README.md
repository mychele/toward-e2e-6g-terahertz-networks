## Toward End-to-End, Full-Stack 6G Terahertz Networks

This repository contains the code to generate the results for the paper M. Polese, J. Jornet, T. Melodia, M. Zorzi, “Toward End-to-End, Full-Stack 6G Terahertz Networks”, https://arxiv.org/abs/2005.07989, 2020.

__Please cite the paper if you plan to use the scripts in your publication.__

## Instructions
The scripts are organized in three folders:
- `matlab-channel-coverage` contains the MATLAB code that implements the channel models from [1] and [2], and generates Fig. 2a and Fig. 3 of the paper.
- `ia_thz_paper` containts the MATLAB code to generate Fig. 2b, based on the model from [3]
- `ns-3-thz` contains the mmWave and terahertz scripts for the ns-3 scenarios. To use them, clone the [ns-3 mmWave module](https://github.com/nyuwireless-unipd/ns3-mmwave) and place the [ns-3 TeraSim module](https://github.com/UBnano-Terasim/thz) in the `contrib` folder. The scripts can go in the `scratch` folder or can be added as examples in the thz or mmWave module folders. We also provide the [sem](https://github.com/signetlabdei/sem) scripts that can be use to automatically run the simulations and analyze the results for Fig. 4 and 5 of the paper.

[1] J. M. Jornet and I. F. Akyildiz, “Channel Modeling and Capacity Analysis for Electromagnetic Wireless Nanonetworks in the Terahertz Band,” IEEE Trans. Wireless Commun., vol. 10, no. 10, pp. 3211–3221, Oct. 2011.

[2] 3GPP, “TR 38.901, Study on channel model for frequencies from 0.5 to 100 GHz, V16.1.0,” 2020.

[3] M. Giordani, M. Polese, A. Roy, D. Castor, and M. Zorzi, “A Tutorial on Beam Management for 3GPP NR at mmWave Frequencies,” IEEE Commun. Surveys Tuts., vol. 21, no. 1, pp. 173–196, First Quarter 2019.
