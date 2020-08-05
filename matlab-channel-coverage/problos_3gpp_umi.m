function [probLos] = problos_3gpp_umi(bsPos, utPos)
% 3GPP UMi LOS probability

distance2D = sqrt((utPos(1) - bsPos(1))^2 + (utPos(2) - bsPos(2))^2);
probLos = 18./distance2D + exp(-distance2D/36) .* (1-18./distance2D);
probLos(distance2D <= 18) = 1;
end

