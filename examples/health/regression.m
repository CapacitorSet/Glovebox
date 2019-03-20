# Select people not older than 25
age_limit = 25
# Make the interval fit by shifting left of 120
middle_height = 120
# The degree of the polynomial
poly_degree = 2
height_index = 1
weight_index = 2

raw_data = dlmread('data.csv', ',', 1, 0)
young_data = raw_data(raw_data(:,3) <= age_limit, :)
poly = polyfit(young_data(:, height_index) .- middle_height, young_data(:, weight_index), poly_degree)

clf
hold on
plot(linspace(50 - middle_height, 178 - middle_height), arrayfun(@(x) polyval(poly, x), linspace(50 - middle_height, 178 - middle_height)))
scatter(young_data(:,height_index) .- middle_height, young_data(:,weight_index), '+')