function [pproc] = poisson2d(lambda)
% the number of points is Poisson(lambda)-distributed

%npoints = poissrnd(lambda);
npoints = get_poisson_value(lambda);

% conditioned that the number of points is N,
% the points are uniformly distributed
pproc = rand(npoints, 2);
