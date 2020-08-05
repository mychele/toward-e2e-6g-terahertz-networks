function [result] = get_poisson_value(lambda)

result = 0;
while(result ==0 )

    k=1; produ = 1;
    produ = produ*rand(1,1);
    while produ >= exp(-lambda)
        produ = produ*rand(1);
        k = k+1;
    end
    result = k-1;
end

end