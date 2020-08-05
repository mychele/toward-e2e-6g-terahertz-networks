function [ loss ] = getSpreadLoss( f, d )

loss = 20 * log10(4 * pi * f * d / physconst('LightSpeed'));

end
