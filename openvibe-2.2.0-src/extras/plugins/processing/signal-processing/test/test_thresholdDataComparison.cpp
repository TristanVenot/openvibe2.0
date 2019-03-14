///-------------------------------------------------------------------------------------------------
/// 
/// \author Thibaut Monseigne / Inria.
/// 
/// \version 1.0.
/// 
/// \date 19/11/2018.
/// 
/// \copyright <a href="https://choosealicense.com/licenses/agpl-3.0/">GNU Affero General Public License v3.0</a>.
/// 
/// \remarks Based on code of Alison Cellard / Inria.
/// 
///-------------------------------------------------------------------------------------------------


#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <cmath> // std::fabs

using namespace std;

vector<std::string> split(const std::string& s, const std::string& sep)
{
	vector<string> result;
	string::size_type i = 0, j;
	const string::size_type n = sep.size();

	while ((j = s.find(sep, i)) != string::npos)
	{
		result.emplace_back(s, i, j - i);			// Add part
		i = j + n;									// Update pos
	}
	result.emplace_back(s, i, s.size() - 1 - i);	// Last without \n
	return result;
}

/* Method to read a file a store it in a buffer */
bool fileToBuffer(const char* filename, std::vector<double>& output)
{
	output.clear();
	ifstream iFile(filename, ifstream::in);

	if (!iFile.is_open())
	{
		cout << "Error opening file [" << filename << "] for reading" << endl;
		return false;
	}

	// First Line Analyser
	vector< vector<string> > data;
	string line;
	getline(iFile, line);
	string separator = ",";
	vector<string> vLine = split(line, separator);
	if (vLine.size() == 1)
	{
		separator = ";";
		vLine = split(line, separator);
	}
	if (vLine.size() == 1)
	{
		cout << "Error : CSV with an unknown separator or single data (different from ',' and ';')" << endl;
		return false;
	}

	// Store Data
	for (; getline(iFile, line);) { data.push_back(split(line, separator)); }
	iFile.close();

	if (data.empty())
	{
		cout << "Error : No Data in CSV" << endl;
		return false;
	}

	// Select Columns
	size_t columnFirst, columnLast = data[0].size();

	if (separator == ",")
	{				// Standard OpenVibe CSV
		columnFirst = 2;				// Skip "Time:..." and "End Time" column
		columnLast -= 3;				// Skip Event Column
	}
	else
	{									// Legacy CSV. O Legacy, Legacy, wherefore art thou Legacy?
		columnFirst = 1;				// Only Skip "Time (s)" column
		if (data[0].back() == "Sampling Rate")
		{
			columnLast -= 1;			// Skip The "Sampling Rate" column if it exist legacy
		}
	}
	if (columnLast - columnFirst < 1)
	{
		cout << "Error : No useful column in CSV" << endl;
		return false;
	}

	for (const auto& row : data)
	{
		for (size_t i = columnFirst; i < columnLast; ++i)
		{
			if (!row[i].empty())
			{
				output.push_back(stod(row[i]));
			}
		}
	}

	cout << "Found " << data.size() << " rows with " << data[0].size() << " columns ("
		<< columnLast - columnFirst << " use) give " << output.size() << " values." << endl;

	return true;
}


/* Method to compare two files */
bool compareBuffers(const std::vector<double>& inVector, const std::vector<double>& outVector, const double threshold)
{
	// Compare size
	if (inVector.empty() || outVector.empty())
	{
		cout << "Error : no values \n" << endl;
		return false;
	}
	if (inVector.size() != outVector.size())
	{
		cout << "Error : Files have different size, check input data" << endl;
		return false;
	}

	int errorCount = 0, errorIdx = 0;
	double errorMax = 0;
	std::vector<double> difference;

	for (size_t i = 0, n = inVector.size(); i < n; ++i)
	{
		const double err = std::fabs(inVector[i] - outVector[i]);
		if (err > threshold)
		{
			errorCount++;
			if (err > errorMax)
			{
				errorMax = err;
				errorIdx = i;
			}
			difference.push_back(err);
		}
	}

	if (errorCount != 0)
	{
		double mean = 0, var = 0;
		for (const auto& diff : difference)
		{
			mean += diff;
			var += diff * diff;
		}
		mean /= double(errorCount);
		var /= double(errorCount);
		var -= mean * mean;
		cout << "Comparison failed, " << errorCount << " data differ, the largest difference is " << errorMax
			<< " at value [" << errorIdx << "]\n" << endl << inVector[errorIdx] << " ins of " << outVector[errorIdx] << endl
			<< "Error Mean : " << mean << "\t Error Variance : " << var << endl;
		return false;
	}

	return true;
}


// validation Test take in the algorithm output file, the expected output file (reference) and a tolerance threshold
bool validationTest(const char* file1, const char* file2, const double threshold)
{
	std::vector<double> v1, v2;

	if (!fileToBuffer(file1, v1)) { return false; }
	if (!fileToBuffer(file2, v2)) { return false; }
	if (!compareBuffers(v1, v2, threshold)) { return false; }

	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		cout << "Usage: " << argv[0] << " <data> <reference> <threshold>\n";
		return -1;
	}

	const double threshold = strtod(argv[3], nullptr);

	if (!validationTest(argv[1], argv[2], threshold))
	{
		cout << "Algorithm failed validation test \n" << endl;
		return 1;
	}

	cout << "Test passed\n" << endl;
	return 0;
}
