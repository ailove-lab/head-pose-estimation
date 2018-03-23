
#include <iostream>
#include <dlib/opencv.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>

#include <zmq.hpp>

using std::vector;

using cv::Point3d;
using cv::Point2d;
using cv::Scalar;
using cv::Mat;

//Intrisics can be calculated using opencv sample code under opencv/sources/samples/cpp/tutorial_code/calib3d
//Normally, you can also apprximate fx and fy by image width, cx by half image width, cy by half image height instead
double K[9] = { 6.5308391993466671e+002, 0.0, 3.1950000000000000e+002, 0.0, 6.5308391993466671e+002, 2.3950000000000000e+002, 0.0, 0.0, 1.0 };
double D[5] = { 7.0834633684407095e-002, 6.9140193737175351e-002, 0.0, 0.0, -1.3073460323689292e+000 };

// fill in 3D ref points(world coordinates), 
// model referenced from http://aifi.isr.uc.pt/Downloads/OpenGL/glAnthropometric3DModel.cpp
vector<Point3d> object_pts = {
   { 6.825897,  6.760612, 4.402142}, //#33 left brow left corner
   { 1.330353,  7.122144, 6.903745}, //#29 left brow right corner
   {-1.330353,  7.122144, 6.903745}, //#34 right brow left corner
   {-6.825897,  6.760612, 4.402142}, //#38 right brow right corner
   { 5.311432,  5.485328, 3.987654}, //#13 left eye left corner
   { 1.789930,  5.393625, 4.413414}, //#17 left eye right corner
   {-1.789930,  5.393625, 4.413414}, //#25 right eye left corner
   {-5.311432,  5.485328, 3.987654}, //#21 right eye right corner
   { 2.005628,  1.409845, 6.165652}, //#55 nose left corner
   {-2.005628,  1.409845, 6.165652}, //#49 nose right corner
   { 2.774015, -2.080775, 5.048531}, //#43 mouth left corner
   {-2.774015, -2.080775, 5.048531}, //#39 mouth right corner
   { 0.000000, -3.116408, 6.097667}, //#45 mouth central bottom corner
   { 0.000000, -7.415691, 4.070434}, //#6 chin corner
};

//reproject 3D points world coordinate axis to verify result pose
vector<Point3d> reprojectsrc = {
    { 10.0,  10.0,  10.0},
    { 10.0,  10.0, -10.0},
    { 10.0, -10.0, -10.0},
    { 10.0, -10.0,  10.0},
    {-10.0,  10.0,  10.0},
    {-10.0,  10.0, -10.0},
    {-10.0, -10.0, -10.0},
    {-10.0, -10.0,  10.0},
};
    
// debug color
Scalar color = Scalar(0,0,255); 

//fill in cam intrinsics and distortion coefficients
Mat cam_matrix  = Mat(3, 3, CV_64FC1, K);
Mat dist_coeffs = Mat(5, 1, CV_64FC1, D);

//2D ref points(image coordinates), referenced from detected facial feature
vector<Point2d> image_pts;

//result
Mat rotation_vec;                           //3 x 1
Mat rotation_mat;                           //3 x 3 R
Mat translation_vec;                        //3 x 1 T
Mat pose_mat    = Mat(3, 4, CV_64FC1);      //3 x 4 R | T
Mat euler_angle = Mat(3, 1, CV_64FC1);

//reprojected 2D points
vector<Point2d> reproject_dst(8);

//temp buf for decomposeProjectionMatrix()
Mat out_intrinsics  = Mat(3, 3, CV_64FC1);
Mat out_rotation    = Mat(3, 3, CV_64FC1);
Mat out_translation = Mat(3, 1, CV_64FC1);

// fill in 2D ref points, annotations follow https://ibug.doc.ic.ac.uk/resources/300-W/
// 17 left brow left 
// 21 left brow right 
// 22 right brow left 
// 26 right brow right 
// 36 left eye left 
// 39 left eye right 
// 42 right eye left 
// 45 right eye right 
// 31 nose left 
// 35 nose right 
// 48 mouth left 
// 54 mouth right 
// 57 mouth central bottom 
//  8 chin 
int part_ids[14] = { 17,  21,  22,  26,  36,  39,  42,  45,  31,  35,  48,  54,  57,  8 };

//cube edges
int edges[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

int main() {

    //open cam
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cout << "Unable to connect to camera" << std::endl;
        return EXIT_FAILURE;
    }

    // zmq server for result broadcasting
    zmq::context_t context(1);
    zmq::socket_t  publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");
    //publisher.bind("ipc://head-pose.ipc");

    //Load face detection and pose estimation models (dlib).
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor predictor;
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> predictor;

    //text on screen
    std::ostringstream outtext;

    //main loop
    while (1) {
        // Grab a frame
        Mat temp;
        cap >> temp;
        dlib::cv_image<dlib::bgr_pixel> cimg(temp);

        // Detect faces
        std::vector<dlib::rectangle> faces = detector(cimg);

        // Find the pose of each face
        if (faces.size() > 0) {
            
            //track features
            dlib::full_object_detection shape = predictor(cimg, faces[0]);

            //draw features
            for (unsigned int i = 0; i < 68; ++i) {
                cv::circle(temp, cv::Point(shape.part(i).x(), shape.part(i).y()), 2, color, -1);
            }

            for(int i=0; i<14; i++)
                image_pts.push_back(
                    Point2d(shape.part(part_ids[i]).x(), shape.part(part_ids[i]).y()));

            //calc pose
            cv::solvePnP(
                object_pts, 
                image_pts, 
                cam_matrix, 
                dist_coeffs, 
                rotation_vec, 
                translation_vec);

            //reproject
            cv::projectPoints(
                reprojectsrc, 
                rotation_vec, 
                translation_vec, 
                cam_matrix, 
                dist_coeffs,
                reproject_dst);

            //calc euler angle
            cv::Rodrigues(rotation_vec, rotation_mat);

            cv::hconcat(rotation_mat, translation_vec, pose_mat);
            cv::decomposeProjectionMatrix(
                pose_mat, 
                out_intrinsics, 
                out_rotation, 
                out_translation, 
                cv::noArray(), 
                cv::noArray(), 
                cv::noArray(), 
                euler_angle);

            // Convert to GL
            // http://answers.opencv.org/question/23089/opencv-opengl-proper-camera-pose-using-solvepnp/
            // Build view mattrix
            Mat view_mat = Mat::zeros(4,4, CV_64FC1);
            for(unsigned int row=0; row<3; ++row) {
               for(unsigned int col=0; col<3; ++col) {
                  view_mat.at<double>(row, col) = rotation_mat.at<double>(row, col);
               }
               view_mat.at<double>(row, 3) = translation_vec.at<double>(row, 0);
            }
            view_mat.at<double>(3, 3) = 1.0f;

            // invert axis
            Mat cv2gl = Mat::zeros(4, 4, CV_64FC1);
            cv2gl.at<double>(0, 0) =  1.0f;
            cv2gl.at<double>(1, 1) = -1.0f; // Invert the y axis
            cv2gl.at<double>(2, 2) = -1.0f; // invert the z axis
            cv2gl.at<double>(3, 3) =  1.0f;
            view_mat = cv2gl * view_mat;

            // transpose mattrix
            Mat gl_mat = cv::Mat::zeros(4, 4, CV_64FC1);
            cv::transpose(view_mat , gl_mat);

            // broadcast values throught ZMQ
            // 4x4 8byte floats = 128 bytes 
            zmq::message_t msg(128);
            std::memcpy(msg.data(), &gl_mat.at<double>(0,0), 128);
            // double* d = (double*)msg.data(); for (int i=0; i<16; i++) printf("%.2f ", d[i]); printf("\n");
            publisher.send(msg);

            //draw axis
            for(int i=0; i<12; i++) 
                cv::line(temp, reproject_dst[edges[i][0]], reproject_dst[edges[i][1]], color);

            //show angle result
            outtext << "X: " << std::setprecision(3) << euler_angle.at<double>(0);
            cv::putText(temp, outtext.str(), cv::Point(50, 40), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
            outtext.str("");
            
            outtext << "Y: " << std::setprecision(3) << euler_angle.at<double>(1);
            cv::putText(temp, outtext.str(), cv::Point(50, 60), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
            outtext.str("");
            
            outtext << "Z: " << std::setprecision(3) << euler_angle.at<double>(2);
            cv::putText(temp, outtext.str(), cv::Point(50, 80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
            outtext.str("");

            image_pts.clear();

            
        }

        //press esc to end
        cv::imshow("demo", temp);
        unsigned char key = cv::waitKey(1);
        if (key == 27) {
            break;
        }
    }

    return 0;
}
