#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    if (client.call(srv))
        ROS_INFO("DriveToTarget: %s", srv.response.msg_feedback.c_str());
    else
        ROS_ERROR("DriveToTarget failed");
}

bool pixel_is_white(const sensor_msgs::Image img, uint32_t row, uint32_t col)
{
    int offset = row * img.step + 3*col;
    int white_pixel = 255;

    return (img.data[offset] == white_pixel) &&  (img.data[offset+1] == white_pixel) &&  (img.data[offset+2] == white_pixel);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{



    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int white_pixels_left = 0;
    int white_pixels_center = 0;
    int white_pixels_right = 0;

    for (int r = 0; r < img.height; r+=80) {
        for (int c = 0; c < 200; c+=40)
            if (pixel_is_white(img, r, c))
                white_pixels_left++;
        for (int c = 200; c < 600; c+=40)
            if (pixel_is_white(img, r, c))
                white_pixels_center++;
        for (int c = 600; c < 800; c+=40)
            if (pixel_is_white(img, r, c))
                white_pixels_right++;
    }

    if (white_pixels_center == 0 && white_pixels_left==0 && white_pixels_right==0)
        drive_robot(0, 0);
    else if (white_pixels_left > white_pixels_center)
        drive_robot(0, 0.2);
    else if (white_pixels_right > white_pixels_center)
        drive_robot(0, -0.2);
    else
        drive_robot(0.2, 0);  // forward
    

    // std::stringstream msg;
    // msg << std::fixed << std::setprecision(1);
    // msg << "White pixels: " << white_pixels;
    ROS_INFO("White pixels: l:%d c:%d r:%d", white_pixels_left, white_pixels_center, white_pixels_right);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    ROS_INFO("Process image started");

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}