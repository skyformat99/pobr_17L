#include <fstream>
#include <iostream>
#include <iostream>
#include <sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "ImageRecogn.hpp"

static cv::Scalar GREEN = cv::Scalar(0, 160, 0);
static cv::Scalar RED = cv::Scalar(0, 0, 255);

int main(int argc, char *argv[]) {
    bool save_features_to_file = false;
    bool debug_view = false;
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " <filename> [-d]" << std::endl;
    }
    if (argc == 3) {
        debug_view = true;
    }
    cv::Mat image = cv::imread(argv[1]);
    if (image.rows == 0 || image.cols == 0) {
        std::cout << "Something went wrong while trying to read image " << argv[1] << std::endl;
        return -1;
    }
    cv::Mat converted = preprocess(image);
    Segments s = segmentation(converted);
    unsigned image_id = static_cast<unsigned>(round(1000000 * rand()));
    if (debug_view) {
        cv::cvtColor(converted, image, CV_GRAY2RGB);
        cv::putText(image, argv[1], cv::Point(7, image.rows - 7), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2);
    }
    if (s.size() > 0) {
        std::vector<Features> fs = get_features_for_segments(converted, image_id, s);
        if (save_features_to_file) {
            std::ofstream myfile;
            std::stringstream ss;
            ss << argv[1] << "_features.csv";
            std::cout << "Saving to: " << ss.str() << std::endl;
            myfile.open(ss.str());
            myfile << fs[0].get_csv_header() << std::endl;
            for (auto i : fs) {
                myfile << i.as_csv_row() << std::endl;
            }
            myfile.close();
        }
        for (size_t i = 0; i < s.size(); ++i) {
            cv::Rect r = s[i].first;
            cv::Scalar color = classify(fs[i]) ? GREEN : RED;
            if (debug_view) {
                cv::rectangle(
                        image,
                        r,
                        color
                        );
                char t[3];
                snprintf(t, 3, "%d", static_cast<int>(i));
                int x = r.x + 7;
                int y = r.y + r.height - 7;
                cv::putText(image, t, cv::Point(x, y), cv::FONT_HERSHEY_PLAIN, 2, color, 2);
            }
            else if (color == GREEN) {
                cv::rectangle(
                        image,
                        r,
                        color,
                        2
                        );
            }
        }

    }

    cv::imshow("Logo recognition", image);
    cv::waitKey(-1);
    return 0;
}
