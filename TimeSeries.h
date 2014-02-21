#include <iostream>
#include <vector>
#include <algorithm> 
#include <iterator>
using namespace std;

#ifndef TIMESERIES_H
#define TIMESERIES_H

extern int h;

struct Forecasts {
	// Root mean square error
	double RMSE;

	// Root mean square percentage error
	double RMSPE;

	// The forecasts for the horizons 1, 2, ..., h
	vector<double> forecast;

	// The true values of observations 1, 2, ..., h period ahead
	vector<double> trueValue;
};

class TimeSeries
{
private:
	vector<double> series;
	
	// The parameter of the k-Nearest Neighbors algorithm
	int optimumK;

	// The length of the segment of the series which is used for forecasting
	int optimumL;

public:
	TimeSeries();
	TimeSeries(vector<double>);
	vector<double> GetSeries();
	int GetK();
	int GetL();
	vector<int> NearestNeighbors(int, int);
	Forecasts GetForecasts(int, int, vector<double> &);
	void CrossValidation();
};

#endif