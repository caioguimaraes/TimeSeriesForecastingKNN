#include "TimeSeries.h"

// The maximum value of k to be tested in the cross validation
// as a percentage of the series size.
#define ORDER_OF_MAGNITUDE_K 0.035

// The maximum value of l to be tested in the cross validation
// as a percentage of the series size.
#define ORDER_OF_MAGNITUDE_L 0.05

// Minimum size of the training set in percentage of observations
// The optimum k is increasing with the number of observations; that's why we use don't
// perform cross-validation in training sets being small relative to the size of the original series.
#define PER_MIN_SIZE_TRAIN_SET 0.80

// Number of observations among the last ones which are excluded from the training process
// in order to avoid overfitting.
#define PER_N_LAST_OBS_EXC 0.05

#define DISTANCE(A, B) ( (A-B)*(A-B) )
#define MAX(A, B) ( (A) > (B) ? (A) : (B))

struct IndexDistance {
	int index;
	double distance;
	IndexDistance(int i, double d) { index = i; distance = d; }
};

bool euclideanDistanceComparison(IndexDistance a, IndexDistance b) { 
	return (a.distance<b.distance); 
}

TimeSeries::TimeSeries()
{

}

TimeSeries::TimeSeries(vector<double> s)
{
	series = s;
}

vector<double> TimeSeries::GetSeries()
{
	return series;
}

int TimeSeries::GetK()
{
	return optimumK;
}

int TimeSeries::GetL()
{
	return optimumL;
}

vector<int> TimeSeries::NearestNeighbors(int k, int l)
{
	vector<int> nn;
	vector<IndexDistance> indexDistance;
	double d;

	for (int i=0; i<(int)series.size()-l; i++)
	{
		// To obtain the nearest neighbors, we consider the euclidean distance between
		// vector of observations having length l and starting at the observation i.
		d=0;
		for (int j=0; j<l; j++)
		{
			d += DISTANCE( series[i+j], series[series.size()-l+j] );
		}

		// We store both the index of the first observation in the segment (i)
		// and the distance of the segment to the last segment in the time series,
		// which will be used for forecasting.
		indexDistance.push_back(IndexDistance(i, d));
	}

	// We sort indexDistance according to the distance of the corresponding segment
	// to the last segment in the time series. Therefore, the first k segments of the
	// sorted vector will be the k nearest neighbors.
	sort(indexDistance.begin(), indexDistance.end(), euclideanDistanceComparison);

	// Obtains the k nearest neighbors.
	for(int i=0; i<k; i++) nn.push_back(indexDistance[i].index);

	return nn;
}

Forecasts TimeSeries::GetForecasts(int k, int l, vector<double> & trueValue)
{
	// Object storing some features of the forecasts being performed.
	Forecasts forecasts;

	// Given the k and the l provided, obtains the k nearest neighbors of
	// the last segment of length l of the time series.
	vector<int> nn = NearestNeighbors(k, l);

	// Vector of forecasts to be calculate for all forecasting horizons 1, 2, ..., h.
	vector<double> forecast(h, 0);
	
	// For the k nearest neighbors algorithm, the forecasts of a segment are the
	// average of the forecasts of its nearest neighbours.
	for (int i=0; i<k; i++)
	{
		for (int j=0; j<h; j++)
		{
			if (nn[i] + l + j < (int)series.size())
				forecast[j] += series[nn[i] + l + j];
			else
				// This is necessary during cross-validation because this procedure is
				// executed for a sub time series, which may not contain all the values
				// which are necessary for calculating forecasts of the neighbors of
				// the last segment of length l of the sub time series.
				forecast[j] += trueValue[nn[i] + l + j - series.size()];
		}
	}

	/* Stores the forecasts and its features. */
	for (int j=0; j<h; j++)
	{
		forecast[j] /= k;
		forecasts.forecast.push_back(forecast[j]);
		forecasts.trueValue.push_back(trueValue[j]);
	}
	
	forecasts.RMSPE = 0;
	for (int j=0; j<h; j++) forecasts.RMSPE += DISTANCE(forecast[j]/trueValue[j], 1);
	forecasts.RMSPE /= h;
	forecasts.RMSPE = sqrt(forecasts.RMSPE);

	forecasts.RMSE = 0;
	for (int j=0; j<h; j++) forecasts.RMSE += DISTANCE(forecast[j], trueValue[j]);
	forecasts.RMSE /= h;
	forecasts.RMSE = sqrt(forecasts.RMSE);
	/* *** */
	
	return forecasts;
}

void TimeSeries::CrossValidation()
{
	double sumRMSPE;
	double minimumSumRMSPE = numeric_limits<double>::infinity();
	int optK=-1;
	int optL=-1;

	// The cross-validation procedure tests the values of k and l specified below.
	// The optimum (k, l) are the values of k and l associated with the lowest sum
	// of aggregated errors across the sub time series considered.
	// For each sub time series and each (k, l), the aggregate error is considered to
	// be the root mean square percentage error; it aggregates the forecast errors 
	// for all forecasting horizons.
	for (int kCandidate=1; kCandidate<(int)(ORDER_OF_MAGNITUDE_K*series.size()); kCandidate+=2)
	{
		for (int lCandidate=1; lCandidate<(int)(ORDER_OF_MAGNITUDE_L*series.size()); lCandidate+=3)
		{
			sumRMSPE = 0;
			for (int i = series.size() - 1 - MAX((int)(PER_N_LAST_OBS_EXC*series.size()), h); i >= (int)(PER_MIN_SIZE_TRAIN_SET*series.size()); i-- )
			{
				vector<double> temp(&series[0], &series[i+1]);

				// Get a sub time series including indices from 0 to i.
				// We don't let i be too low, because the optimum k is increasing
				// with the number of observations. Otherwise, our algorithm
				// could not be convergent
				TimeSeries subTs(temp);

				// If we perform forecasts considering  the maximum forecasting horizon
				// to be h and using the last segment of length l of the sub time series subTs, 
				// the true values of the observations are the observations corresponding
				// to the indices i+1, i+2, ..., i+h of the original series.
				vector<double> trueValue(&series[i+1], &series[i+h+1]);

				sumRMSPE += subTs.GetForecasts(kCandidate, lCandidate, trueValue).RMSPE;
			}
			if (sumRMSPE < minimumSumRMSPE)
			{
				minimumSumRMSPE = sumRMSPE;
				optK = kCandidate;
				optL = lCandidate;
			}
		}
	}
	optimumK = optK;
	optimumL = optL;

	// optimumK == -1 means that the loops were not executed as a result of h being too large.
	if (optimumK == -1)
	{
		cout << "The value of h is too large. Try a new value." << endl;
		system("pause");
		exit(0);
	}
}