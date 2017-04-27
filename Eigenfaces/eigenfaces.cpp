#include "eigenfaces.hpp"

const string Eigenfaces::TEST_FOLDER = "test\\";
const string Eigenfaces::TRAINING_FOLDER = "training\\";
const string Eigenfaces::LABEL_FILE = "classes.csv";

Eigenfaces::Eigenfaces(string dir):
	dir_(dir)
{
	string path = dir_ + TRAINING_FOLDER + LABEL_FILE;
	processLabelFile(path, true);	

	path = dir_ + TEST_FOLDER + LABEL_FILE;
	processLabelFile(path, false);

	vectorize();
	computeMean();
	computeCovariance();
}

void Eigenfaces::processLabelFile(string path, bool isTraining)
{
	try {
		ifstream input(path);

		//csv processing
		char delim = ';';
		string line;

		while (getline(input, line)) {
			string s;
			vector<string> values;

			for (int i = 0; i < line.size(); i++) {
				if (line[i] != delim) {
					s += line[i];
				}
				else {
					values.push_back(s);
					s.clear();
				}
			}
			if (!s.empty())
				values.push_back(s);

			if (values.size() < 2) {
				throw (std::exception("Invalid csv line"));
			}
			filenames_.push_back(values[0]);
			labels_.push_back(stoi(values[1]));
			if (isTraining)
				trainingIds_.push_back(labels_.size()-1);
			else
				testIds_.push_back(labels_.size() - 1);
		}
	}
	catch (exception &e) {
		cerr << e.what();
	}
}

void Eigenfaces::vectorize()
{
	vector<string> paths(filenames_.size());

	for (auto&& i : trainingIds_) {
		paths[i] = dir_ + TRAINING_FOLDER + filenames_[i];
	}
	for (auto&& i : testIds_) {
		paths[i] = dir_ + TEST_FOLDER + filenames_[i];
	}

	cvMats_.reserve(labels_.size());
	images_.reserve(labels_.size());
	for (int i = 0; i < paths.size(); i++) {
		cvMats_.push_back(imread(paths[i], IMREAD_GRAYSCALE));

		Image image;
		for (int r = 0; r < cvMats_[i].rows; r++) {
			image.insert(image.end(), cvMats_[i].ptr<uchar>(r), cvMats_[i].ptr<uchar>(r) + cvMats_[i].cols);
		}
		images_.push_back(image);
	}
}

void Eigenfaces::computeMean()
{
	vector<int> sum(images_[0].size());
	for (auto&& image : images_) {
		for (int i = 0; i < image.size(); i++) {
			sum[i] += image[i];
		}
	}
	for (auto&& i : sum) {
		i /= images_.size();
	}
	mean_.insert(mean_.end(), sum.begin(), sum.end());

	/*Mat mat(cvMats_[0].size(), CV_8U);
	memcpy(mat.data, mean_.data(), mean_.size() * sizeof(uchar));
	imshow("Mean", mat);
	waitKey();*/
}

void Eigenfaces::computeCovariance()
{
	//Constructing array of training images
	MatrixXd A(trainingIds_.size(), imageSize());
	for (int i = 0; i < trainingIds_.size(); i++) {
		for (int j = 0; j < images_[i].size(); j++) {
			A(i, j) = double(images_[i][j] - mean_[j]);
		}
	}

	MatrixXd covariance = (A*A.transpose())/imageSize();
	cout << covariance << endl;
}

int Eigenfaces::imageSize()
{
	return images_[0].size();
}