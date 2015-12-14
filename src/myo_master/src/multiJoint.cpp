#include <controller_manager/controller_manager.h>
#include "roboy_hardwareInterface.hpp"

int main(int argc, char* argv[])
{
    ros::init(argc, argv, "multiJoint");
    
    Roboy robot;
    controller_manager::ControllerManager cm(&robot);
    
    // this is for asyncronous ros callbacks
    ros::AsyncSpinner spinner(1);
    spinner.start();
    
    // Control loop
    ros::Time prev_time = ros::Time::now();
    ros::Rate rate(10);
    
    while (ros::ok())
    {
	if(robot.ready){
            const ros::Time	    time = ros::Time::now();
            const ros::Duration period = time - prev_time;
            
            robot.read();
            cm.update(time, period);
            robot.write();
            
            rate.sleep();
        }else{
            ROS_WARN_THROTTLE(1, "roboy not ready");
        }
    }
    return 0;
}

