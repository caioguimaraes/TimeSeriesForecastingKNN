The input of this program is a file with real numbers separated by a line break. The fist real number is the number of forecasting horizons that the user would like to consider. The following lines contain the observations of the time series from the first one to the most recent one.

From this input, the program calculates automatically the optimum number of nearest neighbors and the optimum length of the segment of the series which is used for forecasting. The optimum values are chosen by cross validation, which tests for a large arrays of values. The arrays of values tested can me modified in the code through the parameters ORDER_OF_MAGNITUDE_K, ORDER_OF_MAGNITUDE_L and PER_MIN_SIZE_TRAIN_SET

Finally, the program outputs the forecasts, the true values and the errors.