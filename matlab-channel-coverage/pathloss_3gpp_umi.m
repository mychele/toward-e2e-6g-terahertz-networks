function [pathLossDb] = pathloss_3gpp_umi(freq, bsPos, utPos, isLos)
% 3GPP UMi pathloss

freqGHz = freq/1e9;

distance2D = sqrt((utPos(1) - bsPos(1))^2 + (utPos(2) - bsPos(2))^2);
distance3D = sqrt((utPos(1) - bsPos(1))^2 + (utPos(2) - bsPos(2))^2 + (utPos(3) - bsPos(3))^2);
hUt = utPos(3);
hBs = bsPos(3);

he = 1;
dBP1 = 4 * (hBs - he).*(hUt - he)*freq/3e8;

if (distance2D <= dBP1)
    pathLossDb = 32.4 + 21*log10(distance3D) + 20*log10(freqGHz);
else
    pathLossDb = 32.4 + 40*log10(distance3D) + 20*log10(freqGHz)...
        - 9.5*log10(dBP1.^2 + (hBs-hUt).^2);
end

if(~isLos)
    pathlossNlos = 35.3*log10(distance3D) + 22.4 + ...
        21.3*log10(freqGHz) - 0.3*(hUt-1.5);
    pathLossDb = max( pathLossDb, pathlossNlos);
end

end

