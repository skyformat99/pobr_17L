#include "Features.hpp"
#include <sstream>

/*
 * Calculate normal moments for the given contour
 */
void Features::calc_normal_moments(const cv::Mat& contour) {
    Moments m(4, std::vector<double>(4, 0.));
    for (int i = 0; i < contour.rows; ++i) {
        const unsigned char* g_i = contour.ptr<unsigned char>(i);
        for (int j = 0; j < contour.cols; ++j) {
            if (g_i[j] != 0) {
                for (int p = 0; p < 4; ++p) {
                    for (int q = 0; q < 4; ++q) {
                        m[p][q] += pow(i, p) * pow(j, q);
                    }
                }
            }
        }
    }
    this->normal_moments = m;
}

/*
 * Calculate central moments based on the normal moments for the contour
 */
void Features::calc_central_moments() {
    Moments M(4, std::vector<double>(4, 0.));
    Moments m = this->normal_moments;
    double i_c = m[1][0] / m[0][0];
    double j_c = m[0][1] / m[0][0];

    M[0][0] = m[0][0];
    M[1][1] = m[1][1] - m[1][0]*m[0][1] / m[0][0];

    M[2][0] = m[2][0] - pow(m[1][0], 2) / m[0][0];
    M[0][2] = m[0][2] - pow(m[0][1], 2) / m[0][0];
    M[2][1] = m[2][1] - 2.0*m[1][1]*i_c - m[2][0]*j_c + 2.0*m[0][1]*pow(i_c, 2);
    M[1][2] = m[1][2] - 2.0*m[1][1]*j_c - m[0][2]*i_c + 2.0*m[1][0]*pow(j_c, 2);

    M[0][3] = m[0][3] - 3.0*m[0][2]*j_c + 2.0*m[0][1] *pow(j_c, 2);
    M[3][0] = m[3][0] - 3.0*m[2][0]*i_c + 2.0*m[1][0]*pow(i_c, 2);
    this->central_moments = M;
}

/*
 * Calculate moment invariants based on the normal and central moments of the
 * contour
 */
void Features::calc_moment_invariants() {
    Invariants I(11, 0.);
    Moments& m = this->normal_moments;
    Moments& M = this->central_moments;

    I[1] = (M[2][0] + M[0][2]) / pow(m[0][0], 2);
    I[2] = (pow(M[2][0] - M[0][2], 2) + 4.0 * pow(M[1][1], 2)) / pow(m[0][0], 4);
    I[3] = (pow(M[3][0] - 3.0 * M[1][2], 2) + pow(3.0 * M[2][1] - M[0][3], 2)) / pow(m[0][0], 5);
    I[4] = (pow(M[3][0] + M[1][2], 2) + pow(M[2][1] + M[0][3], 2)) / pow(m[0][0], 5);
    I[5] = ((M[3][0] - 3.0 * M[1][2]) * (M[3][0] + M[1][2])
            * (pow(M[3][0] + M[1][2], 2) - 3.0 * pow(M[2][1] + M[0][3], 2))
            + (3.0 * M[2][1] - M[0][3]) * (M[2][1] + M[0][3])
            * (3.0 * pow(M[3][0] + M[1][2], 2) - pow(M[2][1] + M[0][3], 2)))
           / pow(m[0][0], 10);
    I[6] = ((M[2][0] - M[0][2]) * (pow(M[3][0] + M[1][2], 2) - pow(M[2][1] + M[0][3], 2))
            + 4.0 * M[1][1] * (M[3][0] + M[1][2]) * (M[2][1] + M[0][3]))
           / pow(m[0][0], 7);
    I[7] = (M[2][0] * M[0][2] - pow(M[1][1], 2)) / pow(m[0][0], 4);
    I[8] = (M[3][0] * M[1][2] + M[2][1] * M[0][3] - pow(M[1][2], 2) - pow(M[2][1], 2)) / pow(m[0][0], 5);
    I[9] = (M[2][0] * (M[2][1] * M[0][3] - pow(M[1][2], 2))
            + M[0][2] * (M[0][3] * M[1][2] - pow(M[2][1], 2))
            - M[1][1] * (M[3][0] * M[0][3] - M[2][1] * M[1][2]))
           / pow(m[0][0], 7);
    I[10] = (pow(M[3][0] * M[0][3] - M[2][1] * M[1][2], 2)
             - 4.0 * (M[3][0] * M[1][2] - pow(M[2][1], 2)) * (M[0][3] * M[2][1] - M[1][2]))
            / pow(m[0][0], 10);
    this->invariants = I;
}

/*
 * Calculate area of the contour
 */
unsigned Features::get_area(const cv::Mat& mat) const {
    unsigned area = 0;
    for (int i = 0; i < mat.rows; ++i) {
        const unsigned char* g_i = mat.ptr<unsigned char>(i);
        for (int j = 0; j < mat.cols; ++j) {
            if (g_i[j] != 0) {
                ++area;
            }
        }
    }
    return area;
}

/*
 * Calculate various coefficients for the contour
 */
void Features::calc_coefficients(unsigned S, unsigned L) {
    // 0, 1: circularity coefficients
    // 2: Malinowska's coefficient
    // 3: Mz coefficient
    Coefficients c(4, 0.);

    c[0] = 2 * sqrt(S / M_PI);
    c[1] = L / M_PI;

    c[2] = (L / (2 * sqrt(M_PI * S))) - 1;

    c[3] = (2 * sqrt(M_PI * S)) / L;

    this->coeffs = c;
}

/*
 * C-tor for Features, calculates all possible features
 */
Features::Features(const cv::Mat& contour, unsigned perimeter, unsigned image_id, unsigned contour_id) {
    this->contour_id = contour_id;
    this->image_id = image_id;
    this->calc_normal_moments(contour);
    this->calc_central_moments();
    unsigned area = get_area(contour);
    this->calc_coefficients(area, perimeter);
}

/*
 * Create header for the csv representation of the features
 */
std::string Features::get_csv_header() const {
    std::stringstream ss;
    ss << "id,";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            ss << "normal" << i << j << ",";
        }
    }

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            ss << "central" << i << j << ",";
        }
    }

    for (size_t i = 0; i < invariants.size(); ++i) {
        ss << "invariant" << i << ",";
    }

    for (size_t i = 0; i < coeffs.size(); ++i) {
        ss << "coeff" << i << ",";
    }

    std::string s = ss.str();
    // remove the last comma
    s.pop_back();
    return s;
}

/*
 * Create csv representation of the calculated features
 */
std::string Features::as_csv_row() const {
    std::stringstream ss;
    ss << this->image_id << "_" << this->contour_id << ",";

    for (auto& ms : this->normal_moments) {
        for (auto& m : ms) {
            ss << m << ",";
        }
    }

    for (auto& ms : this->central_moments) {
        for (auto& m : ms) {
            ss << m << ",";
        }
    }

    for (auto i : this->invariants) {
        ss << i << ",";
    }

    for (auto i : this->coeffs) {
        ss << i << ",";
    }

    std::string s = ss.str();
    // remove the last comma
    s.pop_back();
    return s;
}

/*
 * Get features for each contour in the collection
 */
std::vector<Features> get_features_for_contours(const cv::Mat& image, unsigned image_id, const Contours& contours) {
    std::vector<Features> f;
    unsigned id = 0;
    for (auto& s : contours) {
        // get roi from contour
        cv::Mat roi(image, s.first);
        f.push_back(Features(roi, s.second, image_id, id++));
    }
    return f;
}
