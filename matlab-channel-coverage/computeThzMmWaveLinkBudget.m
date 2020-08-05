%% system parameters
mmWaveBandwidth = 400e6; % 400 MHz
thzBandwidth = 50e9; % 50 GHz

distance = [5, 30, 100]; % m

Fdb = 10; % dB
F = 10^(Fdb/10);
T = 290; % K
k = 1.381e-23;

Ptx = 0.5; % W

numSamples = 1000;
freqSupportThz = linspace(0, 1000, numSamples) * 10^9; % Hz
freqSupportMmWave = linspace(24.25, 52.6, numSamples) * 10^9; % Hz

% get the HITRAN matrix
HITRANparams = importdata('data_freq_abscoe.txt');

% addpath('../3GPP38900ChannelModel')
% addpath('../3GPP38900ChannelModel/model')
% addpath('../3GPP38900ChannelModel/utils')

%% compute pathloss and SNR

pathLossThz = zeros(numSamples, length(distance));
snrNoGainThz = zeros(numSamples, length(distance));
lossSpreadDb = zeros(numSamples, length(distance));
lossAbsDb = zeros(numSamples, length(distance));

for freqIndex = 1:numSamples
	for distIndex = 1:length(distance)
		disp(freqSupportThz(freqIndex)), disp(distance(distIndex))
		lossSpreadDb(freqIndex, distIndex) = getSpreadLoss(freqSupportThz(freqIndex), distance(distIndex));
		lossAbsDb(freqIndex, distIndex) = getAbsLoss(freqSupportThz(freqIndex), distance(distIndex), HITRANparams);
		pathLossThz(freqIndex, distIndex) = lossSpreadDb(freqIndex, distIndex) + lossAbsDb(freqIndex, distIndex);
		
		snrNoGainThz(freqIndex, distIndex) = Ptx / (10^(pathLossThz(freqIndex, distIndex) / 10)) / (k * T * F * thzBandwidth);
	end
end


pathLossMmWave = zeros(numSamples, length(distance));
snrNoGainMmWave = zeros(numSamples, length(distance));

scenario = 'UMi';
bsPos = [0,0,10];
uePos = [0,0,1.5];

for freqIndex = 1:numSamples
	for distIndex = 1:length(distance)
		disp(freqSupportMmWave(freqIndex))
		disp(distance(distIndex))
		uePos(1) = distance(distIndex);
		
		probLos = problos_3gpp_umi(bsPos, uePos);
		plLos = pathloss_3gpp_umi(freqSupportMmWave(freqIndex), bsPos, uePos, true);
		plNlos = pathloss_3gpp_umi(freqSupportMmWave(freqIndex), bsPos, uePos, false);
		
		oxyLoss2 = getAbsLoss(freqSupportMmWave(freqIndex), distance(distIndex), HITRANparams);
		pathLossMmWave(freqIndex, distIndex) = probLos * plLos + (1 - probLos) * plNlos + oxyLoss2;
		
		snrNoGainMmWave(freqIndex, distIndex) = Ptx / (10^(pathLossMmWave(freqIndex, distIndex) / 10)) / (k * T * F * mmWaveBandwidth);
	end
end

%% plot
% 
figure, hold all,
for distIndex = 1:length(distance)
	plot(freqSupportMmWave/1e9, 10*log10(snrNoGainMmWave(:, distIndex))), hold on
end
ylim([-100,30])
xlim([24.25, 52.6])
ax1 = gca; % current axes
ax1_pos = ax1.Position; % position of first axes
ax2 = axes('Position',ax1_pos,...
	'XAxisLocation','top',...
	'YAxisLocation','right',...
	'Color','none');
hold(ax2, 'on')
for distIndex = 1:length(distance)
	plot(freqSupportThz/1e9, 10*log10(snrNoGainThz(:, distIndex)), 'Parent', ax2), hold on
end
ylim([-100,30])
xlim([300,1000])
grid on


%% set two carriers for mmwave and thz
mmWaveCarrier = 30e9;
thzCarrier = 440e9;

% find the closest entry in the vectors
[~, indMm] = min(abs(freqSupportMmWave - mmWaveCarrier));
[~, indThz] = min(abs(freqSupportThz- thzCarrier));

snrThzCarrier = snrNoGainThz(indThz, :);
snrMmWaveCarrier = snrNoGainMmWave(indMm, :);

snrRatio = snrMmWaveCarrier ./ snrThzCarrier
snrDiff = 10*log10(snrRatio)

avgDiff = mean(snrDiff)

%% create an antenna object
n_ant_tx_vector = [16  1024]; % antenna array for BS
n_ant_rx_vector = [4   256]; % antenna array for UE
delta_beam_enb = [26.28 3.17];
delta_beam_ue = [59.83 6.3360];
delta_az_enb = 120;
delta_az_ue = 360;
delta_el = 60;

n_ant_tx_vector = [16  1024]; % antenna array for BS
n_ant_rx_vector = [4   256]; % antenna array for UE
delta_beam_enb = [26.28 3.17];
delta_beam_ue = [59.83 6.3360];

%% coverage simulation
counter_lambda = 1;
vector_lambda_bs = [1, 2, 3, 5, 6, 8, 10, 20, 30, 50, 60, 80, ... 
 	100, 150, 200, 250, 300, 350, 400, 450, 500, 550, ...
 	600, 650, 700, 750, 800, 850, 900, 950, 1000, 1100, 1200, 1300, 1400, ...
 	1500, 1600, 1700, 1800, 1900, 2000, 4000, 6000, 8000, 10000, 20000]; % mean BS density / km^2


freqMmWave = 30e9;
freqThz = 430e9;
higherFreqThz = 1500e9;

snrThreshDb = 0; % dB
snrThresh = 10^(snrThreshDb/10); 

n_rep = 5000;
plotScenario = false;

lambda_index = 0;

covThz = zeros(length(vector_lambda_bs), 1);
covThzMmGain = zeros(length(vector_lambda_bs), 1);
covMm = zeros(length(vector_lambda_bs), 1);
covThzHigherFreqMmGain = zeros(length(vector_lambda_bs), 1);
covThzHigherFreq = zeros(length(vector_lambda_bs), 1);

snrThz = zeros(length(vector_lambda_bs), n_rep);
snrThzMmGain = zeros(length(vector_lambda_bs), n_rep);
snrThzHigherFreq = zeros(length(vector_lambda_bs), n_rep);
snrThzHigherFreqMmGain = zeros(length(vector_lambda_bs), n_rep);
snrMmWave = zeros(length(vector_lambda_bs), n_rep);

parfor lambda_index = 1:length(vector_lambda_bs)
	lambda_bs = vector_lambda_bs(lambda_index)
	disp(lambda_bs)
	
	if lambda_bs <= 2000
		areaScenario=250000; % m^2
	elseif lambda_bs <= 200000
		areaScenario=25000; % m^2
	else
		areaScenario=2500; % m^2
	end

	for rep_idx = 1 : n_rep
		%disp(rep_idx)
		% Create locations (PPP) for BSs
		lambda_bss = lambda_bs*areaScenario/1000000;
		bss = poisson2d(lambda_bss).*sqrt(areaScenario);
		n_bss = size(bss,1);
		
		% Create locations (PPP) for UEs
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		% Reference user is always placed in the area center, since on average
		% it does not affect the simulation.
		% ASSUMPTION: consider just one user (then it can be iterated for
		% whatever number of users.
		%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		ue_ref = [sqrt(areaScenario)*0.5,sqrt(areaScenario)*0.5, 1.6];
		ues = ue_ref;
		n_ues = size(ues,1); %total number of users
		
		% Plot UE and BSs
		if plotScenario
			figure
			plot(bss(:,1),bss(:,2),'r.','Marker','s','markersize',20); hold on;
			plot(ues(:,1),ues(:,2),'r.','Marker','x','markersize',10);
			set(gca,'FontSize',18);
			axis([0 sqrt(areaScenario) 0 sqrt(areaScenario)]);
			grid on;
		end
		
		% Compute pathloss for each LINK
		pathloss_values=zeros(n_ues,n_bss,4); % condition, mmWave, terahertz 430 GHz, tera 1500 GHz
		distance=zeros(n_ues,n_bss);
		conditions_random_variable = rand(n_ues,n_bss);
		
		for bs = 1:n_bss
			pos_bs = [bss(bs,:), 10];
			for ue = 1:n_ues
				pos_ue = ues(ue,:);
				distance(ue,bs) = sqrt((pos_ue(2)-pos_bs(2))^2 + (pos_ue(1)-pos_bs(1))^2);  % Distance between BS and MS
				probLos = problos_3gpp_umi(pos_bs, pos_ue);
				condition = (conditions_random_variable(ue,bs) <= probLos);
				
				if condition
					plMmWave = pathloss_3gpp_umi(freqMmWave, pos_bs, pos_ue, true);
					
					lsSpreadDb = getSpreadLoss(freqThz, distance(ue,bs));
					lsAbsDb = getAbsLoss(freqThz, distance(ue,bs), HITRANparams);
					plThz = lsSpreadDb + lsAbsDb;
					
					lsSpreadDb = getSpreadLoss(higherFreqThz, distance(ue,bs));
					lsAbsDb = getAbsLoss(higherFreqThz, distance(ue,bs), HITRANparams);
					plThzHigherFreq = lsSpreadDb + lsAbsDb;
				else
					plMmWave = pathloss_3gpp_umi(freqMmWave, pos_bs, pos_ue, false);
					
					plThz = Inf;
					plThzHigherFreq = Inf;
				end
				oxyLoss2 = getAbsLoss(freqMmWave, distance(ue,bs), HITRANparams);
				pathloss_values(ue,bs,1) = condition;
				pathloss_values(ue,bs,2) = plMmWave + oxyLoss2;
				pathloss_values(ue,bs,3) = plThz;
				pathloss_values(ue,bs,4) = plThzHigherFreq;

			end
		end
		
		% as we account only for pathloss, let's just look for the minimum
		% pathloss for the reference ue
		ue_index = 1;
		minPlMm = min(pathloss_values(ue_index, :, 2));
		minPlThz = min(pathloss_values(ue_index, :, 3));
		minPlThzHigherFreq = min(pathloss_values(ue_index, :, 4));

		snrThz(lambda_index, rep_idx) = Ptx * n_ant_tx_vector(2) * n_ant_rx_vector(2) ...
			/ (10^(minPlThz / 10)) / (k * T * F * thzBandwidth);
		if(snrThz(lambda_index, rep_idx) >= snrThresh)
			covThz(lambda_index) = covThz(lambda_index) + 1;
		end
		snrThzMmGain(lambda_index, rep_idx) = Ptx * n_ant_tx_vector(1) * n_ant_rx_vector(1) ...
			/ (10^(minPlThz / 10)) / (k * T * F * thzBandwidth);
		if(snrThzMmGain(lambda_index, rep_idx) >= snrThresh)
			covThzMmGain(lambda_index) = covThzMmGain(lambda_index) + 1;
		end
		
		snrThzHigherFreq(lambda_index, rep_idx) = Ptx * n_ant_tx_vector(2) * n_ant_rx_vector(2) ...
			/ (10^(minPlThzHigherFreq / 10)) / (k * T * F * thzBandwidth);
		if(snrThzHigherFreq(lambda_index, rep_idx) >= snrThresh)
			covThzHigherFreq(lambda_index) = covThzHigherFreq(lambda_index) + 1;
		end
		snrThzHigherFreqMmGain(lambda_index, rep_idx) = Ptx * n_ant_tx_vector(1) * n_ant_rx_vector(1) ...
			/ (10^(minPlThzHigherFreq / 10)) / (k * T * F * thzBandwidth);
		if(snrThzHigherFreqMmGain(lambda_index, rep_idx) >= snrThresh)
			covThzHigherFreqMmGain(lambda_index) = covThzHigherFreqMmGain(lambda_index) + 1;
		end
		
		snrMmWave(lambda_index, rep_idx) = Ptx * n_ant_tx_vector(1) * n_ant_rx_vector(1) ...
			/ (10^(minPlMm / 10)) / (k * T * F * mmWaveBandwidth);
		if(snrMmWave(lambda_index, rep_idx) >= snrThresh)
			covMm(lambda_index) = covMm(lambda_index) + 1;
		end

		

	end
	covThz(lambda_index) = covThz(lambda_index) / n_rep;
	covThzMmGain(lambda_index) = covThzMmGain(lambda_index) / n_rep;
	covThzHigherFreq(lambda_index) = covThzHigherFreq(lambda_index) / n_rep;
	covThzHigherFreqMmGain(lambda_index) = covThzHigherFreqMmGain(lambda_index) / n_rep;
	covMm(lambda_index) = covMm(lambda_index) / n_rep;
	
	disp(strcat(num2str(lambda_bs), ' completed'))

end

save('teracov_20000.mat')

% 


%% plot
load('teracov_20000.mat')
figure
plot(vector_lambda_bs, covThz, 'x-'), hold on,
plot(vector_lambda_bs, covThzMmGain, 'o-'), hold on,
plot(vector_lambda_bs, covThzHigherFreq, 'v-'), hold on,
plot(vector_lambda_bs, covThzHigherFreqMmGain, '^-'), hold on,
plot(vector_lambda_bs, covMm, '*-'), hold on,
grid on
legend('THz', 'THz with mmWave BF', '1.5 THz', '1.5 THz with mmWave BF', 'mmWave')
