clear all
close all

% based on the model from
% M. Giordani, M. Polese, A. Roy, D. Castor and M. Zorzi, 
% "A Tutorial on Beam Management for 3GPP NR at mmWave Frequencies," 
% in IEEE Communications Surveys & Tutorials, vol. 21, no. 1, pp. 173-196, 
% Firstquarter 2019, doi: 10.1109/COMST.2018.2869411.

%% Get the params
markers = {'+-','o-','*-','.-','x-','s-','d-','^-','v-','>-','<-','p-','h-'};
markersize = 3;
linewidth = 1.2;

% params
n = 4;
delta_f = (15e3)*2.^n; % Hz
T_slot = 1./2.^n; % ms
T_symb = 71.35./(1000*2.^n); % ms

N_ss = [8, 16, 32, 64];
T_ss = [5, 10, 20, 40, 80, 160];
D_max_ss = [5, 2.5]; %ms

n_ant_tx_vector = [16  1024]; % antenna array for BS
n_ant_rx_vector = [4   256]; % antenna array for UE
delta_beam_enb = [26.28 3.17];
delta_beam_ue = [59.83 6.3360];
delta_az_enb = 120;
delta_az_ue = 360;
delta_el = 60;
N_theta_enb = ceil(delta_az_enb./delta_beam_enb);
N_phi_enb = ceil(delta_el./delta_beam_enb);
N_theta_ue = ceil(delta_az_ue./delta_beam_ue);
N_phi_ue = ceil(delta_el./delta_beam_ue);


%% IA

enbSSblocks = N_theta_enb.*N_phi_enb;
ueSSblocks = N_theta_ue.*N_phi_ue;
S_d = enbSSblocks.*ueSSblocks; % eNB analog, UE analog

for nssIndex = 1:length(N_ss)
    for tssIndex = 1:length(T_ss)
        for deltaFreqIndex = 1:length(n)
            N_left = S_d - N_ss(nssIndex)*(ceil(S_d/N_ss(nssIndex)) - 1);
            T_firstPart = T_ss(tssIndex)*(ceil(S_d/N_ss(nssIndex)) - 1);
            
            for nAntIndex = 1:length(n_ant_tx_vector)
                T_ia{nssIndex, tssIndex, deltaFreqIndex}(nAntIndex) = ...
                    T_firstPart(nAntIndex) + ...
                    t_last(N_left(nAntIndex), T_slot(deltaFreqIndex), T_symb(deltaFreqIndex));         
            end
            
        end
    end
end

%% eNB analog UE analog combination
figure,
tssIndex = 3;

for nAntIndex = 1:length(n_ant_tx_vector)
    for nssIndex = 1:length(N_ss)
        nssVecBfNant(nssIndex) = T_ia{nssIndex, tssIndex, deltaFreqIndex}(nAntIndex);
    end
    semilogy(N_ss, nssVecBfNant/1000, markers{mod(nAntIndex, numel(markers)) + 1}, ...
        'LineWidth', linewidth, 'DisplayName', strcat('N_{tx} = ', num2str(n_ant_tx_vector(nAntIndex)), ...
        ' N_{rx} = ', num2str(n_ant_rx_vector(nAntIndex)), ' T_{ss} = ', num2str(T_ss(tssIndex)), ...
        ' ms')), hold on
end

tssIndex = 1;
for nAntIndex = 1:length(n_ant_tx_vector)
    for nssIndex = 1:length(N_ss)
        nssVecBfNant(nssIndex) = T_ia{nssIndex, tssIndex, deltaFreqIndex}(nAntIndex);
    end
    semilogy(N_ss, nssVecBfNant/1000, markers{mod(nAntIndex, numel(markers)) + 1}, ...
        'LineWidth', linewidth, 'DisplayName', strcat('N_{tx} = ', num2str(n_ant_tx_vector(nAntIndex)), ...
        ' N_{rx} = ', num2str(n_ant_rx_vector(nAntIndex)), ' T_{ss} = ', num2str(T_ss(tssIndex)), ...
        ' ms')), hold on
end
%plot(N_ss, 5*ones(1, length(N_ss)), 'DisplayName', 'SS burst duration')
%title('eNB analog UE analog')

grid on
legend('-DynamicLegend');
ylabel('T_{IA} [s]')
xlabel('N_{ss}'),
xlim([min(N_ss) - 1, max(N_ss) + 1])

% matlab2tikz('initial_access_nr_thz.tex', 'width', '\fwidth', 'height', '\fheight');


%% aux function
function [ out ] = t_last( nss, t_slot, t_symb )

if(mod(nss, 2) == 0)
	out = nss/2*t_slot - 2*t_symb;
else
	out = floor(nss/2)*t_slot + 6*t_symb;
end

end
