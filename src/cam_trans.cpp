#include "ros/ros.h"
#include "std_msgs/String.h"
#include "fiducial_msgs/FiducialTransformArray.h"
#include <tf/transform_broadcaster.h> // publish pose to tf
#include <tf/transform_listener.h> // listen pose to tf
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

tf::StampedTransform cam2base;
fiducial_msgs::FiducialTransformArray aruco_;
int origin;
int robot;
int range = 40;



//#define POINT_MODE
#define NODE_MODE

std::string float_to_string(float f,std::string n){ 
  // float to mm then to string
  return std::to_string(static_cast<int>(f*1000)) + n;
}

std::string float_to_node_string(float fx, float fy, std::string n){
  // float to cm then to node
  // n11 n12 n13 n14 n15 y3
  // n6  n7  n8  n9  n10 y2
  // n1  n2  n3  n4  n5  y1
  // x1  x2  x3  x4  x5
  // for case above node = 5y + x
  // for x 11 = 2y + 1x = 10 + 1 = 11
  int fxx = static_cast<int>(fx * 100 / range);
  int fyy = static_cast<int>(fy * 100 / range);
  return std::to_string(fyy*10 + fxx) + n;
}

void MarkerCallback(const fiducial_msgs::FiducialTransformArray::ConstPtr& msg)
{
  //ROS_INFO("I hear marker");
  aruco_ = *msg;

  int found_flag = 0;
  int found_robot_flag = 0;
#ifdef POINT_MODE
  std::string output = "--";
#endif
#ifdef NODE_MODE
  std::string output = "S,";
#endif
  std::string robot_output = "";
  std::string temp;
  for(int i = 0;i < aruco_.transforms.size();i++){
    if(aruco_.transforms[i].fiducial_id == origin){
      tf::TransformBroadcaster br;
      tf::Transform cam2pos;
      cam2pos.setOrigin(  tf::Vector3(aruco_.transforms[i].transform.translation.x,aruco_.transforms[i].transform.translation.y,aruco_.transforms[i].transform.translation.z));
    cam2pos.setRotation( tf::Quaternion(aruco_.transforms[i].transform.rotation.x,aruco_.transforms[i].transform.rotation.y,aruco_.transforms[i].transform.rotation.z ,aruco_.transforms[i].transform.rotation.w));

      tf::Transform pos2base;
      pos2base = cam2pos.inverse() * cam2base;
      br.sendTransform(tf::StampedTransform(pos2base, ros::Time::now(), "map","base_link"));

      found_flag = 1;
      ROS_INFO("found origin !");
    }
    if(aruco_.transforms[i].fiducial_id == robot){
        found_robot_flag = 1;
        #ifdef POINT_MODE
            robot_output += flaot_to_string(aruco_.transforms[i].transform.translation.x, ",");
            robot_output += flaot_to_string(aruco_.transforms[i].transform.translation.y, ",");
            robot_output += flaot_to_string(aruco_.transforms[i].transform.translation.z, "/");
        #endif
        #ifdef NODE_MODE
            robot_output += "";
            //robot_output = float_to_node_string(aruco_.transforms[i].transform.translation.x, aruco_.transforms[i].transform.translation.y, ",");
            //robot_output += float_to_node_string(aruco_.transforms[i].transform.translation.z, ",");
        #endif
    }else{
        #ifdef POINT_MODE
            output += float_to_string(aruco_.transforms[i].transform.translation.x, ",");
            output += float_to_string(aruco_.transforms[i].transform.translation.y, ",");
            output += float_to_string(aruco_.transforms[i].transform.translation.z, "/");
        #endif
        #ifdef NODE_MODE
            if(aruco_.transforms[i].fiducial_id != origin){
              /*double x2 = aruco_.transforms[i].transform.translation.x;
              double y2 = aruco_.transforms[i].transform.translation.y;
              double z2 = aruco_.transforms[i].transform.translation.z;
              double dist = sqrt(x2*x2 + y2*y2 + z2*z2);
              std::cout << "distance: "<<dist<<"\n";
              */
              tf::TransformListener listener;
              tf::StampedTransform transform;
              try{
                //std::string ss1 = "/fiducial_";
		// second try
                std::string ss1 = "/camera_color_optical_frame";
                std::string ss2 = "/fiducial_";
		// second try
                //ss1.append(std::to_string(aruco_.transforms[i].fiducial_id));
                ss2.append(std::to_string(origin));
                listener.waitForTransform(ss2,ss1,ros::Time(0),ros::Duration(2.0));
                listener.lookupTransform(ss2,ss1,ros::Time(0),transform);
                //listener.lookupTransform(ss2,ss1,ros::Time(0),transform);
               }catch(tf::TransformException ex){
                 std::cout<<"\nNOT FOUND --" << ex.what()<<"\n";
               }

		// second try
               //float x = transform.getOrigin().x();
               //float y = transform.getOrigin().y();
               tf::Quaternion q = transform.getRotation();
               tf::Matrix3x3 mr(q);
               //tf::Quaternion q2(aruco_.transforms[i].transform.translation.x,aruco_.transforms[i].transform.translation.y,aruco_.transforms[i].transform.translation.z,0);

               //tf::Quaternion q3 = q*q2;
               //geometry_msgs::Vector3 v3 = q * aruco_.transforms[i].transform.translation;
               //v3 = q + transform.getOrigin();
               //std::cout << x <<" : " << y << "\n";
               //std::cout << "q: " << q.x() <<" "<<q.y()<<" "<<q.z()<<" "<<q.w() << "\n";
               //std::cout << "mr:" << mr[0][0]<<" "<<mr[0][1]<<" "<< mr[0][2] << "\n";
               //std::cout << "mr:" << mr[1][0]<<" "<<mr[1][1]<<" "<< mr[1][2] << "\n";
               //std::cout << "mr:" << mr[2][0]<<" "<<mr[2][1]<<" "<< mr[2][2] << "\n";
               //std::cout << "q2: " << q2.x() <<" "<<q2.y()<<" "<<q2.z()<<" "<<q2.w() << "\n";
               //std::cout << "q3: " << q3.x() <<" "<<q3.y()<<" "<<q3.z()<<" "<<q3.w() << "\n";
               //std::cout << "q: " << q<< " :=:  "<< v3 << "\n";
               //std::cout <<"\t"<< x<<" , "<<y<<"\n";
               float xx = aruco_.transforms[i].transform.translation.x;
               float yy = aruco_.transforms[i].transform.translation.y;
               float zz = aruco_.transforms[i].transform.translation.z;
               float x = mr[0][0] * xx + mr[0][1] * yy + mr[0][2] * zz + transform.getOrigin().x();
               float y = mr[1][0] * xx + mr[1][1] * yy + mr[1][2] * zz + transform.getOrigin().y();
               float z = mr[2][0] * xx + mr[2][1] * yy + mr[2][2] * zz + transform.getOrigin().z();
               std::cout<< "xyz:"<< x<<" "<<y <<" "<<z  <<"\n";
               //float y = transform.getOrigin().y();
               output += float_to_node_string(x, y, ",");
               //output += flaot_to_node_string(aruco_.transforms[i].transform.translation.z, ",");
              }
        #endif
    }
    //std::cout <<"\noutput:"<< output;


/*
    if(aruco_.transforms[i].fiducial_id == std::stoi(robot)){
      tf::TransformBroadcaster br;
      tf::Transform cam2pos;
      cam2pos.setOrigin(  tf::Vector3(aruco_.transforms[i].transform.translation.x,aruco_.transforms[i].transform.translation.y,aruco_.transforms[i].transform.translation.z));
    cam2pos.setRotation( tf::Quaternion(aruco_.transforms[i].transform.rotation.x,aruco_.transforms[i].transform.rotation.y,aruco_.transforms[i].transform.rotation.z ,aruco_.transforms[i].transform.rotation.w));

      tf::Transform pos2base;
      pos2base = cam2pos.inverse() * cam2base;
      br.sendTransform(tf::StampedTransform(pos2base, ros::Time::now(), "map","robot"));

      found_flag = 1;
      ROS_INFO("found origin !");
    }
*/

  }
  if (found_robot_flag == 1){
    temp = "";
    temp.append(output,0,2);
    temp.append(robot_output);
    temp.append(output.begin()+2,output.end());
    output = temp;
  }
  if (found_flag == 0){
      tf::TransformBroadcaster br;
      tf::Transform original;
      original.setOrigin(  tf::Vector3(0,0,1));
      original.setRotation( tf::Quaternion(0,0,0,1));
      br.sendTransform(tf::StampedTransform(original, ros::Time::now(), "map","base_link"));
  }
  //output[output.size()-1] = '\0';
  output = output.substr(0,output.size()-1);
#ifdef POINT_MODE
  output = output + "--";
#endif
#ifdef NODE_MODE
  output = output + ",E";
#endif
  std::cout << output <<"\n";
  std::ofstream fp("/home/icmems/WALLE_project/OA_by_MOEA/OA_by_MOEA/Data_Exchange/obstacle_position.txt");
  fp << output;
  fp.close();

}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "cam_tf");
  ros::NodeHandle n;
  if(ros::param::has("/origin_id")) {
    ros::param::get("/origin_id",origin);
  }else{
    origin = 10;
  }
  std::cout << "origin: "<< origin <<"\n";

  if(n.hasParam("robot_id")) {
    n.getParam("robot_id",robot);
  }else{
    robot = 40;
  }
  std::cout << "robot: "<< robot <<"\n";

  ros::Subscriber sub = n.subscribe("fiducial_transforms", 2, MarkerCallback);
  tf::TransformListener Listener;
  Listener.waitForTransform("/camera_color_optical_frame","/base_link",ros::Time(0),ros::Duration(5.0));
  Listener.lookupTransform("/camera_color_optical_frame","/base_link",ros::Time(0),cam2base);

  ros::spin();

  return 0;
}
