function [ loss ] = getAbsLoss( f, d, HITRANparams )

% get the parameter of the Beer Lambert law from the HITRAN database files

[minDiff, closestFreqIndex] = min(abs(HITRANparams(:, 1) - f));

kfParam = 0;
if (abs(minDiff) < 9.894e8) % this 9.894e8 is from the 
	kfParam = HITRANparams(closestFreqIndex, 2);
end

loss = kfParam * d * 10 * log10(exp(1));

end

