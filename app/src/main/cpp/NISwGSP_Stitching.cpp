#include "NISwGSP_Stitching.hpp"

void NISwGSP_Stitching::stitch_test(Mat img1, Mat img2) {
  Ptr<SIFT> my_sift = SIFT::create();
  vector<KeyPoint> key_points_1, key_points_2;

  // 检测特征点
  my_sift->detect(img1, key_points_1);
  my_sift->detect(img2, key_points_2);
  multiImages->key_points.push_back(key_points_1);
  multiImages->key_points.push_back(key_points_2);

  LOG("sift finished");

  // TODO 匹配类型转换
  Mat descrip_1, descrip_2;
  my_sift->compute(img1, key_points_1, descrip_1);
  my_sift->compute(img2, key_points_2, descrip_2);
  if (descrip_1.type() != CV_32F || descrip_2.type() != CV_32F) {
      descrip_1.convertTo(descrip_1, CV_32F);
      descrip_2.convertTo(descrip_2, CV_32F);
  }

  LOG("compute finished");

  // 特征点匹配
  Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");
  vector<DMatch> feature_matches;// 存储配对信息
  descriptor_matcher->match(descrip_1, descrip_2, feature_matches);// 进行匹配
  multiImages->descriptor.push_back(descrip_1);
  multiImages->descriptor.push_back(descrip_2);

  LOG("match finished");

  // 过滤bad特征匹配
  double max_dis = 0;// 最大的匹配距离
  for (int i = 0; i < descrip_1.rows; i ++) {
    double tmp_dis = feature_matches[i].distance;
    if (tmp_dis > max_dis) {
      max_dis = tmp_dis;
    }
  }
  for (int i = 0; i < descrip_1.rows; i ++) {
    double tmp_dis = feature_matches[i].distance;
    if (tmp_dis < max_dis * 0.5) {
      multiImages->feature_matches.push_back(feature_matches[i]);// 存储好的特征匹配
    }
  }

  LOG("get good mathces");
}

void NISwGSP_Stitching::sift_test(Mat img1, Mat img2) {
  Ptr<SIFT> my_sift = SIFT::create();
  vector<ImageFeatures> features(2);
  computeImageFeatures(my_sift, img1, features[0]);
  computeImageFeatures(my_sift, img2, features[1]);

  LOG("compute finish");

  // 特征匹配
  vector<MatchesInfo> pairwise_matches;
  Ptr<FeaturesMatcher> matcher = makePtr<BestOf2NearestMatcher>(false, 0.3f, 6, 6);// TODO
  (*matcher)(features, pairwise_matches);

  LOG("match finish");

  // 过滤bad特征匹配
  double max_dis = 0;// 最大的匹配距离
  for (int i = 0; i < pairwise_matches.matches.size(); i ++) {
    double tmp_dis = pairwise_matches.matches[i].distance;
    if (tmp_dis > max_dis) {
      max_dis = tmp_dis;
    }
  }
  for (int i = 0; i < pairwise_matches.matches.size(); i ++) {
    double tmp_dis = pairwise_matches.matches[i].distance;
    if (tmp_dis < max_dis * 0.5) {
      multiImages->feature_matches.push_back(pairwise_matches.matches[i]);// 存储好的特征匹配
    }
  }

  LOG("get good mathces");
}

Mat draw_matches() {
  // 匹配特征点
  SIFT_Features::sift_test(multiImages->imgs[0], multiImages->imgs[1]);// 特征点匹配

  // 描绘特征点
  Mat result_1;// 存储结果
  Mat left_1, right_1;// 分割矩阵
  Mat img1 = multiImages->imgs[0];
  Mat img2 = multiImages->imgs[1];
  result_1 = Mat::zeros(max(img1.rows, img2.rows), img1.cols + img2.cols, CV_8UC3);
  left_1  = Mat(result_1, Rect(0, 0, img1.cols, img1.rows));
  right_1 = Mat(result_1, Rect(img1.cols, 0, img2.cols, img2.rows));
  for (int i = 0; i < multiImages->feature_matches.size(); i ++) {
    // 获取特征点
    int src = multiImages->feature_matches[i].queryIdx;
    int dest = multiImages->feature_matches[i].trainIdx;
    Point2f src_p, dest_p;
    src_p = multiImages->key_points[0][src].pt;
    dest_p = multiImages->key_points[1][dest].pt;

    // 描绘
    Scalar color(rand() % 256, rand() % 256, rand() % 256);
    circle(result_1, src_p, 3, color, -1);
    line(result_1, src_p, dest_p + Point2f(img1.cols, 0), color, 1, LINE_AA);
    circle(result_1, dest_p + Point2f(img1.cols, 0), 3, color, -1);
  }

  return result_1;
}