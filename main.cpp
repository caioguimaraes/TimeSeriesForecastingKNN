#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "TimeSeries.h"

using namespace std;

int h;

int main()
{
	cout << "Running . . . ";

	// Reads the input data from a file in the current directory
	ifstream is(string(_getcwd(NULL, 0)) + "/series1.txt");

	// Stores the input data as a vector of double (the input is supposed to be
	// a list of real numbers)
	istream_iterator<double> start(is), end;
	vector<double> temp(start, end);

	// The first figure of the input data is the maximum forecasting horizon;
	// that is, the algorithm will calculate forecasts for the horizons of 1, 2, ..., h periods
	h = (int) temp[0];

	try 
	{
		// Eliminates the last h figures of the time series obtained from the file
		// and converts the remaining time series into a TimeSeries object.
		// These figures are supposed to be unknown for the forecasting algorithm,
		// but they're used later for the evaluation of the forecasting errors
		vector<double> temp2(&temp[1], &temp.at(temp.size()-h));
		TimeSeries ts = TimeSeries(temp2);

		// Stores the last h figures into the vector trueValue
		vector<double> trueValue(&temp[temp.size()-h], &temp[temp.size()-1]);
		trueValue.push_back(temp[temp.size()-1]);
		
		// Procedure which selects the optimum k (the parameter of the k-Nearest Neighbors algorithm)
		// and the optimum l (the length of the segment of the series which is used for forecasting).
		// It takes most of the computation time of the program. The computation time can be ajusted
		// through three constants defined in TimeSeries.cpp: ORDER_OF_MAGNITUDE_K, ORDER_OF_MAGNITUDE_L
		// and PER_MIN_SIZE_TRAIN_SET
		ts.CrossValidation();

		// Clears the screen
		system("CLS");

		cout << "Optimum k = " << ts.GetK() << ", optimum l = " << ts.GetL() << endl << endl;

		// Forecasts the last h figures using the optimum k and the optimum l
		Forecasts forecasts = ts.GetForecasts(ts.GetK(), ts.GetL(), trueValue);

		cout << "Root mean square error = " << forecasts.RMSE << endl << endl;

		cout << "Root mean square percentage error = " << forecasts.RMSPE*100.0 << "%" << endl << endl;

		cout << "True values: " << endl;
		copy(forecasts.trueValue.begin(), forecasts.trueValue.end(), ostream_iterator<double>(cout, " "));
		cout << endl << endl;

		cout << "Forecasts: " << endl;
		copy(forecasts.forecast.begin(), forecasts.forecast.end(), ostream_iterator<double>(cout, " "));
		cout << endl << endl;
	}
	catch (...) 
	{
		cout << "The value of h is too large. Try a new value." << endl;
	}
	
	system("pause");
	return 0;
}