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
	string fileName;
	cout << "Enter the name of a text file containing a forecasting horizon followed by a real (univariate) time series (the provided examples are series1.txt, series2.txt and series3.txt):" << endl;
	cout.flush();
	cin >> fileName;
	
	
	// Reads the input data from a file in the current directory
	//ifstream is(string(_getcwd(NULL, 0)) + "/series1.txt");
	ifstream is;

	is.open(string(_getcwd(NULL, 0)) + "/" + fileName);

	while (!is)
	{
		cout << endl << "The file name you entered is invalid. Type a valid file name (for instance, series1.txt):" << endl;
		cin >> fileName;
		is.open(string(_getcwd(NULL, 0)) + "/" + fileName);
	}
	
	// Stores the input data as a vector of double (the input is supposed to be
	// a list of real numbers)
	istream_iterator<double> start(is), end;
	vector<double> temp(start, end);

	// The first figure of the input data is the maximum forecasting horizon;
	// that is, the algorithm will calculate forecasts for the horizons of 1, 2, ..., h periods
	h = (int) temp[0];

	cout << endl << endl << "Calculating the optimal parameters of the kNN algorithm and the forecasts for the " << h  << " forecasting horizons . . . ";

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

		cout << endl << endl << endl << endl << "For the kNN algorithm:" << endl << endl << "- The optimum k (the number of nearest neighbors) is " << ts.GetK() << endl << endl << "- The optimum l (the length of the segment of the series which is used for forecasting) is " << ts.GetL() << endl << endl << endl;

		// Forecasts the last h figures using the optimum k and the optimum l
		Forecasts forecasts = ts.GetForecasts(ts.GetK(), ts.GetL(), trueValue);

		cout << "The true values of the last " << h << " observations (supposed unknown for the kNN algorithm) of the time series are:" << endl;
		copy(forecasts.trueValue.begin(), forecasts.trueValue.end(), ostream_iterator<double>(cout, " "));
		cout << endl << endl << endl;

		cout << "The forecasts of the kNN algorithm for the observations presente above are: " << endl;
		copy(forecasts.forecast.begin(), forecasts.forecast.end(), ostream_iterator<double>(cout, " "));
		cout << endl << endl << endl;

		cout << "The root mean square error associated with the forecasts is " << forecasts.RMSE << "." << endl << endl << endl;

		cout << "The root mean square percentage error associated with the forecasts is " << forecasts.RMSPE*100.0 << "%" << "." << endl << endl << endl;
	}
	catch (...) 
	{
		cout << "The value of h is too large. Try a new value." << endl;
	}

	system("pause");
	return 0;
}