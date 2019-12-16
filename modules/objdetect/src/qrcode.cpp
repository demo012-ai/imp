// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2018, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.

#include "precomp.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/calib3d.hpp"

#ifdef HAVE_QUIRC
#include "quirc.h"
#endif

#include <limits>
#include <cmath>
#include <iostream>
#include <queue>

namespace cv
{
using std::vector;

class QRDetect
{
public:
    void init(const Mat& src, double eps_vertical_ = 0.2, double eps_horizontal_ = 0.1);
    bool localization();
    bool computeTransformationPoints();
    Mat getBinBarcode() { return bin_barcode; }
    Mat getStraightBarcode() { return straight_barcode; }
    vector<Point2f> getTransformationPoints() { return transformation_points; }
    static Point2f intersectionLines(Point2f a1, Point2f a2, Point2f b1, Point2f b2);
protected:
    vector<Vec3d> searchHorizontalLines();
    vector<Point2f> separateVerticalLines(const vector<Vec3d> &list_lines);
    void fixationPoints(vector<Point2f> &local_point);
    vector<Point2f> getQuadrilateral(vector<Point2f> angle_list);
    bool testBypassRoute(vector<Point2f> hull, int start, int finish);
    inline double getCosVectors(Point2f a, Point2f b, Point2f c);

    Mat barcode, bin_barcode, resized_barcode, resized_bin_barcode, straight_barcode;
    vector<Point2f> localization_points, transformation_points;
    double eps_vertical, eps_horizontal, coeff_expansion;
    enum resize_direction { ZOOMING, SHRINKING, UNCHANGED } purpose;
};


void QRDetect::init(const Mat& src, double eps_vertical_, double eps_horizontal_)
{
    CV_TRACE_FUNCTION();
    CV_Assert(!src.empty());
    barcode = src.clone();
    const double min_side = std::min(src.size().width, src.size().height);
    if (min_side < 512.0)
    {
        purpose = ZOOMING;
        coeff_expansion = 512.0 / min_side;
        const int width  = cvRound(src.size().width  * coeff_expansion);
        const int height = cvRound(src.size().height  * coeff_expansion);
        Size new_size(width, height);
        resize(src, barcode, new_size, 0, 0, INTER_LINEAR);
    }
    else if (min_side > 512.0)
    {
        purpose = SHRINKING;
        coeff_expansion = min_side / 512.0;
        const int width  = cvRound(src.size().width  / coeff_expansion);
        const int height = cvRound(src.size().height  / coeff_expansion);
        Size new_size(width, height);
        resize(src, resized_barcode, new_size, 0, 0, INTER_AREA);
    }
    else
    {
        purpose = UNCHANGED;
        coeff_expansion = 1.0;
    }

    eps_vertical   = eps_vertical_;
    eps_horizontal = eps_horizontal_;
    adaptiveThreshold(barcode, bin_barcode, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 83, 2);
    adaptiveThreshold(resized_barcode, resized_bin_barcode, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 83, 2);

}

vector<Vec3d> QRDetect::searchHorizontalLines()
{
    CV_TRACE_FUNCTION();
    vector<Vec3d> result;
    const int height_bin_barcode = bin_barcode.rows;
    const int width_bin_barcode  = bin_barcode.cols;
    const size_t test_lines_size = 5;
    double test_lines[test_lines_size];
    vector<size_t> pixels_position;

    for (int y = 0; y < height_bin_barcode; y++)
    {
        pixels_position.clear();
        const uint8_t *bin_barcode_row = bin_barcode.ptr<uint8_t>(y);

        int pos = 0;
        for (; pos < width_bin_barcode; pos++) { if (bin_barcode_row[pos] == 0) break; }
        if (pos == width_bin_barcode) { continue; }

        pixels_position.push_back(pos);
        pixels_position.push_back(pos);
        pixels_position.push_back(pos);

        uint8_t future_pixel = 255;
        for (int x = pos; x < width_bin_barcode; x++)
        {
            if (bin_barcode_row[x] == future_pixel)
            {
                future_pixel = static_cast<uint8_t>(~future_pixel);
                pixels_position.push_back(x);
            }
        }
        pixels_position.push_back(width_bin_barcode - 1);
        for (size_t i = 2; i < pixels_position.size() - 4; i+=2)
        {
            test_lines[0] = static_cast<double>(pixels_position[i - 1] - pixels_position[i - 2]);
            test_lines[1] = static_cast<double>(pixels_position[i    ] - pixels_position[i - 1]);
            test_lines[2] = static_cast<double>(pixels_position[i + 1] - pixels_position[i    ]);
            test_lines[3] = static_cast<double>(pixels_position[i + 2] - pixels_position[i + 1]);
            test_lines[4] = static_cast<double>(pixels_position[i + 3] - pixels_position[i + 2]);

            double length = 0.0, weight = 0.0;

            for (size_t j = 0; j < test_lines_size; j++) { length += test_lines[j]; }

            if (length == 0) { continue; }
            for (size_t j = 0; j < test_lines_size; j++)
            {
                if (j != 2) { weight += fabs((test_lines[j] / length) - 1.0/7.0); }
                else        { weight += fabs((test_lines[j] / length) - 3.0/7.0); }
            }

            if (weight < eps_vertical)
            {
                Vec3d line;
                line[0] = static_cast<double>(pixels_position[i - 2]);
                line[1] = y;
                line[2] = length;
                result.push_back(line);
            }
        }
    }
    return result;
}

vector<Point2f> QRDetect::separateVerticalLines(const vector<Vec3d> &list_lines)
{
    CV_TRACE_FUNCTION();
    vector<Vec3d> result;
    int temp_length;
    vector<Point2f> point2f_result;
    uint8_t next_pixel;
    vector<double> test_lines;


    for (int coeff_epsilon = 1; coeff_epsilon < 10; coeff_epsilon++)
    {
        result.clear();
        temp_length = 0;
        point2f_result.clear();

        for (size_t pnt = 0; pnt < list_lines.size(); pnt++)
        {
            const int x = cvRound(list_lines[pnt][0] + list_lines[pnt][2] * 0.5);
            const int y = cvRound(list_lines[pnt][1]);

            // --------------- Search vertical up-lines --------------- //

            test_lines.clear();
            uint8_t future_pixel_up = 255;

            for (int j = y; j < bin_barcode.rows - 1; j++)
            {
                next_pixel = bin_barcode.ptr<uint8_t>(j + 1)[x];
                temp_length++;
                if (next_pixel == future_pixel_up)
                {
                    future_pixel_up = static_cast<uint8_t>(~future_pixel_up);
                    test_lines.push_back(temp_length);
                    temp_length = 0;
                    if (test_lines.size() == 3) { break; }
                }
            }

            // --------------- Search vertical down-lines --------------- //

            uint8_t future_pixel_down = 255;
            for (int j = y; j >= 1; j--)
            {
                next_pixel = bin_barcode.ptr<uint8_t>(j - 1)[x];
                temp_length++;
                if (next_pixel == future_pixel_down)
                {
                    future_pixel_down = static_cast<uint8_t>(~future_pixel_down);
                    test_lines.push_back(temp_length);
                    temp_length = 0;
                    if (test_lines.size() == 6) { break; }
                }
            }

            // --------------- Compute vertical lines --------------- //

            if (test_lines.size() == 6)
            {
                double length = 0.0, weight = 0.0;

                for (size_t i = 0; i < test_lines.size(); i++) { length += test_lines[i]; }

                CV_Assert(length > 0);
                for (size_t i = 0; i < test_lines.size(); i++)
                {
                    if (i % 3 != 0) { weight += fabs((test_lines[i] / length) - 1.0/ 7.0); }
                    else            { weight += fabs((test_lines[i] / length) - 3.0/14.0); }
                }

                if(weight < eps_horizontal * coeff_epsilon)
                {
                    result.push_back(list_lines[pnt]);
                }
            }
        }
        if (result.size() > 2)
        {
            for (size_t i = 0; i < result.size(); i++)
            {
                point2f_result.push_back(
                      Point2f(static_cast<float>(result[i][0] + result[i][2] * 0.5),
                              static_cast<float>(result[i][1])));
            }

            vector<Point2f> centers;
            Mat labels;
            double compactness;
            compactness = kmeans(point2f_result, 3, labels,
                 TermCriteria( TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1),
                 3, KMEANS_PP_CENTERS, centers);
            if (compactness == 0) { continue; }
            if (compactness > 0)  { break; }
        }
    }
    return point2f_result;
}

void QRDetect::fixationPoints(vector<Point2f> &local_point)
{
    CV_TRACE_FUNCTION();
    double cos_angles[3], norm_triangl[3];

    norm_triangl[0] = norm(local_point[1] - local_point[2]);
    norm_triangl[1] = norm(local_point[0] - local_point[2]);
    norm_triangl[2] = norm(local_point[1] - local_point[0]);

    cos_angles[0] = (norm_triangl[1] * norm_triangl[1] + norm_triangl[2] * norm_triangl[2]
                  -  norm_triangl[0] * norm_triangl[0]) / (2 * norm_triangl[1] * norm_triangl[2]);
    cos_angles[1] = (norm_triangl[0] * norm_triangl[0] + norm_triangl[2] * norm_triangl[2]
                  -  norm_triangl[1] * norm_triangl[1]) / (2 * norm_triangl[0] * norm_triangl[2]);
    cos_angles[2] = (norm_triangl[0] * norm_triangl[0] + norm_triangl[1] * norm_triangl[1]
                  -  norm_triangl[2] * norm_triangl[2]) / (2 * norm_triangl[0] * norm_triangl[1]);

    const double angle_barrier = 0.85;
    if (fabs(cos_angles[0]) > angle_barrier || fabs(cos_angles[1]) > angle_barrier || fabs(cos_angles[2]) > angle_barrier)
    {
        local_point.clear();
        return;
    }

    size_t i_min_cos =
       (cos_angles[0] < cos_angles[1] && cos_angles[0] < cos_angles[2]) ? 0 :
       (cos_angles[1] < cos_angles[0] && cos_angles[1] < cos_angles[2]) ? 1 : 2;

    size_t index_max = 0;
    double max_area = std::numeric_limits<double>::min();
    for (size_t i = 0; i < local_point.size(); i++)
    {
        const size_t current_index = i % 3;
        const size_t left_index  = (i + 1) % 3;
        const size_t right_index = (i + 2) % 3;

        const Point2f current_point(local_point[current_index]),
            left_point(local_point[left_index]), right_point(local_point[right_index]),
            central_point(intersectionLines(current_point,
                              Point2f(static_cast<float>((local_point[left_index].x + local_point[right_index].x) * 0.5),
                                      static_cast<float>((local_point[left_index].y + local_point[right_index].y) * 0.5)),
                              Point2f(0, static_cast<float>(bin_barcode.rows - 1)),
                              Point2f(static_cast<float>(bin_barcode.cols - 1),
                                      static_cast<float>(bin_barcode.rows - 1))));


        vector<Point2f> list_area_pnt;
        list_area_pnt.push_back(current_point);

        vector<LineIterator> list_line_iter;
        list_line_iter.push_back(LineIterator(bin_barcode, current_point, left_point));
        list_line_iter.push_back(LineIterator(bin_barcode, current_point, central_point));
        list_line_iter.push_back(LineIterator(bin_barcode, current_point, right_point));

        for (size_t k = 0; k < list_line_iter.size(); k++)
        {
            uint8_t future_pixel = 255, count_index = 0;
            for(int j = 0; j < list_line_iter[k].count; j++, ++list_line_iter[k])
            {
                if (list_line_iter[k].pos().x >= bin_barcode.cols ||
                    list_line_iter[k].pos().y >= bin_barcode.rows) { break; }
                const uint8_t value = bin_barcode.at<uint8_t>(list_line_iter[k].pos());
                if (value == future_pixel)
                {
                    future_pixel = static_cast<uint8_t>(~future_pixel);
                    count_index++;
                    if (count_index == 3)
                    {
                        list_area_pnt.push_back(list_line_iter[k].pos());
                        break;
                    }
                }
            }
        }

        const double temp_check_area = contourArea(list_area_pnt);
        if (temp_check_area > max_area)
        {
            index_max = current_index;
            max_area = temp_check_area;
        }

    }
    if (index_max == i_min_cos) { std::swap(local_point[0], local_point[index_max]); }
    else { local_point.clear(); return; }

    const Point2f rpt = local_point[0], bpt = local_point[1], gpt = local_point[2];
    Matx22f m(rpt.x - bpt.x, rpt.y - bpt.y, gpt.x - rpt.x, gpt.y - rpt.y);
    if( determinant(m) > 0 )
    {
        std::swap(local_point[1], local_point[2]);
    }
}

bool QRDetect::localization()
{
    CV_TRACE_FUNCTION();
    Point2f begin, end;
    vector<Vec3d> list_lines_x = searchHorizontalLines();
    if( list_lines_x.empty() ) { return false; }
    vector<Point2f> list_lines_y = separateVerticalLines(list_lines_x);
    if( list_lines_y.empty() ) { return false; }

    vector<Point2f> centers;
    Mat labels;
    kmeans(list_lines_y, 3, labels,
           TermCriteria( TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1),
           3, KMEANS_PP_CENTERS, localization_points);

    fixationPoints(localization_points);

    bool suare_flag = false, local_points_flag = false;
    double triangle_sides[3];
    double triangle_perim, square_area, img_square_area;
    if (localization_points.size() == 3)
    {
        triangle_sides[0] = norm(localization_points[0] - localization_points[1]);
        triangle_sides[1] = norm(localization_points[1] - localization_points[2]);
        triangle_sides[2] = norm(localization_points[2] - localization_points[0]);

        triangle_perim = (triangle_sides[0] + triangle_sides[1] + triangle_sides[2]) / 2;

        square_area = sqrt((triangle_perim * (triangle_perim - triangle_sides[0])
                                           * (triangle_perim - triangle_sides[1])
                                           * (triangle_perim - triangle_sides[2]))) * 2;
        img_square_area = bin_barcode.cols * bin_barcode.rows;

        if (square_area > (img_square_area * 0.2))
        {
            suare_flag = true;
        }
    }
    else
    {
        local_points_flag = true;
    }
    if ((suare_flag || local_points_flag) && purpose == SHRINKING)
    {
        localization_points.clear();
        bin_barcode = resized_bin_barcode.clone();
        list_lines_x = searchHorizontalLines();
        if( list_lines_x.empty() ) { return false; }
        list_lines_y = separateVerticalLines(list_lines_x);
        if( list_lines_y.empty() ) { return false; }

        kmeans(list_lines_y, 3, labels,
               TermCriteria( TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1),
               3, KMEANS_PP_CENTERS, localization_points);

        fixationPoints(localization_points);
        if (localization_points.size() != 3) { return false; }

        const int width  = cvRound(bin_barcode.size().width  * coeff_expansion);
        const int height = cvRound(bin_barcode.size().height * coeff_expansion);
        Size new_size(width, height);
        Mat intermediate;
        resize(bin_barcode, intermediate, new_size, 0, 0, INTER_LINEAR);
        bin_barcode = intermediate.clone();
        for (size_t i = 0; i < localization_points.size(); i++)
        {
            localization_points[i] *= coeff_expansion;
        }
    }
    if (purpose == ZOOMING)
    {
        const int width  = cvRound(bin_barcode.size().width  / coeff_expansion);
        const int height = cvRound(bin_barcode.size().height / coeff_expansion);
        Size new_size(width, height);
        Mat intermediate;
        resize(bin_barcode, intermediate, new_size, 0, 0, INTER_LINEAR);
        bin_barcode = intermediate.clone();
        for (size_t i = 0; i < localization_points.size(); i++)
        {
            localization_points[i] /= coeff_expansion;
        }
    }

    for (size_t i = 0; i < localization_points.size(); i++)
    {
        for (size_t j = i + 1; j < localization_points.size(); j++)
        {
            if (norm(localization_points[i] - localization_points[j]) < 10)
            {
                return false;
            }
        }
    }
    return true;

}

bool QRDetect::computeTransformationPoints()
{
    CV_TRACE_FUNCTION();
    if (localization_points.size() != 3) { return false; }

    vector<Point> locations, non_zero_elem[3], newHull;
    vector<Point2f> new_non_zero_elem[3];
    for (size_t i = 0; i < 3; i++)
    {
        Mat mask = Mat::zeros(bin_barcode.rows + 2, bin_barcode.cols + 2, CV_8UC1);
        uint8_t next_pixel, future_pixel = 255;
        int count_test_lines = 0, index = cvRound(localization_points[i].x);
        for (; index < bin_barcode.cols - 1; index++)
        {
            next_pixel = bin_barcode.ptr<uint8_t>(cvRound(localization_points[i].y))[index + 1];
            if (next_pixel == future_pixel)
            {
                future_pixel = static_cast<uint8_t>(~future_pixel);
                count_test_lines++;
                if (count_test_lines == 2)
                {
                    floodFill(bin_barcode, mask,
                              Point(index + 1, cvRound(localization_points[i].y)), 255,
                              0, Scalar(), Scalar(), FLOODFILL_MASK_ONLY);
                    break;
                }
            }
        }
        Mat mask_roi = mask(Range(1, bin_barcode.rows - 1), Range(1, bin_barcode.cols - 1));
        findNonZero(mask_roi, non_zero_elem[i]);
        newHull.insert(newHull.end(), non_zero_elem[i].begin(), non_zero_elem[i].end());
    }
    convexHull(newHull, locations);
    for (size_t i = 0; i < locations.size(); i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t k = 0; k < non_zero_elem[j].size(); k++)
            {
                if (locations[i] == non_zero_elem[j][k])
                {
                    new_non_zero_elem[j].push_back(locations[i]);
                }
            }
        }
    }

    double pentagon_diag_norm = -1;
    Point2f down_left_edge_point, up_right_edge_point, up_left_edge_point;
    for (size_t i = 0; i < new_non_zero_elem[1].size(); i++)
    {
        for (size_t j = 0; j < new_non_zero_elem[2].size(); j++)
        {
            double temp_norm = norm(new_non_zero_elem[1][i] - new_non_zero_elem[2][j]);
            if (temp_norm > pentagon_diag_norm)
            {
                down_left_edge_point = new_non_zero_elem[1][i];
                up_right_edge_point  = new_non_zero_elem[2][j];
                pentagon_diag_norm = temp_norm;
            }
        }
    }

    if (down_left_edge_point == Point2f(0, 0) ||
        up_right_edge_point  == Point2f(0, 0) ||
        new_non_zero_elem[0].size() == 0) { return false; }

    double max_area = -1;
    up_left_edge_point = new_non_zero_elem[0][0];

    for (size_t i = 0; i < new_non_zero_elem[0].size(); i++)
    {
        vector<Point2f> list_edge_points;
        list_edge_points.push_back(new_non_zero_elem[0][i]);
        list_edge_points.push_back(down_left_edge_point);
        list_edge_points.push_back(up_right_edge_point);

        double temp_area = fabs(contourArea(list_edge_points));
        if (max_area < temp_area)
        {
            up_left_edge_point = new_non_zero_elem[0][i];
            max_area = temp_area;
        }
    }

    Point2f down_max_delta_point, up_max_delta_point;
    double norm_down_max_delta = -1, norm_up_max_delta = -1;
    for (size_t i = 0; i < new_non_zero_elem[1].size(); i++)
    {
        double temp_norm_delta = norm(up_left_edge_point - new_non_zero_elem[1][i])
                               + norm(down_left_edge_point - new_non_zero_elem[1][i]);
        if (norm_down_max_delta < temp_norm_delta)
        {
            down_max_delta_point = new_non_zero_elem[1][i];
            norm_down_max_delta = temp_norm_delta;
        }
    }


    for (size_t i = 0; i < new_non_zero_elem[2].size(); i++)
    {
        double temp_norm_delta = norm(up_left_edge_point - new_non_zero_elem[2][i])
                               + norm(up_right_edge_point - new_non_zero_elem[2][i]);
        if (norm_up_max_delta < temp_norm_delta)
        {
            up_max_delta_point = new_non_zero_elem[2][i];
            norm_up_max_delta = temp_norm_delta;
        }
    }

    transformation_points.push_back(down_left_edge_point);
    transformation_points.push_back(up_left_edge_point);
    transformation_points.push_back(up_right_edge_point);
    transformation_points.push_back(
        intersectionLines(down_left_edge_point, down_max_delta_point,
                          up_right_edge_point, up_max_delta_point));

    vector<Point2f> quadrilateral = getQuadrilateral(transformation_points);
    transformation_points = quadrilateral;

    int width = bin_barcode.size().width;
    int height = bin_barcode.size().height;
    for (size_t i = 0; i < transformation_points.size(); i++)
    {
        if ((cvRound(transformation_points[i].x) > width) ||
            (cvRound(transformation_points[i].y) > height)) { return false; }
    }
    return true;
}

Point2f QRDetect::intersectionLines(Point2f a1, Point2f a2, Point2f b1, Point2f b2)
{
    Point2f result_square_angle(
                              ((a1.x * a2.y  -  a1.y * a2.x) * (b1.x - b2.x) -
                               (b1.x * b2.y  -  b1.y * b2.x) * (a1.x - a2.x)) /
                              ((a1.x - a2.x) * (b1.y - b2.y) -
                               (a1.y - a2.y) * (b1.x - b2.x)),
                              ((a1.x * a2.y  -  a1.y * a2.x) * (b1.y - b2.y) -
                               (b1.x * b2.y  -  b1.y * b2.x) * (a1.y - a2.y)) /
                              ((a1.x - a2.x) * (b1.y - b2.y) -
                               (a1.y - a2.y) * (b1.x - b2.x))
                              );
    return result_square_angle;
}

// test function (if true then ------> else <------ )
bool QRDetect::testBypassRoute(vector<Point2f> hull, int start, int finish)
{
    CV_TRACE_FUNCTION();
    int index_hull = start, next_index_hull, hull_size = (int)hull.size();
    double test_length[2] = { 0.0, 0.0 };
    do
    {
        next_index_hull = index_hull + 1;
        if (next_index_hull == hull_size) { next_index_hull = 0; }
        test_length[0] += norm(hull[index_hull] - hull[next_index_hull]);
        index_hull = next_index_hull;
    }
    while(index_hull != finish);

    index_hull = start;
    do
    {
        next_index_hull = index_hull - 1;
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }
        test_length[1] += norm(hull[index_hull] - hull[next_index_hull]);
        index_hull = next_index_hull;
    }
    while(index_hull != finish);

    if (test_length[0] < test_length[1]) { return true; } else { return false; }
}

vector<Point2f> QRDetect::getQuadrilateral(vector<Point2f> angle_list)
{
    CV_TRACE_FUNCTION();
    size_t angle_size = angle_list.size();
    uint8_t value, mask_value;
    Mat mask = Mat::zeros(bin_barcode.rows + 2, bin_barcode.cols + 2, CV_8UC1);
    Mat fill_bin_barcode = bin_barcode.clone();
    for (size_t i = 0; i < angle_size; i++)
    {
        LineIterator line_iter(bin_barcode, angle_list[ i      % angle_size],
                                            angle_list[(i + 1) % angle_size]);
        for(int j = 0; j < line_iter.count; j++, ++line_iter)
        {
            value = bin_barcode.at<uint8_t>(line_iter.pos());
            mask_value = mask.at<uint8_t>(line_iter.pos() + Point(1, 1));
            if (value == 0 && mask_value == 0)
            {
                floodFill(fill_bin_barcode, mask, line_iter.pos(), 255,
                          0, Scalar(), Scalar(), FLOODFILL_MASK_ONLY);
            }
        }
    }
    vector<Point> locations;
    Mat mask_roi = mask(Range(1, bin_barcode.rows - 1), Range(1, bin_barcode.cols - 1));

    findNonZero(mask_roi, locations);

    for (size_t i = 0; i < angle_list.size(); i++)
    {
        int x = cvRound(angle_list[i].x);
        int y = cvRound(angle_list[i].y);
        locations.push_back(Point(x, y));
    }

    vector<Point> integer_hull;
    convexHull(locations, integer_hull);
    int hull_size = (int)integer_hull.size();
    vector<Point2f> hull(hull_size);
    for (int i = 0; i < hull_size; i++)
    {
        float x = saturate_cast<float>(integer_hull[i].x);
        float y = saturate_cast<float>(integer_hull[i].y);
        hull[i] = Point2f(x, y);
    }

    const double experimental_area = fabs(contourArea(hull));

    vector<Point2f> result_hull_point(angle_size);
    double min_norm;
    for (size_t i = 0; i < angle_size; i++)
    {
        min_norm = std::numeric_limits<double>::max();
        Point closest_pnt;
        for (int j = 0; j < hull_size; j++)
        {
            double temp_norm = norm(hull[j] - angle_list[i]);
            if (min_norm > temp_norm)
            {
                min_norm = temp_norm;
                closest_pnt = hull[j];
            }
        }
        result_hull_point[i] = closest_pnt;
    }

    int start_line[2] = { 0, 0 }, finish_line[2] = { 0, 0 }, unstable_pnt = 0;
    for (int i = 0; i < hull_size; i++)
    {
        if (result_hull_point[2] == hull[i]) { start_line[0] = i; }
        if (result_hull_point[1] == hull[i]) { finish_line[0] = start_line[1] = i; }
        if (result_hull_point[0] == hull[i]) { finish_line[1] = i; }
        if (result_hull_point[3] == hull[i]) { unstable_pnt = i; }
    }

    int index_hull, extra_index_hull, next_index_hull, extra_next_index_hull;
    Point result_side_begin[4], result_side_end[4];

    bool bypass_orientation = testBypassRoute(hull, start_line[0], finish_line[0]);

    min_norm = std::numeric_limits<double>::max();
    index_hull = start_line[0];
    do
    {
        if (bypass_orientation) { next_index_hull = index_hull + 1; }
        else { next_index_hull = index_hull - 1; }

        if (next_index_hull == hull_size) { next_index_hull = 0; }
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }

        Point angle_closest_pnt =  norm(hull[index_hull] - angle_list[1]) >
        norm(hull[index_hull] - angle_list[2]) ? angle_list[2] : angle_list[1];

        Point intrsc_line_hull =
        intersectionLines(hull[index_hull], hull[next_index_hull],
                          angle_list[1], angle_list[2]);
        double temp_norm = getCosVectors(hull[index_hull], intrsc_line_hull, angle_closest_pnt);
        if (min_norm > temp_norm &&
            norm(hull[index_hull] - hull[next_index_hull]) >
            norm(angle_list[1] - angle_list[2]) * 0.1)
        {
            min_norm = temp_norm;
            result_side_begin[0] = hull[index_hull];
            result_side_end[0]   = hull[next_index_hull];
        }


        index_hull = next_index_hull;
    }
    while(index_hull != finish_line[0]);

    if (min_norm == std::numeric_limits<double>::max())
    {
        result_side_begin[0] = angle_list[1];
        result_side_end[0]   = angle_list[2];
    }

    min_norm = std::numeric_limits<double>::max();
    index_hull = start_line[1];
    bypass_orientation = testBypassRoute(hull, start_line[1], finish_line[1]);
    do
    {
        if (bypass_orientation) { next_index_hull = index_hull + 1; }
        else { next_index_hull = index_hull - 1; }

        if (next_index_hull == hull_size) { next_index_hull = 0; }
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }

        Point angle_closest_pnt =  norm(hull[index_hull] - angle_list[0]) >
        norm(hull[index_hull] - angle_list[1]) ? angle_list[1] : angle_list[0];

        Point intrsc_line_hull =
        intersectionLines(hull[index_hull], hull[next_index_hull],
                          angle_list[0], angle_list[1]);
        double temp_norm = getCosVectors(hull[index_hull], intrsc_line_hull, angle_closest_pnt);
        if (min_norm > temp_norm &&
            norm(hull[index_hull] - hull[next_index_hull]) >
            norm(angle_list[0] - angle_list[1]) * 0.05)
        {
            min_norm = temp_norm;
            result_side_begin[1] = hull[index_hull];
            result_side_end[1]   = hull[next_index_hull];
        }

        index_hull = next_index_hull;
    }
    while(index_hull != finish_line[1]);

    if (min_norm == std::numeric_limits<double>::max())
    {
        result_side_begin[1] = angle_list[0];
        result_side_end[1]   = angle_list[1];
    }

    bypass_orientation = testBypassRoute(hull, start_line[0], unstable_pnt);
    const bool extra_bypass_orientation = testBypassRoute(hull, finish_line[1], unstable_pnt);

    vector<Point2f> result_angle_list(4), test_result_angle_list(4);
    double min_diff_area = std::numeric_limits<double>::max();
    index_hull = start_line[0];
    const double standart_norm = std::max(
        norm(result_side_begin[0] - result_side_end[0]),
        norm(result_side_begin[1] - result_side_end[1]));
    do
    {
        if (bypass_orientation) { next_index_hull = index_hull + 1; }
        else { next_index_hull = index_hull - 1; }

        if (next_index_hull == hull_size) { next_index_hull = 0; }
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }

        if (norm(hull[index_hull] - hull[next_index_hull]) < standart_norm * 0.1)
        { index_hull = next_index_hull; continue; }

        extra_index_hull = finish_line[1];
        do
        {
            if (extra_bypass_orientation) { extra_next_index_hull = extra_index_hull + 1; }
            else { extra_next_index_hull = extra_index_hull - 1; }

            if (extra_next_index_hull == hull_size) { extra_next_index_hull = 0; }
            if (extra_next_index_hull == -1) { extra_next_index_hull = hull_size - 1; }

            if (norm(hull[extra_index_hull] - hull[extra_next_index_hull]) < standart_norm * 0.1)
            { extra_index_hull = extra_next_index_hull; continue; }

            test_result_angle_list[0]
            = intersectionLines(result_side_begin[0], result_side_end[0],
                                result_side_begin[1], result_side_end[1]);
            test_result_angle_list[1]
            = intersectionLines(result_side_begin[1], result_side_end[1],
                                hull[extra_index_hull], hull[extra_next_index_hull]);
            test_result_angle_list[2]
            = intersectionLines(hull[extra_index_hull], hull[extra_next_index_hull],
                                hull[index_hull], hull[next_index_hull]);
            test_result_angle_list[3]
            = intersectionLines(hull[index_hull], hull[next_index_hull],
                                result_side_begin[0], result_side_end[0]);

            const double test_diff_area
                = fabs(fabs(contourArea(test_result_angle_list)) - experimental_area);
            if (min_diff_area > test_diff_area)
            {
                min_diff_area = test_diff_area;
                for (size_t i = 0; i < test_result_angle_list.size(); i++)
                {
                    result_angle_list[i] = test_result_angle_list[i];
                }
            }

            extra_index_hull = extra_next_index_hull;
        }
        while(extra_index_hull != unstable_pnt);

        index_hull = next_index_hull;
    }
    while(index_hull != unstable_pnt);

    // check label points
    if (norm(result_angle_list[0] - angle_list[1]) > 2) { result_angle_list[0] = angle_list[1]; }
    if (norm(result_angle_list[1] - angle_list[0]) > 2) { result_angle_list[1] = angle_list[0]; }
    if (norm(result_angle_list[3] - angle_list[2]) > 2) { result_angle_list[3] = angle_list[2]; }

    // check calculation point
    if (norm(result_angle_list[2] - angle_list[3]) >
       (norm(result_angle_list[0] - result_angle_list[1]) +
        norm(result_angle_list[0] - result_angle_list[3])) * 0.5 )
    { result_angle_list[2] = angle_list[3]; }

    return result_angle_list;
}

//      / | b
//     /  |
//    /   |
//  a/    | c

inline double QRDetect::getCosVectors(Point2f a, Point2f b, Point2f c)
{
    return ((a - b).x * (c - b).x + (a - b).y * (c - b).y) / (norm(a - b) * norm(c - b));
}

struct QRCodeDetector::Impl
{
public:
    Impl() { epsX = 0.2; epsY = 0.1; }
    ~Impl() {}

    double epsX, epsY;
};

QRCodeDetector::QRCodeDetector() : p(new Impl) {}
QRCodeDetector::~QRCodeDetector() {}

void QRCodeDetector::setEpsX(double epsX) { p->epsX = epsX; }
void QRCodeDetector::setEpsY(double epsY) { p->epsY = epsY; }

bool QRCodeDetector::detect(InputArray in, OutputArray points) const
{
    Mat inarr = in.getMat();
    CV_Assert(!inarr.empty());
    CV_Assert(inarr.depth() == CV_8U);
    if (inarr.cols <= 20 || inarr.rows <= 20)
        return false;  // image data is not enough for providing reliable results

    int incn = inarr.channels();
    if( incn == 3 || incn == 4 )
    {
        Mat gray;
        cvtColor(inarr, gray, COLOR_BGR2GRAY);
        inarr = gray;
    }

    QRDetect qrdet;
    qrdet.init(inarr, p->epsX, p->epsY);
    if (!qrdet.localization()) { return false; }
    if (!qrdet.computeTransformationPoints()) { return false; }
    vector<Point2f> pnts2f = qrdet.getTransformationPoints();
    Mat(pnts2f).convertTo(points, points.fixedType() ? points.type() : CV_32FC2);
    return true;
}

bool detectQRCode(InputArray in, vector<Point> &points, double eps_x, double eps_y)
{
    QRCodeDetector qrdetector;
    qrdetector.setEpsX(eps_x);
    qrdetector.setEpsY(eps_y);

    return qrdetector.detect(in, points);
}

class QRDecode
{
public:
    void init(const Mat &src, const vector<Point2f> &points);
    Mat getIntermediateBarcode() { return intermediate; }
    Mat getStraightBarcode() { return straight; }
    size_t getVersion() { return version; }
    std::string getDecodeInformation() { return result_info; }
    bool fullDecodingProcess();
protected:
    bool updatePerspective();
    bool versionDefinition();
    bool samplingForVersion();
    bool decodingProcess();
    Mat original, no_border_intermediate, intermediate, straight;
    vector<Point2f> original_points;
    std::string result_info;
    uint8_t version, version_size;
    float test_perspective_size;
};

void QRDecode::init(const Mat &src, const vector<Point2f> &points)
{
    CV_TRACE_FUNCTION();
    vector<Point2f> bbox = points;
    original = src.clone();
    intermediate = Mat::zeros(original.size(), CV_8UC1);
    original_points = bbox;
    version = 0;
    version_size = 0;
    test_perspective_size = 251;
    result_info = "";
}

bool QRDecode::updatePerspective()
{
    CV_TRACE_FUNCTION();
    const Point2f centerPt = QRDetect::intersectionLines(original_points[0], original_points[2],
                                                         original_points[1], original_points[3]);
    if (cvIsNaN(centerPt.x) || cvIsNaN(centerPt.y))
        return false;

    const Size temporary_size(cvRound(test_perspective_size), cvRound(test_perspective_size));

    vector<Point2f> perspective_points;
    perspective_points.push_back(Point2f(0.f, 0.f));
    perspective_points.push_back(Point2f(test_perspective_size, 0.f));

    perspective_points.push_back(Point2f(test_perspective_size, test_perspective_size));
    perspective_points.push_back(Point2f(0.f, test_perspective_size));

    perspective_points.push_back(Point2f(test_perspective_size * 0.5f, test_perspective_size * 0.5f));

    vector<Point2f> pts = original_points;
    pts.push_back(centerPt);

    Mat H = findHomography(pts, perspective_points);
    Mat bin_original;
    adaptiveThreshold(original, bin_original, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 83, 2);
    Mat temp_intermediate;
    warpPerspective(bin_original, temp_intermediate, H, temporary_size, INTER_NEAREST);
    no_border_intermediate = temp_intermediate(Range(1, temp_intermediate.rows), Range(1, temp_intermediate.cols));

    const int border = cvRound(0.1 * test_perspective_size);
    const int borderType = BORDER_CONSTANT;
    copyMakeBorder(no_border_intermediate, intermediate, border, border, border, border, borderType, Scalar(255));
    return true;
}

inline Point computeOffset(const vector<Point>& v)
{
    // compute the width/height of convex hull
    Rect areaBox = boundingRect(v);

    // compute the good offset
    // the box is consisted by 7 steps
    // to pick the middle of the stripe, it needs to be 1/14 of the size
    const int cStep = 7 * 2;
    Point offset = Point(areaBox.width, areaBox.height);
    offset /= cStep;
    return offset;
}

bool QRDecode::versionDefinition()
{
    CV_TRACE_FUNCTION();
    LineIterator line_iter(intermediate, Point2f(0, 0), Point2f(test_perspective_size, test_perspective_size));
    Point black_point = Point(0, 0);
    for(int j = 0; j < line_iter.count; j++, ++line_iter)
    {
        const uint8_t value = intermediate.at<uint8_t>(line_iter.pos());
        if (value == 0) { black_point = line_iter.pos(); break; }
    }

    Mat mask = Mat::zeros(intermediate.rows + 2, intermediate.cols + 2, CV_8UC1);
    floodFill(intermediate, mask, black_point, 255, 0, Scalar(), Scalar(), FLOODFILL_MASK_ONLY);

    vector<Point> locations, non_zero_elem;
    Mat mask_roi = mask(Range(1, intermediate.rows - 1), Range(1, intermediate.cols - 1));
    findNonZero(mask_roi, non_zero_elem);
    convexHull(non_zero_elem, locations);
    Point offset = computeOffset(locations);

    Point temp_remote = locations[0], remote_point;
    const Point delta_diff = offset;
    for (size_t i = 0; i < locations.size(); i++)
    {
        if (norm(black_point - temp_remote) <= norm(black_point - locations[i]))
        {
            const uint8_t value = intermediate.at<uint8_t>(temp_remote - delta_diff);
            temp_remote = locations[i];
            if (value == 0) { remote_point = temp_remote - delta_diff; }
            else { remote_point = temp_remote - (delta_diff / 2); }
        }
    }

    size_t transition_x = 0 , transition_y = 0;

    uint8_t future_pixel = 255;
    const uint8_t *intermediate_row = intermediate.ptr<uint8_t>(remote_point.y);
    for(int i = remote_point.x; i < intermediate.cols; i++)
    {
        if (intermediate_row[i] == future_pixel)
        {
            future_pixel = static_cast<uint8_t>(~future_pixel);
            transition_x++;
        }
    }

    future_pixel = 255;
    for(int j = remote_point.y; j < intermediate.rows; j++)
    {
        const uint8_t value = intermediate.at<uint8_t>(Point(j, remote_point.x));
        if (value == future_pixel)
        {
            future_pixel = static_cast<uint8_t>(~future_pixel);
            transition_y++;
        }
    }

    version = saturate_cast<uint8_t>((std::min(transition_x, transition_y) - 1) * 0.25 - 1);
    if ( !(  0 < version && version <= 40 ) ) { return false; }
    version_size = 21 + (version - 1) * 4;
    return true;
}

bool QRDecode::samplingForVersion()
{
    CV_TRACE_FUNCTION();
    const double multiplyingFactor = (version < 3)  ? 1 :
                                     (version == 3) ? 1.5 :
                                     version * (5 + version - 4);
    const Size newFactorSize(
                  cvRound(no_border_intermediate.size().width  * multiplyingFactor),
                  cvRound(no_border_intermediate.size().height * multiplyingFactor));
    Mat postIntermediate(newFactorSize, CV_8UC1);
    resize(no_border_intermediate, postIntermediate, newFactorSize, 0, 0, INTER_AREA);

    const int delta_rows = cvRound((postIntermediate.rows * 1.0) / version_size);
    const int delta_cols = cvRound((postIntermediate.cols * 1.0) / version_size);

    vector<double> listFrequencyElem;
    for (int r = 0; r < postIntermediate.rows; r += delta_rows)
    {
        for (int c = 0; c < postIntermediate.cols; c += delta_cols)
        {
            Mat tile = postIntermediate(
                           Range(r, min(r + delta_rows, postIntermediate.rows)),
                           Range(c, min(c + delta_cols, postIntermediate.cols)));
            const double frequencyElem = (countNonZero(tile) * 1.0) / tile.total();
            listFrequencyElem.push_back(frequencyElem);
        }
    }

    double dispersionEFE = std::numeric_limits<double>::max();
    double experimentalFrequencyElem = 0;
    for (double expVal = 0; expVal < 1; expVal+=0.001)
    {
        double testDispersionEFE = 0.0;
        for (size_t i = 0; i < listFrequencyElem.size(); i++)
        {
            testDispersionEFE += (listFrequencyElem[i] - expVal) *
                                 (listFrequencyElem[i] - expVal);
        }
        testDispersionEFE /= (listFrequencyElem.size() - 1);
        if (dispersionEFE > testDispersionEFE)
        {
            dispersionEFE = testDispersionEFE;
            experimentalFrequencyElem = expVal;
        }
    }

    straight = Mat(Size(version_size, version_size), CV_8UC1, Scalar(0));
    for (int r = 0; r < version_size * version_size; r++)
    {
        int i   = r / straight.cols;
        int j   = r % straight.cols;
        straight.ptr<uint8_t>(i)[j] = (listFrequencyElem[r] < experimentalFrequencyElem) ? 0 : 255;
    }
    return true;
}

bool QRDecode::decodingProcess()
{
#ifdef HAVE_QUIRC
    if (straight.empty()) { return false; }

    quirc_code qr_code;
    memset(&qr_code, 0, sizeof(qr_code));

    qr_code.size = straight.size().width;
    for (int x = 0; x < qr_code.size; x++)
    {
        for (int y = 0; y < qr_code.size; y++)
        {
            int position = y * qr_code.size + x;
            qr_code.cell_bitmap[position >> 3]
                |= straight.ptr<uint8_t>(y)[x] ? 0 : (1 << (position & 7));
        }
    }

    quirc_data qr_code_data;
    quirc_decode_error_t errorCode = quirc_decode(&qr_code, &qr_code_data);
    if (errorCode != 0) { return false; }

    for (int i = 0; i < qr_code_data.payload_len; i++)
    {
        result_info += qr_code_data.payload[i];
    }
    return true;
#else
    return false;
#endif

}

bool QRDecode::fullDecodingProcess()
{
#ifdef HAVE_QUIRC
    if (!updatePerspective())  { return false; }
    if (!versionDefinition())  { return false; }
    if (!samplingForVersion()) { return false; }
    if (!decodingProcess())    { return false; }
    return true;
#else
    std::cout << "Library QUIRC is not linked. No decoding is performed. Take it to the OpenCV repository." << std::endl;
    return false;
#endif
}

bool decodeQRCode(InputArray in, InputArray points, std::string &decoded_info, OutputArray straight_qrcode)
{
    QRCodeDetector qrcode;
    decoded_info = qrcode.decode(in, points, straight_qrcode);
    return !decoded_info.empty();
}

cv::String QRCodeDetector::decode(InputArray in, InputArray points,
                                  OutputArray straight_qrcode)
{
    Mat inarr = in.getMat();
    CV_Assert(!inarr.empty());
    CV_Assert(inarr.depth() == CV_8U);
    if (inarr.cols <= 20 || inarr.rows <= 20)
        return cv::String();  // image data is not enough for providing reliable results

    int incn = inarr.channels();
    if( incn == 3 || incn == 4 )
    {
        Mat gray;
        cvtColor(inarr, gray, COLOR_BGR2GRAY);
        inarr = gray;
    }

    vector<Point2f> src_points;
    points.copyTo(src_points);
    CV_Assert(src_points.size() == 4);
    CV_CheckGT(contourArea(src_points), 0.0, "Invalid QR code source points");

    QRDecode qrdec;
    qrdec.init(inarr, src_points);
    bool ok = qrdec.fullDecodingProcess();

    std::string decoded_info = qrdec.getDecodeInformation();

    if (ok && straight_qrcode.needed())
    {
        qrdec.getStraightBarcode().convertTo(straight_qrcode,
                                             straight_qrcode.fixedType() ?
                                             straight_qrcode.type() : CV_32FC2);
    }

    return ok ? decoded_info : std::string();
}

cv::String QRCodeDetector::detectAndDecode(InputArray in,
                                           OutputArray points_,
                                           OutputArray straight_qrcode)
{
    Mat inarr = in.getMat();
    CV_Assert(!inarr.empty());
    CV_Assert(inarr.depth() == CV_8U);
    if (inarr.cols <= 20 || inarr.rows <= 20)
        return cv::String();  // image data is not enough for providing reliable results

    int incn = inarr.channels();
    if( incn == 3 || incn == 4 )
    {
        Mat gray;
        cvtColor(inarr, gray, COLOR_BGR2GRAY);
        inarr = gray;
    }

    vector<Point2f> points;
    bool ok = detect(inarr, points);
    if( points_.needed() )
    {
        if( ok )
            Mat(points).copyTo(points_);
        else
            points_.release();
    }
    std::string decoded_info;
    if( ok )
        decoded_info = decode(inarr, points, straight_qrcode);
    return decoded_info;
}

class QRDetectMulti : public QRDetect
{
public:
    void init(const Mat& src, double eps_vertical_ = 0.2, double eps_horizontal_ = 0.1);
    bool localization();
    bool computeTransformationPoints(size_t cur_ind);
    vector< vector< Point2f > > getTransformationPoints() { return transformation_points; }

protected:
    vector<Point2f> separateVerticalLines(const vector<Vec3d> &list_lines);
    int findNumberLocalizationPoints(vector<Point2f>& tmp_localization_points);
    void findQRCodeContours(vector<Point2f>& tmp_localization_points, vector< vector< Point2f > >& true_points_group, const int& num_qrcodes);
    bool checkSets(vector<vector<Point2f> >& true_points_group, vector<vector<Point2f> >& true_points_group_copy,
                   vector<Point2f>& tmp_localization_points);
    void deleteUsedPoints(vector<vector<Point2f> >& true_points_group, vector<vector<Point2f> >& loc,
                          vector<Point2f>& tmp_localization_points);
    void fixationPoints(vector<Point2f> &local_point);
    bool checkPoints(const vector<Point2f>& quadrangle_points);
    bool checkPointsInsideQuadrangle(const vector<Point2f>& quadrangle_points);
    bool checkPointsInsideTriangle(const vector<Point2f>& triangle_points);

    Mat bin_barcode_fullsize, bin_barcode_temp;
    vector<Point2f> not_resized_loc_points;
    vector<Point2f> resized_loc_points;
    vector< vector< Point2f > > localization_points, transformation_points;
    struct compareSquare
    {
        vector<Point2f> points;
        bool operator()(const Vec3i& a, const Vec3i& b) const;
    } comparator;
    Mat original;
    class ParallelSearch : public ParallelLoopBody
    {
    public:
        ParallelSearch(vector< vector< Point2f > >& true_points_group_,
                        vector< vector< Point2f > >& loc_, int iter_, int* end_,
                        vector< vector< Vec3i > >& all_points_,
                        QRDetectMulti& cl_):
                        true_points_group(true_points_group_),
                        loc(loc_),
                        iter(iter_),
                        end(end_),
                        all_points(all_points_),
                        cl(cl_)
        {
        }
        void operator()(const Range& range) const CV_OVERRIDE;
        vector< vector< Point2f > >& true_points_group;
        vector< vector< Point2f > >& loc;
        int iter;
        int* end;
        vector< vector< Vec3i > >& all_points;
        QRDetectMulti& cl;
    };
};

void QRDetectMulti::ParallelSearch::operator()(const Range& range) const
{
    for(int s = range.start; s < range.end; s++)
    {
        bool flag = false;
        for (int r = iter; r < end[s]; r++)
        {
             if(flag) break;

             size_t x = iter + s;
             size_t k = r - iter;
             vector<Point2f> triangle;

             for(int l = 0; l < 3; l++)
             {
                 triangle.push_back(true_points_group[s][all_points[s][k][l]]);
             }

             if(cl.checkPointsInsideTriangle(triangle))
             {
                 bool flag_for_break = false;
                 cl.fixationPoints(triangle);
                 if(triangle.size() == 3)
                 {
                     cl.localization_points[x] = triangle;
                     if (cl.purpose == cl.SHRINKING)
                     {

                          for (size_t j = 0; j < 3; j++)
                          {
                              cl.localization_points[x][j] *= cl.coeff_expansion;
                          }
                     }
                     else if (cl.purpose == cl.ZOOMING)
                     {
                          for (size_t j = 0; j < 3; j++)
                          {
                               cl.localization_points[x][j] /= cl.coeff_expansion;
                          }
                     }
                     for (size_t i = 0; i < 3; i++)
                     {
                          for (size_t j = i + 1; j < 3; j++)
                          {
                               if (norm(cl.localization_points[x][i] - cl.localization_points[x][j]) < 10)
                               {
                                   cl.localization_points[x].clear();
                                   flag_for_break = true;
                                   break;
                               }
                          } if(flag_for_break)
                                break;
                     }
                     if((!flag_for_break)
                         && (cl.localization_points[x].size() == 3)
                         && (cl.computeTransformationPoints(x))
                         && (cl.checkPointsInsideQuadrangle(cl.transformation_points[x]))
                         && (cl.checkPoints(cl.transformation_points[x])))
                     {
                         for(int l = 0; l < 3; l++)
                            loc[s][all_points[s][k][l]].x = -1;

                         flag = true;
                         break;
                     }
                }
                if(flag)
                    break;
                else
                {
                    cl.transformation_points[x].clear();
                    cl.localization_points[x].clear();
                }
            }
        }
    }
}

void QRDetectMulti::init(const Mat& src, double eps_vertical_, double eps_horizontal_)
{
    CV_TRACE_FUNCTION();
    CV_Assert(!src.empty());
    const double min_side = std::min(src.size().width, src.size().height);
    if (min_side < 512.0)
    {
        purpose = ZOOMING;
        coeff_expansion = 512.0 / min_side;
        const int width  = cvRound(src.size().width  * coeff_expansion);
        const int height = cvRound(src.size().height  * coeff_expansion);
        Size new_size(width, height);
        resize(src, barcode, new_size, 0, 0, INTER_LINEAR);
    }
    else if (min_side > 512.0)
    {
        purpose = SHRINKING;
        coeff_expansion = min_side / 512.0;
        const int width  = cvRound(src.size().width  / coeff_expansion);
        const int height = cvRound(src.size().height  / coeff_expansion);
        Size new_size(width, height);
        resize(src, barcode, new_size, 0, 0, INTER_AREA);
    }
    else
    {
        purpose = UNCHANGED;
        coeff_expansion = 1.0;
        barcode = src.clone();
    }

    eps_vertical   = eps_vertical_;
    eps_horizontal = eps_horizontal_;
    adaptiveThreshold(barcode, bin_barcode, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 83, 2);
    adaptiveThreshold(src, bin_barcode_fullsize, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 83, 2);
}

vector<Point2f> QRDetectMulti::separateVerticalLines(const vector<Vec3d> &list_lines)
{
    CV_TRACE_FUNCTION();
    vector<Vec3d> result;
    int temp_length;
    vector<Point2f> point2f_result;
    uint8_t next_pixel;
    vector<double> test_lines;
    temp_length = 0;

    for (size_t pnt = 0; pnt < list_lines.size(); pnt++)
    {
        const int x = cvRound(list_lines[pnt][0] + list_lines[pnt][2] * 0.5);
        const int y = cvRound(list_lines[pnt][1]);

        // --------------- Search vertical up-lines --------------- //

        test_lines.clear();
        uint8_t future_pixel_up = 255;

        for (int j = y; j < bin_barcode.rows - 1; j++)
        {
            next_pixel = bin_barcode.ptr<uint8_t>(j + 1)[x];
            temp_length++;
            if (next_pixel == future_pixel_up)
            {
                future_pixel_up = static_cast<uint8_t>(~future_pixel_up);
                test_lines.push_back(temp_length);
                temp_length = 0;
                if (test_lines.size() == 3) { break; }
            }
        }

          // --------------- Search vertical down-lines --------------- //

          uint8_t future_pixel_down = 255;
          for (int j = y; j >= 1; j--)
          {
              next_pixel = bin_barcode.ptr<uint8_t>(j - 1)[x];
              temp_length++;
              if (next_pixel == future_pixel_down)
              {
                  future_pixel_down = static_cast<uint8_t>(~future_pixel_down);
                  test_lines.push_back(temp_length);
                  temp_length = 0;
                  if (test_lines.size() == 6) { break; }
              }
          }

          // --------------- Compute vertical lines --------------- //

          if (test_lines.size() == 6)
          {
              double length = 0.0, weight = 0.0;

              for (size_t i = 0; i < test_lines.size(); i++) { length += test_lines[i]; }

              CV_Assert(length > 0);
              for (size_t i = 0; i < test_lines.size(); i++)
              {
                  if (i % 3 != 0) { weight += fabs((test_lines[i] / length) - 1.0/ 7.0); }
                  else            { weight += fabs((test_lines[i] / length) - 3.0/14.0); }
              }

              if(weight < eps_horizontal)
              {
                  result.push_back(list_lines[pnt]);
              }
          }
      }
      if (result.size() > 2)
      {
          for (size_t i = 0; i < result.size(); i++)
          {
              point2f_result.push_back(
                    Point2f(static_cast<float>(result[i][0] + result[i][2] * 0.5),
                            static_cast<float>(result[i][1])));
          }
      }
      return point2f_result;
}

void QRDetectMulti::fixationPoints(vector<Point2f> &local_point)
{
    CV_TRACE_FUNCTION();

    Point2f v0(local_point[1] - local_point[2]);
    Point2f v1(local_point[0] - local_point[2]);
    Point2f v2(local_point[1] - local_point[0]);

    double cos_angles[3], norm_triangl[3];
    norm_triangl[0] = norm(v0);
    norm_triangl[1] = norm(v1);
    norm_triangl[2] = norm(v2);

    cos_angles[0] = v2.dot(-v1) / (norm_triangl[1] * norm_triangl[2]);
    cos_angles[1] = v2.dot(v0) / (norm_triangl[0] * norm_triangl[2]);
    cos_angles[2] = v1.dot(v0) / (norm_triangl[0] * norm_triangl[1]);

    const double angle_barrier = 0.85;
    if (fabs(cos_angles[0]) > angle_barrier || fabs(cos_angles[1]) > angle_barrier || fabs(cos_angles[2]) > angle_barrier)
    {
      local_point.clear();
      return;
    }

    size_t i_min_cos =
       (cos_angles[0] < cos_angles[1] && cos_angles[0] < cos_angles[2]) ? 0 :
       (cos_angles[1] < cos_angles[0] && cos_angles[1] < cos_angles[2]) ? 1 : 2;

    size_t index_max = 0;
    double max_area = std::numeric_limits<double>::min();
    for (size_t i = 0; i < local_point.size(); i++)
    {
        const size_t current_index = i % 3;
        const size_t left_index  = (i + 1) % 3;
        const size_t right_index = (i + 2) % 3;

        const Point2f current_point(local_point[current_index]),
            left_point(local_point[left_index]), right_point(local_point[right_index]),
            central_point(intersectionLines(current_point,
                              Point2f(static_cast<float>((local_point[left_index].x + local_point[right_index].x) * 0.5),
                                      static_cast<float>((local_point[left_index].y + local_point[right_index].y) * 0.5)),
                              Point2f(0, static_cast<float>(bin_barcode_temp.rows - 1)),
                              Point2f(static_cast<float>(bin_barcode_temp.cols - 1),
                                      static_cast<float>(bin_barcode_temp.rows - 1))));


        vector<Point2f> list_area_pnt;
        list_area_pnt.push_back(current_point);

        vector<LineIterator> list_line_iter;
        list_line_iter.push_back(LineIterator(bin_barcode_temp, current_point, left_point));
        list_line_iter.push_back(LineIterator(bin_barcode_temp, current_point, central_point));
        list_line_iter.push_back(LineIterator(bin_barcode_temp, current_point, right_point));

        for (size_t k = 0; k < list_line_iter.size(); k++)
        {
            uint8_t future_pixel = 255, count_index = 0;
            for(int j = 0; j < list_line_iter[k].count; j++, ++list_line_iter[k])
            {
                if (list_line_iter[k].pos().x >= bin_barcode_temp.cols ||
                    list_line_iter[k].pos().y >= bin_barcode_temp.rows) { break; }
                const uint8_t value = bin_barcode_temp.at<uint8_t>(list_line_iter[k].pos());
                if (value == future_pixel)
                {
                    future_pixel = static_cast<uint8_t>(~future_pixel);
                    count_index++;
                    if (count_index == 3)
                    {
                        list_area_pnt.push_back(list_line_iter[k].pos());
                        break;
                    }
                }
            }
        }

        const double temp_check_area = contourArea(list_area_pnt);
        if (temp_check_area > max_area)
        {
            index_max = current_index;
            max_area = temp_check_area;
        }

    }
    if (index_max == i_min_cos) { std::swap(local_point[0], local_point[index_max]); }
    else { local_point.clear(); return; }

    const Point2f rpt = local_point[0], bpt = local_point[1], gpt = local_point[2];
    Matx22f m(rpt.x - bpt.x, rpt.y - bpt.y, gpt.x - rpt.x, gpt.y - rpt.y);
    if( determinant(m) > 0 )
    {
        std::swap(local_point[1], local_point[2]);
    }
}

bool QRDetectMulti::checkPoints(const vector<Point2f>& quadrangle_points)
{
    if(quadrangle_points.size() != 4)
        return false;
    vector<Point> points;
    for(size_t i = 0; i < quadrangle_points.size(); i++)
        points.push_back(Point(quadrangle_points[i]));
    Mat lineMask = Mat::zeros(bin_barcode_fullsize.size(), bin_barcode_fullsize.type());
    fillConvexPoly(lineMask, points, int(points.size()), 255);
    vector<Point> indices;
    findNonZero(lineMask, indices);
    int count_w = 0;
    int count_b = 0;
    for (size_t r = 0; r < indices.size(); r++)
    {
        int pixel = bin_barcode.at<uchar>(indices[r].y , indices[r].x);
        if(pixel == 255)
        {
            count_w++;
        }
        if(pixel == 0)
        {
            count_b++;
        }
    }
    double frac = double(count_b) / double(count_w);
    double bottom_bound = 0.83;
    double upper_bound = 1.24;
    if ((frac <= bottom_bound) || (frac >= upper_bound))
        return false;
    return true;
}

bool QRDetectMulti::checkPointsInsideQuadrangle(const vector<Point2f>& quadrangle_points)
{
    if(quadrangle_points.size() != 4)
      return false;
    vector<Point> points;
    for(size_t i = 0; i < quadrangle_points.size(); i++)
      points.push_back(Point(quadrangle_points[i]));
    Mat lineMask = Mat::zeros(bin_barcode_fullsize.size(), bin_barcode_fullsize.type());
    fillConvexPoly(lineMask, points, int(points.size()), 255);
    int count = 0;
    for(size_t i = 0; i < not_resized_loc_points.size(); i++)
    {
        int pixel = lineMask.at<uchar>(not_resized_loc_points[i].y , not_resized_loc_points[i].x);
        if(pixel != 0)
        {
            count++;
        }
    }
    if (count == 3) return true;
    else return false;
}

bool QRDetectMulti::checkPointsInsideTriangle(const vector<Point2f>& triangle_points)
{
    if(triangle_points.size() != 3)
      return false;
    vector<Point> points;
    for(size_t i = 0; i < triangle_points.size(); i++)
      points.push_back(Point(triangle_points[i]));
    Mat lineMask = Mat::zeros(bin_barcode_temp.size(), bin_barcode_temp.type());
    fillConvexPoly(lineMask, points, int(points.size()), 255);
    double eps = 3;
    for(size_t i = 0; i < resized_loc_points.size(); i++)
    {
        int pixel = lineMask.at<uchar>(resized_loc_points[i].y , resized_loc_points[i].x);
        if(pixel != 0)
        {
            if((abs(resized_loc_points[i].x - points[0].x) > eps)
            && (abs(resized_loc_points[i].x - points[1].x) > eps)
            && (abs(resized_loc_points[i].x - points[2].x) > eps))
            {
                return false;
            }
        }
    }
    return true;
}

bool QRDetectMulti::compareSquare::operator()(const Vec3i& a, const Vec3i& b) const
{
    return abs((points[a[1]].x - points[a[0]].x) *
    (points[a[2]].y - points[a[0]].y) -
    (points[a[2]].x - points[a[0]].x) *
    (points[a[1]].y - points[a[0]].y)) <
    abs((points[b[1]].x - points[b[0]].x) *
    (points[b[2]].y - points[b[0]].y) -
    (points[b[2]].x - points[b[0]].x) *
    (points[b[1]].y - points[b[0]].y));
}

int QRDetectMulti::findNumberLocalizationPoints(vector<Point2f>& tmp_localization_points)
{
    size_t number_possible_purpose = 1;
    if(purpose == SHRINKING) number_possible_purpose = 2;
    Mat tmp_shrinking = bin_barcode;
    int tmp_num_points = 0;
    int num_points = -1;
    for(eps_horizontal = 0.1; eps_horizontal < 0.4; eps_horizontal += 0.1)
    {
        tmp_num_points = 0;
        num_points = -1;
        if(purpose == SHRINKING)
            number_possible_purpose = 2;
        else
            number_possible_purpose = 1;
        for(size_t k = 0; k < number_possible_purpose; k++)
        {
            if(k == 1)
                bin_barcode = bin_barcode_fullsize;
            vector<Vec3d> list_lines_x = searchHorizontalLines();
            if( list_lines_x.empty() )
            {
                if(k == 0)
                {
                    k = 1;
                    bin_barcode = bin_barcode_fullsize;
                    list_lines_x = searchHorizontalLines();
                    if (list_lines_x.empty())
                        break;
                }
                else
                    break;
            }
            vector<Point2f> list_lines_y = separateVerticalLines(list_lines_x);
            if ( list_lines_y.size() < 3 )
            {
                if(k == 0)
                {
                    k = 1;
                    bin_barcode = bin_barcode_fullsize;
                    list_lines_x = searchHorizontalLines();
                    if (list_lines_x.empty())
                        break;
                    list_lines_y = separateVerticalLines(list_lines_x);
                    if( list_lines_y.size() < 3 )
                        break;
                }
                else
                    break;
             }
             vector<int> index_list_lines_y;
             for(size_t i = 0; i < list_lines_y.size(); i++)
                 index_list_lines_y.push_back(-1);
             num_points = 0;
             for(size_t i = 0; i < list_lines_y.size() - 1; i++)
             {
                 for(size_t j = i; j < list_lines_y.size(); j++ )
                 {

                     double points_distance = norm(list_lines_y[i] - list_lines_y[j]);
                     if(points_distance <= 10)
                     {
                         if((index_list_lines_y[i] == -1) && (index_list_lines_y[j] == -1))
                         {
                             index_list_lines_y[i] = num_points;
                             index_list_lines_y[j] = num_points;
                             num_points++;
                         }
                         else if (index_list_lines_y[i] != -1)
                            index_list_lines_y[j] = index_list_lines_y[i];
                         else if (index_list_lines_y[j] != -1)
                            index_list_lines_y[i] = index_list_lines_y[j];
                     }
                  }
               }
               for(size_t i = 0; i < index_list_lines_y.size(); i++)
                   if(index_list_lines_y[i] == -1)
                   {
                       index_list_lines_y[i] = num_points;
                       num_points++;
                   }
                if ((tmp_num_points < num_points) && (k == 1))
                {
                   purpose = UNCHANGED;
                   tmp_num_points = num_points;
                   bin_barcode = bin_barcode_fullsize;
                   coeff_expansion = 1.0;
                }
                if ((tmp_num_points < num_points) && (k == 0))
                {
                    tmp_num_points = num_points;
                }
       }

       if ((tmp_num_points < 3) && (tmp_num_points >= 1))
       {
           const double min_side = std::min(bin_barcode_fullsize.size().width, bin_barcode_fullsize.size().height);
           if(min_side > 512)
           {
               bin_barcode = tmp_shrinking;
               purpose = SHRINKING;
               coeff_expansion = min_side / 512.0;
           }
           if(min_side < 512)
           {
               bin_barcode = tmp_shrinking;
               purpose = ZOOMING;
               coeff_expansion = 512 / min_side;
           }
       }
       else break;
   }
   if(purpose == SHRINKING)
        bin_barcode = tmp_shrinking;
   num_points = tmp_num_points;
   vector<Vec3d> list_lines_x = searchHorizontalLines();
   if( list_lines_x.empty() )
        { return num_points; }
   vector<Point2f> list_lines_y = separateVerticalLines(list_lines_x);
   if( list_lines_y.size() < 3 )
        { return num_points; }
   if(num_points < 3)
        return num_points;

   Mat labels;
   kmeans(list_lines_y, num_points, labels,
          TermCriteria( TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1),
          num_points, KMEANS_PP_CENTERS, tmp_localization_points);
   bin_barcode_temp = bin_barcode.clone();
   if (purpose == SHRINKING)
   {
        const int width  = cvRound(bin_barcode.size().width  * coeff_expansion);
        const int height = cvRound(bin_barcode.size().height * coeff_expansion);
        Size new_size(width, height);
        Mat intermediate;
        resize(bin_barcode, intermediate, new_size, 0, 0, INTER_LINEAR);
        bin_barcode = intermediate.clone();
   }
   else if (purpose == ZOOMING)
   {
       const int width  = cvRound(bin_barcode.size().width  / coeff_expansion);
       const int height = cvRound(bin_barcode.size().height / coeff_expansion);
       Size new_size(width, height);
       Mat intermediate;
       resize(bin_barcode, intermediate, new_size, 0, 0, INTER_LINEAR);
       bin_barcode = intermediate.clone();
   }
   else bin_barcode = bin_barcode_fullsize.clone();
   return num_points;
}

void QRDetectMulti::findQRCodeContours(vector<Point2f>& tmp_localization_points,
                                      vector< vector< Point2f > >& true_points_group, const int& num_qrcodes)
{
    Mat gray, blur_image, threshold_output;
    Mat bar = barcode;
    const int width  = cvRound(bin_barcode.size().width);
    const int height = cvRound(bin_barcode.size().height);
    Size new_size(width, height);
    resize(bar, bar, new_size, 0, 0, INTER_LINEAR);
    blur(bar, blur_image, Size(3, 3));
    threshold(blur_image, threshold_output, 50, 255, THRESH_BINARY);

    vector< vector< Point > > contours;
    vector<Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    vector<Point2f> all_contours_points;
    for(size_t i = 0; i < contours.size(); i ++)
        for(size_t j = 0; j < contours[i].size(); j++)
            all_contours_points.push_back(contours[i][j]);

    Mat qrcode_labels;
    vector<Point2f> clustered_localization_points;
    int count_contours = num_qrcodes;
    if(all_contours_points.size() < size_t(num_qrcodes))
        count_contours = int(all_contours_points.size());
    kmeans(all_contours_points, count_contours, qrcode_labels,
          TermCriteria( TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1),
          count_contours, KMEANS_PP_CENTERS, clustered_localization_points);

    vector< vector< Point2f > > qrcode_clusters(count_contours);
    for(int i = 0; i < count_contours; i++)
        for(int j = 0; j < int(all_contours_points.size()); j++)
        {
            if (qrcode_labels.at<int>(j, 0) == i)
            {
                qrcode_clusters[i].push_back(all_contours_points[j]);
            }
        }
    vector< vector< Point2f > > hull(count_contours);
    for(size_t i = 0; i < qrcode_clusters.size(); i++)
        convexHull(Mat(qrcode_clusters[i]), hull[i]);
    not_resized_loc_points = tmp_localization_points;
    resized_loc_points = tmp_localization_points;
    if (purpose == SHRINKING)
    {
        for (size_t j = 0; j < not_resized_loc_points.size(); j++)
        {
            not_resized_loc_points[j] *= coeff_expansion;
        }
    }
    else if (purpose == ZOOMING)
    {
        for (size_t j = 0; j < not_resized_loc_points.size(); j++)
        {
            not_resized_loc_points[j] /= coeff_expansion;
        }
    }

    true_points_group.resize(hull.size());

    for(size_t j = 0; j < hull.size(); j++)
    {
        vector<Point> hull_int;
        for(size_t k = 0; k < hull[j].size(); k++)
            hull_int.push_back(Point(hull[j][k]));
        Mat lineMask = Mat::zeros(bar.size(), threshold_output.type());
        fillConvexPoly(lineMask, hull_int, int(hull_int.size()), 255);
        for(size_t i = 0; i < not_resized_loc_points.size(); i++)
        {
            int pixel = lineMask.at<uchar>(not_resized_loc_points[i].y , not_resized_loc_points[i].x);
            if(pixel != 0)
            {
                true_points_group[j].push_back(tmp_localization_points[i]);
                tmp_localization_points[i].x = -1;
            }

        }
    }
    vector<Point2f> copy;
    for(size_t j = 0; j < tmp_localization_points.size(); j++)
    {
       if(tmp_localization_points[j].x != -1)
            copy.push_back(tmp_localization_points[j]);
    }
    tmp_localization_points = copy;
}

bool QRDetectMulti::checkSets(vector<vector<Point2f> >& true_points_group, vector<vector<Point2f> >& true_points_group_copy,
                              vector<Point2f>& tmp_localization_points)
{
    for(size_t i = 0; i < true_points_group.size(); i++)
    {
         if (true_points_group[i].size() < 3)
         {
             for(size_t j = 0; j < true_points_group[i].size(); j++)
                 tmp_localization_points.push_back(true_points_group[i][j]);
             true_points_group[i].clear();
         }
    }
    vector< vector< Point2f > > temp_for_copy;
    for(size_t i = 0; i < true_points_group.size(); i++)
    {
         if(true_points_group[i].size() != 0)
             temp_for_copy.push_back(true_points_group[i]);
    }
    true_points_group = temp_for_copy;
    if(true_points_group.size() == 0)
    {
         true_points_group.push_back(tmp_localization_points);
         tmp_localization_points.clear();
    }
    if(true_points_group.size() == 0)
        return false;
    if (true_points_group[0].size() < 3)
        return false;


    int* set_size = new int[true_points_group.size()];
    for(size_t i = 0; i < true_points_group.size(); i++)
    {
        set_size[i] = int(0.5 * (true_points_group[i].size() - 2 ) * (true_points_group[i].size() - 1));
        //set_size[i] = int( (true_points_group[i].size() - 2 ) * (true_points_group[i].size() - 1) * true_points_group[i].size() / 6);
    }
    vector< vector< Vec3i > > all_points(true_points_group.size());
    for(size_t i = 0; i < true_points_group.size(); i++)
        all_points[i].resize(set_size[i]);
    int cur_cluster = 0;
    for(size_t i = 0; i < true_points_group.size(); i++)
    {
         cur_cluster = 0;
         for(size_t j = 1; j < true_points_group[i].size() - 1; j++)
             for(size_t k = j + 1; k < true_points_group[i].size(); k++)
             {
                 all_points[i][cur_cluster][0] = 0;
                 all_points[i][cur_cluster][1] = int(j);
                 all_points[i][cur_cluster][2] = int(k);
                 cur_cluster++;
             }
    }

    for(size_t i = 0; i < true_points_group.size(); i++)
    {
       comparator.points = true_points_group[i];
       std::sort(all_points[i].begin(), all_points[i].end(), comparator);
    }
    if(true_points_group.size() == 1)
    {
        int check_number = 35;
        if (set_size[0] > check_number)
           set_size[0] = check_number;
        all_points[0].resize(set_size[0]);
    }
    int iter = int(localization_points.size());
    localization_points.resize(iter + true_points_group.size());
    transformation_points.resize(iter + true_points_group.size());

    true_points_group_copy = true_points_group;
    int* end = new int[true_points_group.size()];
    for(size_t i = 0; i < true_points_group.size(); i++)
        end[i] = iter + set_size[i];
    ParallelSearch parallelSearch(true_points_group,
                     true_points_group_copy, iter, end, all_points, *this);
    parallel_for_(Range(0, int(true_points_group.size())), parallelSearch);

    return true;
}

void QRDetectMulti::deleteUsedPoints(vector<vector<Point2f> >& true_points_group, vector<vector<Point2f> >& loc,
                                     vector<Point2f>& tmp_localization_points)
{
    size_t iter = localization_points.size() - true_points_group.size() ;
    for(size_t s = 0; s < true_points_group.size(); s++)
    {
        if(localization_points[iter + s].empty())
              loc[s][0].x = -2;

        if(loc[s].size() == 3)
        {

            if((true_points_group.size() > 1) || ((true_points_group.size() == 1) && (tmp_localization_points.size() != 0)) )
            {
               for(size_t j = 0; j < true_points_group[s].size(); j++)
               {
                   if(loc[s][j].x != -1)
                   {
                       loc[s][j].x = -1;
                       tmp_localization_points.push_back(true_points_group[s][j]);
                   }
               }
            }
        }
        vector<Point2f> for_copy;
        for(size_t j = 0; j < loc[s].size(); j++)
        {
             if((loc[s][j].x != -1) && (loc[s][j].x != -2) )
             {
                 for_copy.push_back(true_points_group[s][j]);
             }
             if ((loc[s][j].x == -2) && (true_points_group.size() > 1))
             {
                 tmp_localization_points.push_back(true_points_group[s][j]);
             }
         }
         true_points_group[s] = for_copy;
     }

     vector< vector< Point2f > > for_copy_loc;
     vector< vector< Point2f > > for_copy_trans;


     for(size_t i = 0; i < localization_points.size(); i++)
     {
         if ((localization_points[i].size() == 3) && (transformation_points[i].size() == 4))
         {
             for_copy_loc.push_back(localization_points[i]);
             for_copy_trans.push_back(transformation_points[i]);
         }
     }
     localization_points = for_copy_loc;
     transformation_points = for_copy_trans;
}

bool QRDetectMulti::localization()
{
    CV_TRACE_FUNCTION();
    vector<Point2f> tmp_localization_points;
    int num_points = findNumberLocalizationPoints(tmp_localization_points);
    if(num_points < 3)
        return false;
    int num_qrcodes;
    num_qrcodes = num_points / 3;
    if ((num_points % 3) != 0)
        num_qrcodes++;
    vector<vector<Point2f> > true_points_group;
    findQRCodeContours(tmp_localization_points, true_points_group, num_qrcodes);
    for(int q = 0; q < num_qrcodes; q++)
    {
       vector<vector<Point2f> > loc;
       size_t iter = localization_points.size();

       if(!checkSets(true_points_group, loc, tmp_localization_points))
            break;
       deleteUsedPoints(true_points_group, loc, tmp_localization_points);
       if((localization_points.size() - iter) == 1)
           q--;
       if(((localization_points.size() - iter) == 0) && (tmp_localization_points.size() == 0) && (true_points_group.size() == 1) )
            break;
    }
    if ((transformation_points.size() == 0) || (localization_points.size() == 0))
       return false;
    return true;
}
bool QRDetectMulti::computeTransformationPoints(size_t cur_ind)
{
    CV_TRACE_FUNCTION();
    if (localization_points[cur_ind].size() != 3)
    {
         return false;
    }

    vector<Point> locations, non_zero_elem[3], newHull;
    vector<Point2f> new_non_zero_elem[3];
    for (size_t i = 0; i < 3 ; i++)
    {
          Mat mask = Mat::zeros(bin_barcode.rows + 2, bin_barcode.cols + 2, CV_8UC1);
          uint8_t next_pixel, future_pixel = 255;
          int count_test_lines = 0, index = cvRound(localization_points[cur_ind][i].x);
          for (; index < bin_barcode.cols - 1; index++)
          {

                  next_pixel = bin_barcode.ptr<uint8_t>(cvRound(localization_points[cur_ind][i].y))[index + 1];
                  if (next_pixel == future_pixel)
                  {
                      future_pixel = static_cast<uint8_t>(~future_pixel);
                      count_test_lines++;

                      if (count_test_lines == 2)
                      {

                            floodFill(bin_barcode, mask,
                                      Point(index + 1, cvRound(localization_points[cur_ind][i].y)), 255,
                                      0, Scalar(), Scalar(), FLOODFILL_MASK_ONLY);
                            break;
                      }
                  }

            }
            Mat mask_roi = mask(Range(1, bin_barcode.rows - 1), Range(1, bin_barcode.cols - 1));
            findNonZero(mask_roi, non_zero_elem[i]);
            newHull.insert(newHull.end(), non_zero_elem[i].begin(), non_zero_elem[i].end());
      }
      convexHull(newHull, locations);
      for (size_t i = 0; i < locations.size(); i++)
      {
          for (size_t j = 0; j < 3; j++)
          {
              for (size_t k = 0; k < non_zero_elem[j].size(); k++)
              {
                  if (locations[i] == non_zero_elem[j][k])
                  {
                      new_non_zero_elem[j].push_back(locations[i]);
                  }

              }
          }
      }

      double pentagon_diag_norm = -1;
      Point2f down_left_edge_point, up_right_edge_point, up_left_edge_point;
      for (size_t i = 0; i < new_non_zero_elem[1].size(); i++)
      {
          for (size_t j = 0; j < new_non_zero_elem[2].size(); j++)
          {
              double temp_norm = norm(new_non_zero_elem[1][i] - new_non_zero_elem[2][j]);
              if (temp_norm > pentagon_diag_norm)
              {
                  down_left_edge_point = new_non_zero_elem[1][i];
                  up_right_edge_point  = new_non_zero_elem[2][j];
                  pentagon_diag_norm = temp_norm;
              }
          }
        }

        if (down_left_edge_point == Point2f(0, 0) ||
        up_right_edge_point  == Point2f(0, 0) ||
        new_non_zero_elem[0].size() == 0) { return false; }

        double max_area = -1;
        up_left_edge_point = new_non_zero_elem[0][0];

        for (size_t i = 0; i < new_non_zero_elem[0].size(); i++)
        {
            vector<Point2f> list_edge_points;
            list_edge_points.push_back(new_non_zero_elem[0][i]);
            list_edge_points.push_back(down_left_edge_point);
            list_edge_points.push_back(up_right_edge_point);

            double temp_area = fabs(contourArea(list_edge_points));
            if (max_area < temp_area)
            {
                up_left_edge_point = new_non_zero_elem[0][i];
                max_area = temp_area;
            }
        }

        Point2f down_max_delta_point, up_max_delta_point;
        double norm_down_max_delta = -1, norm_up_max_delta = -1;
        for (size_t i = 0; i < new_non_zero_elem[1].size(); i++)
        {
            double temp_norm_delta = norm(up_left_edge_point - new_non_zero_elem[1][i]) + norm(down_left_edge_point - new_non_zero_elem[1][i]);
            if (norm_down_max_delta < temp_norm_delta)
            {
                down_max_delta_point = new_non_zero_elem[1][i];
                norm_down_max_delta = temp_norm_delta;
            }
        }


        for (size_t i = 0; i < new_non_zero_elem[2].size(); i++)
        {
            double temp_norm_delta = norm(up_left_edge_point - new_non_zero_elem[2][i]) + norm(up_right_edge_point - new_non_zero_elem[2][i]);
            if (norm_up_max_delta < temp_norm_delta)
            {
                up_max_delta_point = new_non_zero_elem[2][i];
                norm_up_max_delta = temp_norm_delta;
            }
         }
         vector<Point2f> tmp_transformation_points;
         tmp_transformation_points.push_back(down_left_edge_point);
         tmp_transformation_points.push_back(up_left_edge_point);
         tmp_transformation_points.push_back(up_right_edge_point);
         tmp_transformation_points.push_back(intersectionLines(down_left_edge_point, down_max_delta_point,
                       up_right_edge_point, up_max_delta_point));
         transformation_points[cur_ind] = tmp_transformation_points;

         vector<Point2f> quadrilateral = getQuadrilateral(transformation_points[cur_ind]);
         transformation_points[cur_ind] = quadrilateral;

  return true;
}

bool QRCodeDetector::detectMulti(InputArray in, OutputArrayOfArrays points) const
{
  Mat inarr = in.getMat();
  CV_Assert(!inarr.empty());
  CV_Assert(inarr.depth() == CV_8U);
  if (inarr.cols <= 20 || inarr.rows <= 20)
      return false;  // image data is not enough for providing reliable results

  int incn = inarr.channels();
  if( incn == 3 || incn == 4 )
  {
      Mat gray;
      cvtColor(inarr, gray, COLOR_BGR2GRAY);
      inarr = gray;
  }
  QRDetectMulti qrdet;
  qrdet.init(inarr, p->epsX, p->epsY);
  if (!qrdet.localization()) { return false; }
  vector< vector< Point2f > > pnts2f = qrdet.getTransformationPoints();
  points.create((int)pnts2f.size(), 1, points.fixedType() ? points.type() : CV_32FC2);
  for(int i = 0; i < (int)pnts2f.size(); i++)
  {
      points.create(4, 1, points.fixedType() ? points.type() : CV_32FC2, i, true);
      Mat m = points.getMat(i);
      switch(points.type())
      {
          case CV_16UC2:
              for(int j = 0; j < 4; j++)
                  m.ptr<Vec2w>(0)[j] = Point_<unsigned short>(pnts2f[i][j]);
              break;
          case CV_16SC2:
              for(int j = 0; j < 4; j++)
                  m.ptr<Vec2s>(0)[j] = Point_<short>(pnts2f[i][j]);
              break;
          case CV_32SC2:
              for(int j = 0; j < 4; j++)
                  m.ptr<Vec2i>(0)[j] = Point2i(pnts2f[i][j]);
              break;
          case CV_32FC2:
              for(int j = 0; j < 4; j++)
                  m.ptr<Vec2f>(0)[j] = Point2f(pnts2f[i][j]);
              break;
          case CV_64FC2:
              for(int j = 0; j < 4; j++)
                  m.ptr<Vec2d>(0)[j] = Point2d(pnts2f[i][j]);
              break;
      }
  }
  return true;
}

bool detectQRCodeMulti(InputArray in, vector< vector< Point > > &points, double eps_x, double eps_y)
{
    QRCodeDetector qrdetector;
    qrdetector.setEpsX(eps_x);
    qrdetector.setEpsY(eps_y);
    return qrdetector.detectMulti(in, points);
}

class ParallelDecodeProcess : public ParallelLoopBody
{
public:
  ParallelDecodeProcess(Mat& inarr_, vector<QRDecode>& qrdec_, vector<std::string>& decoded_info_,
                        vector<Mat>& straight_barcode_, vector< vector< Point2f > >& src_points_)
     : inarr(inarr_), qrdec(qrdec_), decoded_info(decoded_info_),
     straight_barcode(straight_barcode_), src_points(src_points_)
  {
  };
  void operator ()(const Range& range) const CV_OVERRIDE
  {
      for(int i = range.start; i < range.end; i++)
      {
          qrdec[i].init(inarr, src_points[i]);
          bool ok = qrdec[i].fullDecodingProcess();
          if(ok)
          {
              decoded_info[i] = qrdec[i].getDecodeInformation();
              straight_barcode[i] = qrdec[i].getStraightBarcode();
          }
          else if (std::min(inarr.size().width, inarr.size().height) > 512)
          {
              const int min_side = std::min(inarr.size().width, inarr.size().height);
              double coeff_expansion = min_side / 512;
              const int width  = cvRound(inarr.size().width  / coeff_expansion);
              const int height = cvRound(inarr.size().height / coeff_expansion);
              Size new_size(width, height);
              Mat inarr2;
              resize(inarr, inarr2, new_size, 0, 0, INTER_AREA);
              for (size_t j = 0; j < 4; j++)
              {
                  src_points[i][j] /= static_cast<float>(coeff_expansion);
              }
              qrdec[i].init(inarr2, src_points[i]);
              ok = qrdec[i].fullDecodingProcess();
              if(ok)
              {
                  decoded_info[i] = qrdec[i].getDecodeInformation();
                  straight_barcode[i] = qrdec[i].getStraightBarcode();
              }

          }
          if(decoded_info[i].empty())
              decoded_info[i] = "";

      }
  };

  private:
  Mat& inarr;
  vector<QRDecode>& qrdec;
  vector<std::string>& decoded_info;
  vector<Mat>& straight_barcode;
  vector< vector< Point2f > >& src_points;

};

bool QRCodeDetector:: decodeMulti(InputArray in, CV_OUT std::vector<cv::String>& decoded_info,
                                   InputArrayOfArrays points,
                                   OutputArrayOfArrays straight_qrcode)
{
    Mat inarr = in.getMat();
    CV_Assert(!inarr.empty());
    CV_Assert(inarr.depth() == CV_8U);
    if (inarr.cols <= 20 || inarr.rows <= 20)
    {
        return false; // image data is not enough for providing reliable results
    }
    int incn = inarr.channels();
    if( incn == 3 || incn == 4 )
    {
        Mat gray;
        cvtColor(inarr, gray, COLOR_BGR2GRAY);
        inarr = gray;
    }
    decoded_info.clear();
    vector< vector< Point2f > > src_points ;
    for(int i = 0; i < points.size().width; i++)
    {
        vector<Point2f> tempMat = points.getMat(i);
        if((tempMat.size() == 4) && (contourArea(tempMat) > 0.0))
        {
            src_points.push_back(tempMat);
        }
    }
    CV_Assert(src_points.size() > 0);
    vector<QRDecode> qrdec(src_points.size());
    vector<Mat> straight_barcode(src_points.size());
    vector<std::string> info(src_points.size());
    ParallelDecodeProcess parallelDecodeProcess(inarr, qrdec, info, straight_barcode, src_points);
    parallel_for_(Range(0, int(src_points.size())), parallelDecodeProcess);
    vector<Mat> for_copy;
    for(size_t i = 0; i < straight_barcode.size(); i++)
    {
        if(!(straight_barcode[i].empty()))
            for_copy.push_back(straight_barcode[i]);
    }
    straight_barcode = for_copy;
    vector<Mat> tmp_straight_qrcodes;
    if (straight_qrcode.needed())
    {
        for(size_t i = 0; i < straight_barcode.size(); i++)
        {
            Mat tmp_straight_qrcode;
            tmp_straight_qrcodes.push_back(tmp_straight_qrcode);
            straight_barcode[i].convertTo(((OutputArray)tmp_straight_qrcodes[i]),
                                             ((OutputArray)tmp_straight_qrcodes[i]).fixedType() ?
                                             ((OutputArray)tmp_straight_qrcodes[i]).type() : CV_32FC2);
        }
        straight_qrcode.createSameSize(tmp_straight_qrcodes, CV_32FC2);
        straight_qrcode.assign(tmp_straight_qrcodes);
    }
    for(size_t i = 0; i < info.size(); i++)
    {
       decoded_info.push_back(info[i]);
    }
    if (!decoded_info.empty()) return true;
    else return false;
}

bool QRCodeDetector:: detectAndDecodeMulti(InputArray img,
                                            CV_OUT std::vector<cv::String>& decoded_info,
                                            OutputArrayOfArrays points_,
                                            OutputArrayOfArrays straight_qrcode)
{
    Mat inarr = img.getMat();
    CV_Assert(!inarr.empty());
    CV_Assert(inarr.depth() == CV_8U);

    if (inarr.cols <= 20 || inarr.rows <= 20)
    {
        return false;  // image data is not enough for providing reliable results
    }
    int incn = inarr.channels();
    if( incn == 3 || incn == 4 )
    {
        Mat gray;
        cvtColor(inarr, gray, COLOR_BGR2GRAY);
        inarr = gray;
    }
    decoded_info.clear();
    vector< vector< Point2f > > points;
    bool ok = detectMulti(inarr, points);
    if (points_.needed())
    {
        if(ok)
        {
            points_.create(int(points.size()), 1,  points_.fixedType() ? points_.type() : CV_32FC2);
            for(size_t i = 0; i < points.size(); i++)
            {
                points_.create(4, 1, points_.fixedType() ? points_.type() : CV_32FC2, int(i), true);
                Mat m = points_.getMat(int(i));
                switch(points_.type())
                {
                    case CV_16UC2:
                        for(int j = 0; j < 4; j++)
                            m.ptr<Vec2w>(0)[j] = Point_<unsigned short>(points[i][j]);
                        break;
                    case CV_16SC2:
                        for(int j = 0; j < 4; j++)
                            m.ptr<Vec2s>(0)[j] = Point_<short>(points[i][j]);
                        break;
                    case CV_32SC2:
                        for(int j = 0; j < 4; j++)
                            m.ptr<Vec2i>(0)[j] = Point2i(points[i][j]);
                        break;
                    case CV_32FC2:
                        for(int j = 0; j < 4; j++)
                            m.ptr<Vec2f>(0)[j] = Point2f(points[i][j]);
                        break;
                    case CV_64FC2:
                        for(int j = 0; j < 4; j++)
                            m.ptr<Vec2d>(0)[j] = Point2d(points[i][j]);
                        break;
                }
            }
        }
        else
            points_.release();
    }
    if(ok)
        return decodeMulti(inarr, decoded_info, points, straight_qrcode);
    else return false;
}

bool decodeQRCodeMulti(InputArray  in, InputArrayOfArrays points,
                  vector<std::string> &decoded_info, OutputArrayOfArrays straight_qrcode)
{
    QRCodeDetector qrcode;
    vector<cv::String> info;
    bool ok = qrcode.decodeMulti(in, info, points, straight_qrcode);
    for(size_t i = 0; i < info.size(); i++)
        decoded_info.push_back(info[i]);
    return ok;
}
}
