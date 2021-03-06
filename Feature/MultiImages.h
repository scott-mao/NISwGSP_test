#if !defined(MultiImages_H)
#define MultiImages_H

#include "../common.h"

#include "../Feature/FeatureController.h"
#include "../Feature/ImageData.h"
#include "../Stitch/APAP_Stitching.h"
#include "../Util/Blending.h"
#include "../Util/Statistics.h"
#include "../Util/Transform.h"

class FeatureDistance {
public:
  double distance;
  int feature_index[2];

  FeatureDistance() {// 无参初始化
    feature_index[0] = feature_index[1] = -1;
    distance = MAXFLOAT;
  }

  FeatureDistance(const double _distance,
                  const int _p,
                  const int _feature_index_1,
                  const int _feature_index_2) {// 有参构造函数
    distance = _distance;
    feature_index[    _p] = _feature_index_1;
    feature_index[1 - _p] = _feature_index_2;
  }

  bool operator < (const FeatureDistance &fd) const {
      return distance > fd.distance;
  }
};

class dijkstraNode {
public:
  int from, pos;
  double dis;
  dijkstraNode(const int & _from,
      const int & _pos,
      const double & _dis) : from(_from), pos(_pos), dis(_dis) {
  }
  bool operator < (const dijkstraNode & rhs) const {
    return dis > rhs.dis;
  }
};

class SimilarityElements {
public:
  double scale;
  double theta;
  SimilarityElements() {// TODO
    scale = 1;
    theta = 0;
  };
  SimilarityElements(const double _scale,
                     const double _theta) {
    scale = _scale;
    theta = _theta;
  }
};

class MultiImages {// 注意reserve与resize的区别
public:
  MultiImages();

  int img_num;
  int center_index = 0;// 参照图片的索引
  vector<ImageData *> imgs;
  vector<double> img_rotations;// 拍摄时的旋转角度

  vector<pair<int, int> > img_pairs;// 图片的配对信息
  vector<vector<bool> >   images_match_graph;// 配对矩阵

  // 两辆图片的配对信息:[m1][m2],第m1张图片为参照,与第m2张图片为目标
  vector<vector<vector<pair<int, int> > > > feature_pairs;// 特征点配对信息:[m1][m2]<i, j>,第m1张图片的第i个网格点对应第m2张图片的第j个匹配点(实际上[m1][m2]与[m2][m1]重复(相反))
  vector<vector<vector<Point2f> > >         feature_points;// [m1][m2]: m1与m2成功匹配的特征点;

  // vector<vector<Point2f> >                  keypoints;// mesh点 + 过滤后的匹配点(原images_features)
  vector<ImageFeatures>             image_features;// 包含keypoints
  vector<MatchesInfo>               pairwise_matches;// 只用于调库函数
  vector<CameraParams>              camera_params;

  vector<vector<bool> >             image_features_mask;// [m1][i],第m1张的第i个匹配点是否可行(只要对任意一张图片可行则可行)

  vector<vector<vector<pair<int, int> > > > keypoints_pairs;// (pairwise_matches)(mesh点 + 匹配点)配对信息:[m1][m2]<i, j>,第m1张图片的第i个网格点对应第m2张图片的第j个匹配点

  vector<vector<vector<Mat> > >     apap_homographies;
  vector<vector<vector<bool> > >    apap_overlap_mask;
  vector<vector<vector<Point2f> > > apap_matching_points;

  vector<vector<InterpolateVertex> > mesh_interpolate_vertex_of_matching_pts;// TODO

  vector<int>                             images_vertices_start_index;// TODO
  vector<vector<pair<double, double> > >  images_relative_rotation_range;// TODO 旋转角度范围

  vector<vector<double> >            images_polygon_space_matching_pts_weight;// TODO

  vector<SimilarityElements> images_similarity_elements;// 旋转角度和缩放比

  /* Blending */
  int using_seam_finder;// 使用接缝线进行图像拼接
  Size2f target_size;// 最终Mat大小

  vector<vector<Point2f> > image_mesh_points;// 最终结果(从上往下, 从左往右)
  vector<Mat>              images_warped;// 存放wrap后图片
  vector<Mat>              masks_warped;// 存放wrap后mask
  vector<UMat>             gpu_images_warped;
  vector<UMat>             gpu_masks_warped;

  vector<Point2f>          origins;// 每幅warped图像的初始坐标
  vector<Point2i>          corners;// 初始坐标的整数形式
  vector<Mat>              blend_weight_mask;// new_weight_mask

  /* Line */
  // 已删除

  void read_img(const char *img_path);
  void getFeaturePairs();
  vector<pair<int, int> > getVlfeatFeaturePairs(const int m1, const int m2);
  vector<pair<int, int> > getFeaturePairsBySequentialRANSAC(const vector<Point2f> & _X,
                                                            const vector<Point2f> & _Y,
                                                            const vector<pair<int, int> > & _initial_indices);
  vector<vector<InterpolateVertex> > getInterpolateVerticesOfMatchingPoints();
  vector<int> getImagesVerticesStartIndex();
  vector<vector<double> > getImagesGridSpaceMatchingPointsWeight(const double _global_weight_gamma);
  vector<CameraParams> getCameraParams();
  vector<SimilarityElements> getImagesSimilarityElements();

  void do_matching();
  void warpImages();
  void exposureCompensate();// 曝光补偿
  void getSeam();// 寻找接缝线
  Mat textureMapping();
};

#endif